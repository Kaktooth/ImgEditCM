
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

// TODO fix alignment when loading (loads 16 elements and converts to 8 elements and skips 8 elements)
// void ConvolutionMatrix::filter_p(Image& image, int redChannelBias, int greenChannelBias, int blueChannelBias, int redChannelThreshold, int greenChannelThreshold, int blueChannelThreshold)
//{
//    if (!enabled)
//        return;
//
//    std::stringstream stream;
//    std::vector<unsigned char> pixelBuffer;
//    auto pixels = image.getStructuredPixels();
//    auto channels = image.getChannels();
//    auto width = image.getWidth();
//    auto height = image.getHeight();
//
//    const bool haveAlpha = channels >= 4;
//
//    unsigned __int8 halfKernelWidth = kernel.size() / 2;
//    unsigned __int8 halfKernelHeight = kernel[0].size() / 2;
//    unsigned __int8 kernelRemainderWidth = kernel.size() - halfKernelWidth;
//    unsigned __int8 kernelRemainderHeight = kernel[0].size() - halfKernelHeight;
//
//    alignas(32) __m256 kernelSimd[kernel.size()];
//
//    for (int i = 0; i < kernel.size(); i++) {
//        kernelSimd[i] = _mm256_loadu_ps(kernel[i].data());
//        /* float f[8];
//         _mm256_store_ps(f, kernelSimd[i]);
//         std::cout << f[0] << " | " << f[1] << " | " << f[2] << " | " << f[3] << " | "
//                   << f[4] << " | " << f[5] << " | " << f[6] << " | " << f[7] << " | " << std::endl;*/
//    }
//
//    __m256i redChannelBiasVector = _mm256_set1_epi32(redChannelBias);
//    __m256i greenChannelBiasVector = _mm256_set1_epi32(greenChannelBias);
//    __m256i blueChannelBiasVector = _mm256_set1_epi32(blueChannelBias);
//    __m256 pixelValues = _mm256_set1_ps(255);
//    __m256i zeroes = _mm256_setzero_si256();
//    __m256 zeroes2 = _mm256_setzero_ps();
//    __m256 ones = _mm256_set1_ps(1);
//    for (int h = 0; h < height; h++) {
//        for (int w = 0; w < width; w++) {
//
//            __m256 rowSum = _mm256_setzero_ps();
//            __m256 calculatedRow = _mm256_setzero_ps();
//
//            for (int i = 0; i < kernel.size(); i++) {
//
//                if (h + i > height) {
//                    continue;
//                }
//                unsigned char* offset = pixels + (w * kernel.size() + h + i);
//
//                __m256i loadedPixels = _mm256_cvtepu8_epi32(_mm_load_si128((__m128i*)&offset));
//                __m256 pixelRow = _mm256_cvtepi32_ps(loadedPixels);
//
//                calculatedRow = _mm256_mul_ps(pixelRow, kernelSimd[i]);
//                rowSum = _mm256_add_ps(calculatedRow, rowSum);
//            }
//
//            __m256 bias = _mm256_set1_ps(redChannelBias);
//            rowSum = _mm256_add_ps(rowSum, bias);
//
//            //// pixel color normalization
//            __m256 threshold = _mm256_set1_ps(redChannelThreshold);
//
//            __m256 isGreaterThanThreshold = _mm256_cmp_ps(rowSum, threshold, _CMP_GT_OS);
//            __m256 isGraterThanZero = _mm256_cmp_ps(rowSum, zeroes2, _CMP_GT_OS);
//            __m256 isLessThanThreshold = _mm256_cmp_ps(rowSum, threshold, _CMP_LE_OS);
//            __m256 isGreaterThanThresholdAndLessThanZero = _mm256_and_ps(isGraterThanZero, isLessThanThreshold);
//
//            __m256 greaterValues = _mm256_and_ps(rowSum, isGreaterThanThreshold);
//            __m256 maskedPixelValues = _mm256_and_ps(pixelValues, isGreaterThanThreshold);
//            __m256 scaledValues = _mm256_div_ps(greaterValues, maskedPixelValues);
//            __m256 normalizedValues = _mm256_div_ps(ones, scaledValues);
//            normalizedValues = _mm256_mul_ps(normalizedValues, maskedPixelValues);
//
//            __m256 normalValues = _mm256_and_ps(rowSum, isGreaterThanThresholdAndLessThanZero);
//            __m256 mergedValues = _mm256_blendv_ps(normalizedValues, normalValues, isGreaterThanThresholdAndLessThanZero);
//
//            float sum[kernel.size()];
//            _mm256_storeu_ps(sum, mergedValues);
//
//            pixelBuffer.push_back(sum[0]);
//            pixelBuffer.push_back(sum[1]);
//            pixelBuffer.push_back(sum[2]);
//
//            if (haveAlpha) {
//                pixelBuffer.push_back((pixels + ((w) + width * (h)) * channels)[3]);
//            }
//        }
//    }
//
//    image.setPixels(pixelBuffer.data());
//}

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
