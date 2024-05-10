#pragma once
#include <filesystem>

namespace img_edit_cm {
class Image {

private:
    int width, height, channels;
    unsigned char* pixels;
    unsigned char* structuredPixels;

    unsigned char* toRRGGBB(unsigned char* rgb);
    unsigned char* toRGB(unsigned char* rrggbb);

public:
    Image();
    Image(Image& img);
    Image(const char* filePath);
    void load(const char* filePath);
    void save(std::filesystem::path saveResultPath);
    unsigned char* resize(int width, int height);
    void setPixels(unsigned char* pixels);
    void Image::setStructurePixels(unsigned char* pixels);
    unsigned char* getPixels();
    unsigned char* getStructuredPixels();
    int getWidth();
    int getHeight();
    int getChannels();
};
}
