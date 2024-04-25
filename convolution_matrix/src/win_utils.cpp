// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "convolution_matrix_img.h"
#include <filesystem>
#include <iostream>
#include <string.h>
#include <windows.h>

using namespace std;

static OPENFILENAMEW createDialog()
{
    OPENFILENAMEW ofn;
    wchar_t file[8192];
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = file;
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrFilter = L"All\0*.*\0png\0*.PNG\0jpg\0*.JPG\0bmp\0*.BMP\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFile = 8192;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    return ofn;
}

static filesystem::path getOpenedFilePath()
{
    try {
        OPENFILENAMEW openFileName = createDialog();
        if (GetOpenFileNameW(&openFileName) == TRUE) {
            filesystem::path filePath = openFileName.lpstrFile;
            wcout << L"Selected file: " << openFileName.lpstrFile << endl;

            return filePath;
        } else {
            cerr << "Open dialog failed or canceled.\n";
        }
    } catch (exception ex) {
        cout << ex.what();
    }
}

static filesystem::path saveFilePath()
{
    try {
        OPENFILENAMEW saveFileName = createDialog();
        if (GetSaveFileNameW(&saveFileName) == TRUE) {

            filesystem::path filePath = saveFileName.lpstrFile;
            wcout << L"Created file: " << saveFileName.lpstrFile << endl;

            return filePath;
        } else {
            cerr << "Open dialog failed or canceled.\n";
        }
    } catch (exception ex) {
        cout << ex.what();
    }
}
