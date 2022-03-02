#pragma once

// Help functions for work with samples

#include "pch.h"
#include "resource.h"

#include <filesystem>
#include <fstream>
#include <string>

#include <ext/scope/on_exit.h>
#include <ext/std/string.h>

namespace test::samples {

// compare text with resource content
inline void compare_with_resource_file(const std::wstring& text, const UINT resourceId, const wchar_t* resourceName)
{
    // load resource file
    HRSRC hResource = ::FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceId), resourceName);
    ASSERT_TRUE(hResource) << "Failed to load resource file " << resourceName;
    HGLOBAL hGlob = ::LoadResource(GetModuleHandle(NULL), hResource);
    ASSERT_TRUE(hGlob) << "Failed to load resource file " << resourceName;
    ext::scope::FreeObject resourceFree(hGlob, &::FreeResource);

    // lock resource and get data size
    const LPVOID resourceLocker = ::LockResource(hGlob);
    const DWORD dwResourceSize = ::SizeofResource(GetModuleHandle(NULL), hResource);
    ASSERT_TRUE(resourceLocker && dwResourceSize != 0) << "Failed to load resource file " << resourceName;
    ASSERT_STREQ(std::narrow(text.c_str()).c_str(), (const char*)resourceLocker) << "Content unmatched for resource file " << resourceName;
}

// compare file with resoursce
inline void compare_with_resource_file(const std::filesystem::path& pathToFile, const UINT resourceId, const wchar_t* resourceName)
{
    std::ifstream file(pathToFile, std::ifstream::binary | std::ifstream::ate);
    ASSERT_FALSE(file.fail()) << "Failed to open file " << pathToFile;

    // load resource file
    HRSRC hResource = ::FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceId), resourceName);
    ASSERT_TRUE(hResource) << "Failed to load resource file " << resourceName;
    HGLOBAL hGlob = ::LoadResource(GetModuleHandle(NULL), hResource);
    ASSERT_TRUE(hGlob) << "Failed to load resource file " << resourceName;
    ext::scope::FreeObject resourceFree(hGlob, &::FreeResource);

    // lock resource and get data size
    const LPVOID resourceLocker = ::LockResource(hGlob);
    const DWORD dwResourceSize = ::SizeofResource(GetModuleHandle(NULL), hResource);
    ASSERT_TRUE(resourceLocker && dwResourceSize != 0) << "Failed to load resource file " << resourceName;

    ASSERT_EQ(file.tellg(), dwResourceSize) << "File sizes unmatched " << pathToFile << " and resource " << resourceName;

    file.seekg(0, std::ifstream::beg);

    // load file content
    std::string fileText;
    std::copy(std::istreambuf_iterator<char>(file),
              std::istreambuf_iterator<char>(),
              std::insert_iterator<std::string>(fileText, fileText.begin()));

    ASSERT_STREQ(fileText.c_str(), (const char*)resourceLocker) << "File content unmatched " << pathToFile << " and resource " << resourceName;
}

} // namespace test::samples
