
// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#define CPU_EXTENTIONS_SUPPORTED (__builtin_cpu_supports("avx2"))

#include "convolution_matrix.h"
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>

namespace img_edit_cm {

void ConvolutionMatrix::filter(std::vector<ConvolutionMatrix> convolutionMatrices, Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold)
{
    for (auto convolutionMatrix : convolutionMatrices) {

#ifdef CPU_EXTENTIONS_SUPPORTED
        convolutionMatrix.parallel_filter(image, colorBias[0], colorBias[1], colorBias[2], colorThreshold[0], colorThreshold[1], colorThreshold[2]);
#else
        convolutionMatrix.standard_filter(image, colorBias[0], colorBias[1], colorBias[2], colorThreshold[0], colorThreshold[1], colorThreshold[2]);
#endif
    }
}

void ConvolutionMatrix::standard_filter(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redChannelThreshold, int greenChannelThreshold, int blueChannelThreshold)
{
    if (!enabled)
        return;

    std::vector<unsigned char> pixelBuffer;
    unsigned char* pixels = image.getPixels();
    unsigned __int8 channels = image.getChannels();
    unsigned __int8 width = image.getWidth();
    unsigned __int8 height = image.getHeight();

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
                    auto r = pixelOffset[0];
                    auto g = pixelOffset[1];
                    auto b = pixelOffset[2];

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

void ConvolutionMatrix::parallel_filter(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redChannelThreshold, int greenChannelThreshold, int blueChannelThreshold)
{
    if (!enabled)
        return;

    const int BATCH_SIZE = 8;
    std::vector<unsigned char> pixelBuffer;
    unsigned char* pixels = image.getStructuredPixels();
    unsigned __int8 channels = image.getChannels();
    unsigned __int8 width = image.getWidth();
    unsigned __int8 height = image.getHeight();

    const bool haveAlpha = channels >= 4;

    __m256 redBias = _mm256_set1_ps(redChannelBias);
    __m256 greenBias = _mm256_set1_ps(greenChannelBias);
    __m256 blueBias = _mm256_set1_ps(blueChannelBias);
    __m256 redThreshold = _mm256_set1_ps(redChannelThreshold);
    __m256 greenThreshold = _mm256_set1_ps(greenChannelThreshold);
    __m256 blueThreshold = _mm256_set1_ps(blueChannelThreshold);
    __m256 pixelValues = _mm256_set1_ps(255);
    __m256 zeroes = _mm256_setzero_ps();
    __m256 ones = _mm256_set1_ps(1);

    __m256 kernelSimd[kernel.size()];

    __m256 rowSum[BATCH_SIZE];

    int imageSize = width * height * channels;

    for (int channel = 0; channel < channels; channel++) {
        int channelOffset = width * height * channel;

        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w += BATCH_SIZE) {
                // TODO fix bug with image offset. When image is filtered last line of pixels is incorrect.
                unsigned char* offset = pixels + (w - 1 + h * width);
                float rows[BATCH_SIZE][8];
                float pixelsResult[BATCH_SIZE] = { 0 };

                for (int p = 0; p < BATCH_SIZE; p++) {
                    rowSum[p] = _mm256_setzero_ps();

                    for (int i = 0; i < kernel.size(); i++) {
                        kernelSimd[i] = _mm256_loadu_ps(kernel[i].data());

                        __m256i loadedPixels = _mm256_cvtepu8_epi32(_mm_lddqu_si128((__m128i*)&offset[channelOffset + p]));
                        __m256 pixelRow = _mm256_cvtepi32_ps(loadedPixels);
                        __m256 calculatedRow = _mm256_mul_ps(kernelSimd[i], pixelRow);
                        rowSum[p] = _mm256_add_ps(rowSum[p], calculatedRow);
                    }

                    _mm256_storeu_ps(rows[p], rowSum[p]);
                    for (int ki = 0; ki < kernel.size(); ki++) {
                        pixelsResult[p] += rows[p][ki];
                    }
                }

                __m256 pixelResult = _mm256_set_ps(pixelsResult[7], pixelsResult[6], pixelsResult[5], pixelsResult[4], pixelsResult[3], pixelsResult[2], pixelsResult[1], pixelsResult[0]);

                switch (channel) {
                case 0:
                    pixelResult = _mm256_add_ps(pixelResult, redBias);
                    break;
                case 1:
                    pixelResult = _mm256_add_ps(pixelResult, greenBias);
                    break;
                case 2:
                    pixelResult = _mm256_add_ps(pixelResult, blueBias);
                    break;
                }

                __m256 threshold;
                switch (channel) {
                case 0:
                    threshold = redThreshold;
                    break;
                case 1:
                    threshold = greenThreshold;
                    break;
                case 2:
                    threshold = blueThreshold;
                    break;
                }

                __m256 isGreaterThanThreshold = _mm256_cmp_ps(pixelResult, threshold, _CMP_GT_OS);
                __m256 isGraterThanZero = _mm256_cmp_ps(pixelResult, zeroes, _CMP_GT_OS);
                __m256 isLessThanThreshold = _mm256_cmp_ps(pixelResult, threshold, _CMP_LE_OS);
                __m256 isGreaterThanThresholdAndLessThanZero = _mm256_and_ps(isGraterThanZero, isLessThanThreshold);

                __m256 greaterValues = _mm256_and_ps(pixelResult, isGreaterThanThreshold);
                __m256 maskedPixelValues = _mm256_and_ps(pixelValues, isGreaterThanThreshold);

                // pixel color normalization
                __m256 scaledValues = _mm256_div_ps(greaterValues, maskedPixelValues);
                __m256 normalizedValues = _mm256_div_ps(ones, scaledValues);
                normalizedValues = _mm256_mul_ps(normalizedValues, maskedPixelValues);

                __m256 normalValues = _mm256_and_ps(pixelResult, isGreaterThanThresholdAndLessThanZero);
                __m256 mergedValues = _mm256_blendv_ps(normalizedValues, normalValues, isGreaterThanThresholdAndLessThanZero);

                float sum[kernel.size()];
                _mm256_storeu_ps(sum, mergedValues);

                for (int i = 0; i < BATCH_SIZE; i++) {
                    pixelBuffer.push_back(sum[i]);
                }
            }
        }
    }

    image.setStructurePixels(pixelBuffer.data());
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
}
