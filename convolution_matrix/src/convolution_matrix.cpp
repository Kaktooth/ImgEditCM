
// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "convolution_matrix.h"
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>

void ConvolutionMatrix::filter(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redChannelThreshold, int greenChannelThreshold, int blueChannelThreshold)
{
    if (!enabled)
        return;

    std::stringstream stream;
    std::vector<unsigned char> pixelBuffer;
    auto pixels = image.getPixels();
    auto channels = image.getChannels();
    auto width = image.getWidth();
    auto height = image.getHeight();

    const bool haveAlpha = channels >= 4;

    unsigned __int8 halfKernelWidth = kernel.size() / 2;
    unsigned __int8 halfKernelHeight = kernel[0].size() / 2;
    unsigned __int8 kernelRemainderWidth = kernel.size() - halfKernelWidth;
    unsigned __int8 kernelRemainderHeight = kernel[0].size() - halfKernelHeight;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            __int16 calculatedRedChannel = redChannelBias;
            __int16 calculatedGreenChannel = greenChannelBias;
            __int16 calculatedBlueChannel = blueChannelBias;

            for (int ki = w - halfKernelWidth <= 0 ? halfKernelWidth - w : -halfKernelWidth; ki < kernelRemainderWidth && width >= w + kernelRemainderWidth; ki++) {
                for (int kj = h - halfKernelHeight <= 0 ? halfKernelHeight - h : -halfKernelHeight; kj < kernelRemainderHeight && height >= h + kernelRemainderHeight; kj++) {
                    unsigned char* pixelOffset = pixels + ((w + ki) + width * (h + kj)) * channels;
                    unsigned char r = pixelOffset[0];
                    unsigned char g = pixelOffset[1];
                    unsigned char b = pixelOffset[2];

                    auto kernelWidth = ki + halfKernelWidth;
                    auto kernelHeight = kj + halfKernelHeight;
                    calculatedRedChannel += r * kernel[kernelWidth][kernelHeight];
                    calculatedGreenChannel += g * kernel[kernelWidth][kernelHeight];
                    calculatedBlueChannel += b * kernel[kernelWidth][kernelHeight];
                }
            }

            // pixel normalization

            if (calculatedRedChannel > redChannelThreshold) {
                calculatedRedChannel = std::lerp(0, 255, calculatedRedChannel / 255);
            } else if (calculatedRedChannel < 0) {
                calculatedRedChannel = 0;
            }
            if (calculatedGreenChannel > greenChannelThreshold) {
                calculatedGreenChannel = std::lerp(0, 255, calculatedGreenChannel / 255);
            } else if (calculatedGreenChannel < 0) {
                calculatedGreenChannel = 0;
            }
            if (calculatedBlueChannel > blueChannelThreshold) {
                calculatedBlueChannel = std::lerp(0, 255, calculatedBlueChannel / 255);
            } else if (calculatedBlueChannel < 0) {
                calculatedBlueChannel = 0;
            }

            pixelBuffer.push_back(calculatedRedChannel);
            pixelBuffer.push_back(calculatedGreenChannel);
            pixelBuffer.push_back(calculatedBlueChannel);

            if (haveAlpha) {
                pixelBuffer.push_back((pixels + ((w) + width * (h)) * channels)[3]);
            }
        }
    }
    image.setPixels(pixelBuffer.data());
}

std::string ConvolutionMatrix::getKernelName()
{
    return kernelName;
}

std::vector<std::vector<float>> ConvolutionMatrix::getKernelMatrix()
{
    return kernel;
}

std::vector<std::vector<float>>& ConvolutionMatrix::getModifiableKernelMatrix()
{
    return kernel;
}

float& ConvolutionMatrix::getMatrixValue(int row, int col)
{
    return kernel[row][col];
}

bool ConvolutionMatrix::isEnabled()
{
    return enabled;
}
