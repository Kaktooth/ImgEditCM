#pragma once
// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "convolution_matrix_img.h"
#include <immintrin.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

    void filter(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redThreshold, int greenThreshold, int blueThreshold);
    void filter_p(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redThreshold, int greenThreshold, int blueThreshold);
    std::string getKernelName();
    std::vector<std::vector<float>> getKernelMatrix();
    std::vector<std::vector<float>>& getModifiableKernelMatrix();
    float& getMatrixValue(int row, int col);
    bool isEnabled();
};
