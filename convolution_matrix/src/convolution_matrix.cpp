
// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#define CPU_EXTENTIONS_SUPPORTED (__builtin_cpu_supports("avx2"))

#include "convolution_matrix.h"

namespace img_edit_cm {

void ConvolutionMatrix::filter(std::vector<ConvolutionMatrix> convolutionMatrices, Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold, std::array<int, 2> dilation)
{
    for (auto convolutionMatrix : convolutionMatrices) {
        convolutionMatrix.applyKernelDilation(dilation);
#ifdef CPU_EXTENTIONS_SUPPORTED
        convolutionMatrix.parallel_filter(image, colorBias, colorThreshold);
#else
        convolutionMatrix.sequential_filter(image, colorBias, colorThreshold);
#endif
    }
}

void ConvolutionMatrix::sequential_filter(Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold)
{
    if (!enabled)
        return;

    std::vector<unsigned char> pixelBuffer;
    unsigned char* pixels = image.getPixels();
    unsigned __int16 channels = image.getChannels();
    unsigned __int16 width = image.getWidth();
    unsigned __int16 height = image.getHeight();

    const bool haveAlpha = channels >= 4;

    unsigned __int8 halfKernelWidth = kernel[0].size() / 2;
    unsigned __int8 halfKernelHeight = kernel.size() / 2;
    unsigned __int8 kernelRemainderWidth = kernel[0].size() - halfKernelWidth;
    unsigned __int8 kernelRemainderHeight = kernel.size() - halfKernelHeight;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            __int16 calculatedRedChannel = colorBias[0];
            __int16 calculatedGreenChannel = colorBias[1];
            __int16 calculatedBlueChannel = colorBias[2];

            for (int ki = w - halfKernelWidth <= 0 ? halfKernelWidth - w : -halfKernelWidth; ki < kernelRemainderWidth && width >= w + kernelRemainderWidth; ki++) {
                for (int kj = h - halfKernelHeight <= 0 ? halfKernelHeight - h : -halfKernelHeight; kj < kernelRemainderHeight && height >= h + kernelRemainderHeight; kj++) {
                    unsigned char* pixelOffset = pixels + (w + ki + width * (h + kj)) * channels;
                    auto r = pixelOffset[0];
                    auto g = pixelOffset[1];
                    auto b = pixelOffset[2];

                    auto kernelWidth = ki + halfKernelWidth;
                    auto kernelHeight = kj + halfKernelHeight;
                    calculatedRedChannel += r * kernel[kernelHeight][kernelWidth];
                    calculatedGreenChannel += g * kernel[kernelHeight][kernelWidth];
                    calculatedBlueChannel += b * kernel[kernelHeight][kernelWidth];
                }
            }

            // pixel normalization

            if (calculatedRedChannel > colorThreshold[0]) {
                calculatedRedChannel = std::lerp(0, 255, calculatedRedChannel / 255);
            } else if (calculatedRedChannel < 0) {
                calculatedRedChannel = 0;
            }
            if (calculatedGreenChannel > colorThreshold[1]) {
                calculatedGreenChannel = std::lerp(0, 255, calculatedGreenChannel / 255);
            } else if (calculatedGreenChannel < 0) {
                calculatedGreenChannel = 0;
            }
            if (calculatedBlueChannel > colorThreshold[2]) {
                calculatedBlueChannel = std::lerp(0, 255, calculatedBlueChannel / 255);
            } else if (calculatedBlueChannel < 0) {
                calculatedBlueChannel = 0;
            }

            pixelBuffer.push_back(calculatedRedChannel);
            pixelBuffer.push_back(calculatedGreenChannel);
            pixelBuffer.push_back(calculatedBlueChannel);

            if (haveAlpha) {
                pixelBuffer.push_back((pixels + (w + width * h) * channels)[3]);
            }
        }
    }
    image.setPixels(pixelBuffer.data());
}

void ConvolutionMatrix::parallel_filter(Image& image, std::array<int, 3> colorBias, std::array<int, 3> colorThreshold)
{
    if (!enabled)
        return;

    const int BATCH_SIZE = 8;
    std::vector<unsigned char> pixelBuffer;
    unsigned char* pixels = image.getStructuredPixels();
    unsigned __int16 channels = image.getChannels();
    unsigned __int16 width = image.getWidth();
    unsigned __int16 height = image.getHeight();

    const bool haveAlpha = channels >= 4;

    __m256 redBias = _mm256_set1_ps(colorBias[0]);
    __m256 greenBias = _mm256_set1_ps(colorBias[1]);
    __m256 blueBias = _mm256_set1_ps(colorBias[2]);
    __m256 redThreshold = _mm256_set1_ps(colorThreshold[0]);
    __m256 greenThreshold = _mm256_set1_ps(colorThreshold[1]);
    __m256 blueThreshold = _mm256_set1_ps(colorThreshold[2]);
    __m256 pixelValues = _mm256_set1_ps(255);
    __m256 zeroes = _mm256_setzero_ps();
    __m256 ones = _mm256_set1_ps(1);

    __m256 kernelSimd[kernel.size()];

    __m256 rowSum[BATCH_SIZE];

    unsigned __int8 halfKernelWidth = kernel[0].size() / 2;
    unsigned __int8 halfKernelHeight = kernel.size() / 2;

    for (int channel = 0; channel < 3; channel++) {
        int channelOffset = width * height * channel;

        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w += BATCH_SIZE) {
                // TODO fix bug with image offset. When image is filtered last line of pixels is incorrect.
                unsigned char* offset = pixels + (w + h * width);
                float rows[BATCH_SIZE][8];
                float pixelsResult[BATCH_SIZE] = { 0 };

                for (int p = 0; p < BATCH_SIZE && p + w < width; p++) {
                    rowSum[p] = _mm256_setzero_ps();

                    for (int kh = 0; kh < kernel.size() && kh < height - h; kh++) {
                        kernelSimd[kh] = _mm256_loadu_ps(kernel[kh].data());
                        __m256i loadedPixels = _mm256_cvtepu8_epi32(_mm_lddqu_si128((__m128i*)&offset[channelOffset + p + kh * width]));
                        __m256 pixelRow = _mm256_cvtepi32_ps(loadedPixels);
                        __m256 calculatedRow = _mm256_mul_ps(kernelSimd[kh], pixelRow);
                        rowSum[p] = _mm256_add_ps(rowSum[p], calculatedRow);
                    }

                    // sum the vector in width
                    _mm256_storeu_ps(rows[p], rowSum[p]);
                    for (int kh = 0; kh < kernel[0].size(); kh++) {
                        pixelsResult[p] += rows[p][kh];
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

                for (int i = 0; i < BATCH_SIZE && i + w < width; i++) {
                    pixelBuffer.push_back(sum[i]);
                }
            }
        }
    }

    if (haveAlpha) {
        unsigned char* offset = pixels + width * height * 3;
        std::vector<unsigned char> alphaChannel(offset, offset + width * height);

        pixelBuffer.insert(pixelBuffer.end(), alphaChannel.begin(), alphaChannel.end());
    }

    image.setStructurePixels(pixelBuffer.data());
}

void ConvolutionMatrix::applyKernelDilation(std::array<int, 2> dilation)
{
    int heightDilation = dilation[0];
    int widthDilation = dilation[1];

    if (heightDilation > 0 || widthDilation > 0) {
        int kernelHeight = kernel[0].size();
        int newKernelWidth = kernel[0].size() * widthDilation;
        int newKernelHeight = kernel.size() * heightDilation;
        int kernelDilationEncounters = 1;
        for (int k = 0; k < kernelHeight; k++) {
            for (int kernelWidth = 1; kernelWidth < newKernelWidth + 1; kernelWidth += widthDilation + 1) {
                for (int d = 0; d < widthDilation; d++) {
                    kernel[k].insert(kernel[k].begin() + kernelWidth + d, 0);
                }
            }
        }

        std::vector<float> zeroes(kernel[0].size());
        while (kernelDilationEncounters < newKernelHeight) {
            for (int d = 0; d < heightDilation; d++) {
                kernel.insert(kernel.begin() + kernelDilationEncounters + d, zeroes);
            }
            kernelDilationEncounters += heightDilation + 1;
        }
    }
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
