#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX
#include "GLFW/glfw3native.h"
#pragma comment(lib, "Dwmapi")
#include "dwmapi.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "kernels.h"
#include "win_utils.cpp"

#include "convolution_matrix.h"
#include <array>
#include <list>
#include <sstream>

#define PARAMETER_CHANGED(inputParameterChanged, isAlreadyChanged) (isAlreadyChanged = inputParameterChanged || isAlreadyChanged)

using namespace img_edit_cm;

class Gui {

private:
    const COLORREF windowTitleColor = 0x211330FF;
    const int displayImageWidth = 1280;
    const int displayImageHeight = 793;
    const int menuOffset = 650;
    const ImVec2 displayResolution = ImVec2(displayImageWidth, displayImageHeight);
    const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    HWND hwnd;
    int displayWidth, displayHeight = 0;
    float bias[3] = { 0.0, 0.0, 0.0 };
    float threshold[3] = { 1.0, 1.0, 1.0 };
    bool useRandom = true;
    int matWidth, matHeight = 0;
    char inputKernelName[32];
    bool openMatrixCreationWindow = false;
    GLFWwindow* window;
    std::vector<ConvolutionMatrix> convolutionMatrices {};
    Image* img = nullptr;
    Image* imgResult = nullptr;
    void* loadedTexture = nullptr;

    GLuint loadTexture()
    {
        auto imageChannels = imgResult->getChannels() >= 4 ? GL_RGBA : GL_RGB;
        unsigned char* resisedImageData = imgResult->resize(displayImageWidth, displayImageHeight);

        GLuint imageTexture;
        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

#if defined(GL_UPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

        glTexImage2D(GL_TEXTURE_2D, 0, imageChannels, displayImageWidth, displayImageHeight, 0, imageChannels, GL_UNSIGNED_BYTE, resisedImageData);
        delete resisedImageData;
        return imageTexture;
    }

    std::array<int, 3> lerp(float values[3])
    {
        std::array<int, 3> interpolatedValues;

        for (int i = 0; i < 3; i++) {
            interpolatedValues[i] = std::lerp(0, 255, values[i]);
        }
        return interpolatedValues;
    }

    void openImage()
    {
        if (img) {
            delete img;
            delete imgResult;
        }

        auto openPath = getOpenedFilePath().generic_string();
        if (openPath == "")
            return;

        img = new Image(openPath.c_str());
        imgResult = new Image(*img);
        loadedTexture = (void*)(intptr_t)loadTexture();
    }

    void HelpMarker(const char* description)
    {
        ImGui::TextDisabled("<?>");
        if (ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(description);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

public:
    void init(int windowWidth, int windowHeight)
    {
        glfwInit();
        window = glfwCreateWindow(windowWidth, windowHeight, "ImgEditCM", nullptr, nullptr);
        hwnd = glfwGetWin32Window(window);
        DwmSetWindowAttribute(hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &windowTitleColor, sizeof(windowTitleColor));
        if (!window) {
            glfwTerminate();
        }

        glfwMakeContextCurrent(window);
        glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
    }

    void draw()
    {
        bool valuesChanged = false;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            int leftMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowPos(viewport->Pos);

            // Start to draw user interface elements
            {
                ImGui::Begin("ImgEditCM", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar);
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Image")) {
                        if (ImGui::MenuItem("Open", "Ctrl+O")) {
                            openImage();
                        }
                        if (ImGui::MenuItem("Export", "Ctrl+S")) {
                            filesystem::path savePath = saveFilePath();
                            if (savePath == "")
                                return;

                            imgResult->save(savePath);
                        }
                        ImGui::Separator();
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                if (loadedTexture) {
                    ImGui::Image(loadedTexture, displayResolution);
                } else {
                    ImGui::Text("No image is selected...");
                    if (ImGui::Button("Select image")) {
                        openImage();
                        valuesChanged = true;
                    }
                }
            }

            ImGui::SameLine(viewport->Size.x - menuOffset);
            ImGui::BeginGroup();
            {
                if (ImGui::BeginMenu("Add convolution kernel")) {
                    if (ImGui::BeginMenu("from existing presets")) {
                        for (int n = 0; n < kernelPresets.size(); n++) {
                            if (ImGui::MenuItem(kernelPresets[n].first.c_str())) {
                                convolutionMatrices.push_back(kernelPresets[n]);
                                valuesChanged = true;
                            }
                        }
                        ImGui::EndMenu();
                    }

                    if (ImGui::MenuItem("from new matrix")) {
                        openMatrixCreationWindow = !openMatrixCreationWindow;
                    }

                    ImGui::EndMenu();
                }

                for (int i = convolutionMatrices.size() - 1; i >= 0; i--) {

                    auto kernelName = convolutionMatrices[i].getKernelName();
                    auto popupButtonLabel = "Show Matrix " + kernelName;
                    auto popupCloseButtonLabel = "Close Matrix " + kernelName;
                    auto popupLabel = "Matrix Values " + kernelName;
                    auto tableLabel = "Matrix " + kernelName;
                    auto removeButtonLabel = "Remove " + kernelName;
                    auto enableKernelLabel = "enable " + kernelName + '?';
                    auto equalsName = [kernelName](ConvolutionMatrix convMat) { return convMat.getKernelName() == kernelName; };
                    auto matrix = convolutionMatrices[i].getModifiableKernelMatrix();

                    ImGui::Text("Kernel: %s", kernelName.c_str());

                    auto columnSize = matrix[0].size();
                    if (ImGui::BeginTable(tableLabel.c_str(), columnSize)) {
                        for (int row = 0; row < matrix.size(); row++) {
                            ImGui::TableNextRow();
                            for (int col = 0; col < columnSize; col++) {
                                ImGui::TableSetColumnIndex(col);
                                stringstream strStream;
                                strStream << "##" << kernelName << "_r" << row << "c" << col;
                                std::string label = strStream.str();
                                PARAMETER_CHANGED(ImGui::DragFloat(label.c_str(), &convolutionMatrices[i].getMatrixValue(row, col), 0.005f), valuesChanged);
                            }
                        }
                        ImGui::EndTable();
                    }

                    PARAMETER_CHANGED(ImGui::Checkbox(enableKernelLabel.c_str(), &convolutionMatrices[i].enabled), valuesChanged);

                    ImGui::SameLine();
                    if (ImGui::Button(removeButtonLabel.c_str())) {
                        std::erase_if(convolutionMatrices, equalsName);
                        valuesChanged = true;
                    }
                }
            }
            ImGui::EndGroup();
            if (openMatrixCreationWindow) {
                ImGui::Begin("Matrix Creation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Modal | ImGuiViewportFlags_TopMost);
                ImGui::SetWindowFocus();
                ImGui::Text("Choose width and height of your matrix");
                ImGui::InputText("kernelName", inputKernelName, 32);
                ImGui::SameLine();
                ImGui::Checkbox("use random?", &useRandom);
                ImGui::DragInt("Width", &matWidth, 1, 0, 8);
                ImGui::SameLine();
                ImGui::DragInt("Height", &matHeight, 1, 0, 8);
                if (ImGui::Button("Create")) {
                    if (inputKernelName[0] != 0 && matWidth != 0 && matHeight != 0) {
                        std::vector<float> row(matWidth);
                        std::vector<std::vector<float>> matrix(matHeight, row);
                        if (useRandom) {
                            for (int i = 0; i < matHeight; i++) {
                                for (int j = 0; j < matWidth; j++) {
                                    int number = (1 + std::rand() / ((RAND_MAX + 1u) / 10)) - 5;
                                    matrix[i][j] = number;
                                }
                            }
                        }
                        ConvolutionMatrix convMat(inputKernelName, matrix);
                        convolutionMatrices.push_back(convMat);
                        valuesChanged = true;
                    } else {
                        ImGui::Text("Cant create new matrix. You must enter name and width and height != 0");
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Close")) {
                    openMatrixCreationWindow = false;
                }

                ImGui::End();
            }

            HelpMarker("Control amount of color to be added to the image.");
            ImGui::Text("Color bias");

            PARAMETER_CHANGED(ImGui::ColorEdit3(" ", bias), valuesChanged);

            HelpMarker("Filter color from image.");
            ImGui::Text("Color threshold");
            PARAMETER_CHANGED(ImGui::ColorEdit3("  ", threshold), valuesChanged);

            // if values changed - calculate image convolution using list of kernels

            if (img && valuesChanged && leftMouseButton == GLFW_RELEASE) {

                imgResult = new Image(*img);

                ImGui::Text("Calculating image convolution");

                auto colorBias = lerp(bias);
                auto colorThreshold = lerp(threshold);

                ConvolutionMatrix::filter(convolutionMatrices, *imgResult, colorBias, colorThreshold);

                valuesChanged = false;

                if (loadedTexture) {
                    loadedTexture = (void*)(intptr_t)loadTexture();
                }
            }

            ImGui::End();

            ImGui::Render();
            ImGui::UpdatePlatformWindows();
            glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);
            glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
        glfwTerminate();
    }

    void
    clear()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};
