#pragma once

enum IMAGE_FORMAT {
    JPG,
    PNG,
    BMP,
    IMAGE_FORMAT_MAX = BMP
};
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
    void save(const char* saveResultPath, IMAGE_FORMAT imageFormat);
    unsigned char* resize(int width, int height);
    void setPixels(unsigned char* pixels);
    unsigned char* getPixels();
    unsigned char* getStructuredPixels();
    int getWidth();
    int getHeight();
    int getChannels();
};
