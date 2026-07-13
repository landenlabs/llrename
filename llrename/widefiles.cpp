// Copyright (c) 2026 Dennis Lang
//
//-------------------------------------------------------------------------------------------------
// File: widefiles.cpp  Author: Dennis Lang  Desc: Windows wide-char (UTF-16) directory walk
//-------------------------------------------------------------------------------------------------

#include "ll_stdhdr.hpp"
#include "widefiles.hpp"

#ifdef HAVE_WIN

#include <tchar.h>
#include <iostream>
#include <windows.h>

/**
 * @brief Recursively traverses a directory and calls a function for each file.
 * * @param path The starting directory path. Must be a wide string (L"C:\\path").
 * @param callback A function object or function pointer to be called for each file.
 * It must accept a single const std::wstring& argument.
 */
void DirUtil::getWideFiles(const std::wstring& path, std::function<void(const std::wstring&)> callback) {
    // Construct the search pattern, which is the path followed by "\\*"
    // to search for all files and subdirectories.
    std::wstring searchPath = path + L"\\*";
    WIN32_FIND_DATAW findFileData;

    // Find the first file or directory in the specified path.
    // We use FindFirstFileW for Unicode support.
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    // Check if the handle is valid. If not, it's likely an invalid path.
    if (hFind == INVALID_HANDLE_VALUE) {
        // You could add more robust error handling here.
        std::wcerr << L"Error: Could not open directory " << path << std::endl;
        return;
    }

    // Loop through all files and directories in the path.
    do {
        // Get the name of the current file or directory.
        const std::wstring fileName = findFileData.cFileName;

        // Skip the current directory (.) and the parent directory (..).
        // This is crucial to prevent infinite recursion.
        if (fileName == L"." || fileName == L"..") {
            continue;
        }

        // Check if the found item is a directory.
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // It's a directory, so we call the function recursively with the new path.
            std::wstring newPath = path + L"\\" + fileName;
            getWideFiles(newPath, callback);
        } else {
            // It's a file, so we construct the full file path and call the callback function.
            std::wstring fullPath = path + L"\\" + fileName;
            callback(fullPath);
        }
    } while (FindNextFileW(hFind, &findFileData) != 0); // Continue until FindNextFileW fails.

    // Close the search handle to free up resources.
    FindClose(hFind);
}
#endif
