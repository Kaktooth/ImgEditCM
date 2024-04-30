// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <filesystem>
#include <shobjidl.h>
#include <string.h>
#include <windows.h>

using namespace std;

static filesystem::path toFilesystemPath(PWSTR filePath)
{
    size_t convertedChars = 0;
    size_t pathLength = wcslen(filePath) + 1;
    char* charPath = new char[pathLength];
    wcstombs_s(&convertedChars, charPath, pathLength, filePath, _TRUNCATE);
    return charPath;
}

static filesystem::path getOpenedFilePath()
{
    PWSTR filePath = nullptr;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }

    if (filePath != nullptr)
        return toFilesystemPath(filePath);

    return filesystem::path();
}

static filesystem::path saveFilePath()
{
    PWSTR filePath = nullptr;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileSaveDialog* pFileSave;

        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr)) {
            hr = pFileSave->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                    pItem->Release();
                }
            }
            pFileSave->Release();
        }
        CoUninitialize();
    }

    if (filePath != nullptr)
        return toFilesystemPath(filePath);

    return filesystem::path();
}
