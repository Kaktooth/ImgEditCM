#pragma once
// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "convolution_matrix_img.h"
#include <array>
#include <immintrin.h>
#include <string>
#include <vector>

namespace img_edit_cm {
class ConvolutionMatrix {

private:
    std::string kernelName;
    std::vector<std::vector<float>> kernel;

public:
    bool enabled = true;

    ConvolutionMatrix(std::string kernelName, std::vector<std::vector<float>> kernel)
    {
        this->kernelName = kernelName;
        this->kernel = kernel;
    }
    ConvolutionMatrix(std::pair<std::string, std::vector<std::vector<float>>> kernel)
    {
        this->kernelName = kernel.first;
        this->kernel = kernel.second;
    }

    static void filter(std::vector<ConvolutionMatrix> convolutionMatrices, Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold, std::array<int, 2> dilation);
    void applyKernelDilation(std::array<int, 2> dilation);
    void sequential_filter(Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold);
    void parallel_filter(Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold);
    std::string getKernelName();
    std::vector<std::vector<float>> getKernelMatrix();
    std::vector<std::vector<float>>& getModifiableKernelMatrix();
    float& getMatrixValue(int row, int col);
    bool isEnabled();
};
}
