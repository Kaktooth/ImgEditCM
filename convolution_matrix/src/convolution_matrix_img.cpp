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

Image::Image() = default;

Image::Image(Image& img)
{
    channels = img.channels;
    width = img.width;
    height = img.height;
    pixels = (unsigned char*)malloc(img.width * img.height * img.channels * sizeof(char));
    memcpy(pixels, img.pixels, img.width * img.height * img.channels * sizeof(char));
}

Image::Image(const char* filePath)
{
    load(filePath);
}

void Image::load(const char* filePath)
{
    pixels = stbi_load(filePath, &width, &height, &channels, 0);
}

void Image::save(std::filesystem::path saveResultPath)
{
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
}

unsigned char* Image::getPixels()
{
    return (unsigned char*)pixels;
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
