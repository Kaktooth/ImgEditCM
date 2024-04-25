#include <map>
#include <string>
#include <vector>

struct KernelPresets {

    const std::vector<std::vector<float>> sharpen = {
        { 0, 0, 0, 0, 0 },
        { 0, 0, -1, 0, 0 },
        { 0, -1, 5, -1, 0 },
        { 0, 0, -1, 0, 0 },
        { 0, 0, 0, 0, 0 }
    };

    const std::vector<std::vector<float>> boxBlur = {
        { 0.111f, 0.111f, 0.111f },
        { 0.111f, 0.111f, 0.111f },
        { 0.111f, 0.111f, 0.111f }
    };

    const std::vector<std::vector<float>> gaussianBlur = {
        { 0.0625, 0.125, 0.0625 },
        { 0.125, 0.25, 0.125 },
        { 0.0625, 0.125, 0.0625 },
    };

    const std::vector<std::vector<float>> enchanceLine = {
        { 0, 0, 0 },
        { -1, 1, 0 },
        { 0, 0, 0 }
    };

    const std::vector<std::vector<float>> laplacianEdgeDetect = {
        { 0, 1, 0 },
        { 1, -4, 1 },
        { 0, 1, 0 }
    };

    const std::vector<std::vector<float>> emboss = {
        { -2, -1, 0 },
        { -1, 1, 1 },
        { 0, 1, 2 }
    };

    const std::vector<std::vector<float>> lineEmboss = {
        { 0.165f, 2.445f, 4.415f, 0.0f, 4.220f, 0.235f, -2.0f },
        { -1.35f, 0.0f, -2.075f, -4.0f, 0.0f, 0.0f, -2.0f },
    };

    const std::vector<std::vector<float>> lineEmboss2 = {
        {
            -2.0f,
            -1.0f,
            0.0f,
            1.0f,
            2.0f,
        },
        { 0.061f, 0.245f, 0.388f, 0.245f, 0.061f },
    };

    const std::vector<std::vector<float>> bigKernelEmboss = {
        { 0, 0, 0, -1, 0, 0, 0 },
        { 0, 0, -1, 2, -1, 0, 0 },
        { 0, -1, 2, 1, 2, -1, 0 },
        { 0, 0, -1, 2, -1, 0, 0 },
        { 0, 0, 0, -1, 0, 0, 0 }
    };

    const std::vector<std::vector<float>> bottomSobel = {
        { -1, -2, -1 },
        { 0, 0, 0 },
        { 1, 2, 1 }
    };

    const std::vector<std::vector<float>> leftSobel = {
        { 1, 0, -1 },
        { 2, 0, -2 },
        { 1, 0, -1 }
    };

    const std::vector<std::vector<float>> rightSobel = {
        { -1, 0, 1 },
        { -2, 0, 2 },
        { -1, 0, 1 }
    };

    const std::vector<std::vector<float>> topSobel = {
        { 1, 2, 1 },
        { 0, 0, 0 },
        { -1, -2, -1 }
    };

    const std::vector<std::vector<float>> identity = {
        { 0, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 0 }
    };

    const std::vector<std::vector<float>> halfIntencity = {
        { 0, 0, 0 },
        { 0, 0.5, 0 },
        { 0, 0, 0 }
    };

    const std::vector<std::vector<float>> zeroesKernel = {
        { 0, 0, 0 },
        { 0, 0, 0 },
        { 0, 0, 0 }
    };
} kernelPreset;

std::vector<std::pair<std::string, std::vector<std::vector<float>>>> kernelPresets {
    { "sharpen", kernelPreset.sharpen },
    { "boxBlur", kernelPreset.boxBlur },
    { "gaussianBlur", kernelPreset.gaussianBlur },
    { "enchanceLine", kernelPreset.enchanceLine },
    { "laplacianEdgeDetect", kernelPreset.laplacianEdgeDetect },
    { "emboss", kernelPreset.emboss },
    { "lineEmboss", kernelPreset.lineEmboss },
    { "bigKernelEmboss", kernelPreset.bigKernelEmboss },
    { "bottomSobel", kernelPreset.bottomSobel },
    { "leftSobel", kernelPreset.leftSobel },
    { "rightSobel", kernelPreset.rightSobel },
    { "topSobel", kernelPreset.topSobel },
    { "identity", kernelPreset.identity },
    { "halfIntencity", kernelPreset.halfIntencity },
    { "zeroesKernel", kernelPreset.zeroesKernel }
};
