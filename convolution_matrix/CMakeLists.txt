# CMakeList.txt : CMake project for ConvolutionMatrix, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.21)
SET(CMAKE_CXX_STANDARD 20)

find_package(Stb REQUIRED)
add_library(img_edit_cm STATIC "src/convolution_matrix.cpp" "src/convolution_matrix.h" "src/convolution_matrix_img.h" "src/convolution_matrix_img.cpp")
target_include_directories(img_edit_cm PRIVATE ${Stb_INCLUDE_DIR})


find_package(glfw3 REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
add_executable(img_edit_cm_test WIN32 "src/test.cpp")
target_link_libraries(img_edit_cm_test PRIVATE img_edit_cm OpenGL::GL glfw imgui::imgui)
target_include_directories(img_edit_cm_test PRIVATE ${OPENGL_INCLUDE_DIR})