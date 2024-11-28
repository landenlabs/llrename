//-------------------------------------------------------------------------------------------------
//
// File: directory.cpp   Author: Dennis Lang  Desc: Get files from directories
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// http://landenlabs.com
//
// This file is part of llrename project.
//
// ----- License ----
//
// Copyright (c) 2024  Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "ll_stdhdr.hpp"
#include "directory.hpp"

#include <iostream>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_WIN
    #include <windows.h>
#endif

const char EXTN_CHAR = '.';

#if defined(_WIN32) || defined(_WIN64)

// #include <sys/stat.h>
// #include <sys/types.h>
#include <io.h>
// #include <stdio.h>
 
typedef unsigned short mode_t;

static const mode_t S_IRUSR = mode_t(_S_IREAD);     //  read by user
static const mode_t S_IWUSR = mode_t(_S_IWRITE);    //  write by user
#define chmod _chmod

const lstring ANY("\\*");
const lstring Directory_files::SLASH = "\\";
const char Directory_files::SLASH_CHAR = '\\';
const lstring Directory_files::SLASH2 = "\\\\";

//-------------------------------------------------------------------------------------------------
// Return true if attribute is a Directory
inline static bool isDir(DWORD attr) {
    return (attr != -1) && ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//-------------------------------------------------------------------------------------------------
// Return 'clean' full path, remove extra slahes.
static lstring& GetFullPath(lstring& fname) {
    char fullPath[MAX_PATH];
    DWORD len1 = GetFullPathName(fname, ARRAYSIZE(fullPath), fullPath, NULL);
    fname = fullPath;
    return fname;
}

//-------------------------------------------------------------------------------------------------
Directory_files::Directory_files(const lstring& dirName) :
    my_dir_hnd(INVALID_HANDLE_VALUE),
    my_dirName(dirName) {
}

//-------------------------------------------------------------------------------------------------
Directory_files::~Directory_files() {
    if (my_dir_hnd != INVALID_HANDLE_VALUE)
        FindClose(my_dir_hnd);
}

//-------------------------------------------------------------------------------------------------
void Directory_files::close() {
    if (my_dir_hnd != INVALID_HANDLE_VALUE) {
        FindClose(my_dir_hnd);
        my_dir_hnd = INVALID_HANDLE_VALUE;
    }
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::begin() {
    close();

    lstring dir = my_dirName;
    if (dir.empty())
        dir = ".";    // Default to current directory

    DWORD attr = GetFileAttributes(dir);
    int err = GetLastError();       // Error 3 = invalid path.

    if (isDir(attr)) {
        dir += ANY;
    } else { // if (attr != INVALID_FILE_ATTRIBUTES)
        GetFullPath(my_dirName);
        // Peel off one subdir from reference name.
        size_t pos = my_dirName.find_last_of(":/\\");
        if (pos != std::string::npos)
            my_dirName.resize(pos);
    }

    my_dir_hnd = FindFirstFile(dir, &my_dirent);
    bool is_more = (my_dir_hnd != INVALID_HANDLE_VALUE);

    while (is_more
        && (isDir(my_dirent.dwFileAttributes)
    && strspn(my_dirent.cFileName, ".") == strlen(my_dirent.cFileName) )) {
        is_more = (FindNextFile(my_dir_hnd, &my_dirent) != 0);
    }

    return is_more;
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::more() {
    if (my_dir_hnd == INVALID_HANDLE_VALUE)
        return begin();

    bool is_more = false;
    if (my_dir_hnd != INVALID_HANDLE_VALUE) {
        // Determine if there any more files
        //   skip any dot-directories.
        do {
            is_more = (FindNextFile(my_dir_hnd, &my_dirent) != 0);
        } while (is_more
            && (isDir(my_dirent.dwFileAttributes)
        && strspn(my_dirent.cFileName, ".") == strlen(my_dirent.cFileName)));

    }

    return is_more;
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::is_directory() const {
    return (my_dir_hnd != INVALID_HANDLE_VALUE && isDir(my_dirent.dwFileAttributes));
}

//-------------------------------------------------------------------------------------------------
const char* Directory_files::name() const {
    return (my_dir_hnd != INVALID_HANDLE_VALUE) ?
        my_dirent.cFileName : NULL;
}

//-------------------------------------------------------------------------------------------------
const lstring& Directory_files::fullName(lstring& fname) const {
    fname = my_dirName + SLASH + name();
    return GetFullPath(fname);
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::exists( const char* path) {
    const DWORD attr = GetFileAttributes(path);
    return (attr != INVALID_FILE_ATTRIBUTES);
}


#else

#include <unistd.h>
#include <stdlib.h>

const lstring Directory_files::SLASH = "/";
const char Directory_files::SLASH_CHAR = '/';
const lstring Directory_files::SLASH2 = "//";

//-------------------------------------------------------------------------------------------------
Directory_files::Directory_files(const lstring& dirName) {
    realpath(dirName.c_str(), my_fullname);
    my_baseDir = my_fullname;
    my_pDir = opendir(my_baseDir);
    my_is_more = (my_pDir != NULL);
}

//-------------------------------------------------------------------------------------------------
Directory_files::~Directory_files() {
    if (my_pDir != NULL)
        closedir(my_pDir);
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::more() {
    if (my_is_more) {
        my_pDirEnt = readdir(my_pDir);
        my_is_more = my_pDirEnt != NULL;
        if (my_is_more) {
            if (my_pDirEnt->d_type == DT_DIR) {
                while (my_is_more &&
                (my_pDirEnt->d_name[0] == '.' && ! isalnum(my_pDirEnt->d_name[1]))) {
                    more();
                }
            }
        }
    }

    return my_is_more;
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::is_directory() const {
    return my_pDirEnt->d_type == DT_DIR;
}

//-------------------------------------------------------------------------------------------------
const char* Directory_files::name() const {
    return my_pDirEnt->d_name;
}

//-------------------------------------------------------------------------------------------------
const lstring& Directory_files::fullName(lstring& fname) const {
    return join(fname, my_baseDir, my_pDirEnt->d_name);
}

//-------------------------------------------------------------------------------------------------
bool Directory_files::exists(const char* path) {
    return access(path, F_OK) == 0;

}
#endif

//-------------------------------------------------------------------------------------------------
// Extract directory part from path.
lstring& Directory_files::getDir(lstring& outDir, const lstring& inPath) {
    size_t nameStart = inPath.rfind(SLASH_CHAR);
    if (nameStart == string::npos)
        outDir.clear();
    else
        outDir = inPath.substr(0, nameStart);
    return outDir;
}

//-------------------------------------------------------------------------------------------------
// Extract name part from path.
lstring& Directory_files::getName(lstring& outName, const lstring& inPath) {
    size_t nameStart = inPath.rfind(SLASH_CHAR);
    if (nameStart == std::string::npos)
        outName = inPath;
    else
        outName = inPath.substr(nameStart + 1);
    return outName;
}

//-------------------------------------------------------------------------------------------------
// Extract name part from path.
lstring& Directory_files::getExt(lstring& outExt, const lstring& inPath) {
    size_t extPos = inPath.rfind(EXTN_CHAR);
    if (extPos == std::string::npos)
        outExt = "";
    else
        outExt = inPath.substr(extPos + 1);
    return outExt;
}

//-------------------------------------------------------------------------------------------------
// Extract name part from path.
bool Directory_files::deleteFile(const char* inPath) {
    int err = remove(inPath);
    if (err != 0) {
        if (errno == EPERM || errno == EACCES)
            setPermission(inPath, S_IWUSR);
        err = remove(inPath);
    }
    
    if (err != 0)
        std::cerr << strerror(errno) << " deleting " << inPath << std::endl;
    else
        std::cerr << "Deleted " << inPath << std::endl;
    
    
    return (err == 0);
}

//-------------------------------------------------------------------------------------------------
// Set permission on relative path file and directories.
bool Directory_files::setPermission(const char* relPath, unsigned permission, bool setAllParts) {
    if (relPath == nullptr || strlen(relPath) <= 1)
        return true;
    
    lstring dir;
    struct stat pathStat;
    int err = stat(relPath, &pathStat);
    if (err == 0) {
        if ((pathStat.st_mode & permission) != permission) {
            err = chmod(relPath, pathStat.st_mode | permission);
            // Windows
            // SetFileAttributes(filename, attributes | FILE_ATTRIBUTE_READONLY)
        }
        if (setAllParts)
            return setPermission(getDir(dir, relPath), permission, true);
    }
    return (err == 0);
}