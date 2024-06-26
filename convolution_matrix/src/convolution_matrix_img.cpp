// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "convolution_matrix_img.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

namespace img_edit_cm {

Image::Image() = default;

Image::Image(Image& img)
{
    channels = img.channels;
    width = img.width;
    height = img.height;
    pixels = (unsigned char*)malloc(img.width * img.height * img.channels * sizeof(char));
    memcpy(pixels, img.pixels, img.width * img.height * img.channels * sizeof(char));
    structuredPixels = (unsigned char*)malloc(img.width * img.height * img.channels * sizeof(char));
    memcpy(structuredPixels, img.structuredPixels, img.width * img.height * img.channels * sizeof(char));
}

Image::Image(const char* filePath)
{
    load(filePath);
}

void Image::load(const char* filePath)
{
    pixels = stbi_load(filePath, &width, &height, &channels, 0);
    structuredPixels = toRRGGBB(pixels);
}

// convert RRRGGGBBB structure to RGBRGBRGB
unsigned char* Image::toRRGGBB(unsigned char* rgb)
{
    int pixelsNumber = width * height;
    unsigned char* pixels = (unsigned char*)malloc(pixelsNumber * channels * sizeof(char));
    for (int channel = 0; channel < channels; channel++) {
        int channelOffset = pixelsNumber * channel;
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < height; h++) {
                unsigned char* offset = rgb + (w + width * h) * channels;
                unsigned char* writeOffset = pixels + (w + width * h) + channelOffset;

                memcpy(&writeOffset[0], &offset[channel], sizeof(char));
            }
        }
    }

    return pixels;
}

// convert from RGBRGBRGB structure to RRRGGGBBB
unsigned char* Image::toRGB(unsigned char* rrggbb)
{
    int pixelsNumber = width * height;
    unsigned char* pixels = (unsigned char*)malloc(pixelsNumber * channels * sizeof(char));
    for (int channel = 0; channel < channels; channel++) {
        int channelOffset = pixelsNumber * channel;
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < height; h++) {
                unsigned char* newOffset = pixels + (w + width * h) * channels;
                unsigned char* writeOffset = rrggbb + (w + width * h) + channelOffset;

                memcpy(&newOffset[channel], &writeOffset[0], sizeof(char));
            }
        }
    }

    return pixels;
}

void Image::save(std::filesystem::path saveResultPath)
{
    pixels = toRGB(structuredPixels);
    auto imageFormat = saveResultPath.extension().generic_string();
    auto savePath = saveResultPath.generic_string();
    if (imageFormat == ".jpg") {
        stbi_write_jpg(savePath.c_str(), width, height, channels, pixels, 80);
    } else if (imageFormat == ".png") {
        stbi_write_png(savePath.c_str(), width, height, channels, pixels, width * channels);
    }
}

unsigned char* Image::resize(int width, int height)
{
    unsigned char* image_output = (unsigned char*)malloc(width * height * channels * sizeof(char));
    stbir_resize_uint8(pixels, Image::width, Image::height, 0, image_output, width,
        height, 0, channels);
    return image_output;
}

void Image::setPixels(unsigned char* pixels)
{
    memcpy(this->pixels, pixels, width * height * channels * sizeof(char));
    memcpy(this->structuredPixels, toRRGGBB(pixels), width * height * channels * sizeof(char));
}

void Image::setStructurePixels(unsigned char* pixels)
{
    memcpy(this->pixels, toRGB(pixels), width * height * channels * sizeof(char));
    memcpy(this->structuredPixels, pixels, width * height * channels * sizeof(char));
}

unsigned char* Image::getPixels()
{
    return (unsigned char*)pixels;
}

unsigned char* Image::getStructuredPixels()
{
    return (unsigned char*)structuredPixels;
}

int Image::getWidth()
{
    return width;
}

int Image::getHeight()
{
    return height;
}

int Image::getChannels()
{
    return channels;
}
}
