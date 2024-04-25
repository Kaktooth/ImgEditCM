#pragma once
#include <filesystem>

class Image {

private:
    int width, height, channels;
    unsigned char* pixels;

public:
    Image();
    Image(Image& img);
    Image(const char* filePath);
    void load(const char* filePath);
    void save(std::filesystem::path saveResultPath);
    unsigned char* resize(int width, int height);
    void setPixels(unsigned char* pixels);
    unsigned char* getPixels();
    int getWidth();
    int getHeight();
    int getChannels();
};
