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

Image::Image()
{
}

Image::Image(Image& img)
{
    channels = img.channels;
    width = img.width;
    height = img.height;
    pixels = (unsigned char*)malloc(img.width * img.height * img.channels * sizeof(char));
    memcpy(pixels, img.pixels, img.width * img.height * img.channels * sizeof(char));
    // structuredPixels = (unsigned char*)malloc(img.width * img.height * img.channels * sizeof(char));
    // memcpy(structuredPixels, img.structuredPixels, img.width * img.height * img.channels * sizeof(char));
}

Image::Image(const char* filePath)
{
    load(filePath);
}

void Image::load(const char* filePath)
{
    pixels = stbi_load(filePath, &width, &height, &channels, 0);
    // use this structure only for parallel method
    // structuredPixels = toRRGGBB(pixels);
}

// convert RRRGGGBBB structure to RGBRGBRGB
unsigned char* Image::toRRGGBB(unsigned char* rgb)
{
    int pixelsNumber = width * height;
    unsigned char* pixels = (unsigned char*)malloc(pixelsNumber * channels * sizeof(char));
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {

            unsigned char* offset = rgb + (w + width * h) * channels;

            unsigned char* redOffset = pixels + (w + width * h);
            unsigned char* greenOffset = pixels + (w + width * h) + pixelsNumber;
            unsigned char* blueOffset = pixels + (w + width * h) + pixelsNumber * 2;

            memcpy(&redOffset[0], &offset[0], sizeof(char));
            memcpy(&greenOffset[0], &offset[1], sizeof(char));
            memcpy(&blueOffset[0], &offset[2], sizeof(char));
        }
    }

    return pixels;
}

// convert from RGBRGBRGB structure to RRRGGGBBB
unsigned char* Image::toRGB(unsigned char* rrggbb)
{
    int pixelsNumber = width * height;
    unsigned char* pixels = (unsigned char*)malloc(pixelsNumber * channels * sizeof(char));

    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            unsigned char* newOffset = pixels + (w + width * h) * channels;

            unsigned char* redOffset = rrggbb + (w + width * h);
            unsigned char* greenOffset = rrggbb + (w + width * h) + pixelsNumber;
            unsigned char* blueOffset = rrggbb + (w + width * h) + pixelsNumber * 2;

            memcpy(&newOffset[0], &redOffset[0], sizeof(char));
            memcpy(&newOffset[1], &greenOffset[0], sizeof(char));
            memcpy(&newOffset[2], &blueOffset[0], sizeof(char));
        }
    }

    return pixels;
}

void Image::save(const char* saveResultPath, IMAGE_FORMAT imageFormat)
{
    // use for parallel method
    // pixels = toRGB(structuredPixels);
    switch (imageFormat) {
    case JPG:
        stbi_write_jpg(saveResultPath, width, height, channels, pixels, 80);
        break;
    case PNG:
        stbi_write_png(saveResultPath, width, height, channels, pixels, width * channels);
        break;
    case BMP:
        stbi_write_bmp(saveResultPath, width, height, channels, pixels);
        break;
    default:
        break;
    }
    stbi_image_free(pixels);
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
    // use for parallel method
    // memcpy(this->pixels, toRGB(pixels), width * height * channels * sizeof(char));

    memcpy(this->pixels, pixels, width * height * channels * sizeof(char));
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
