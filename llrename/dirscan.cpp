//-------------------------------------------------------------------------------------------------
// File: Dirscan.cpp
// Author: Dennis Lang
//
// Desc: Class to scan directories for valid files

//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// https://landenlabs.com
//
//  
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
#include "signals.hpp"
#include "dirscan.hpp"
#include "directory.hpp"

#include <iostream>

// ---------------------------------------------------------------------------
// Return true if inName matches pattern in patternList
static bool FileMatches(const lstring& inName, const PatternList& patternList, bool emptyResult) {
    if (patternList.empty() || inName.empty())
        return emptyResult;

    for (size_t idx = 0; idx != patternList.size(); idx++)
        if (std::regex_match(inName.begin(), inName.end(), patternList[idx]))
            return true;

    return false;
}

// ---------------------------------------------------------------------------
// Locate matching files which are not in exclude list.
size_t Dirscan::FindFile(const lstring& fullname) {
    size_t fileCount = 0;
    lstring name;
    DirUtil::getName(name, fullname);

    if (! name.empty()
        && ! FileMatches(name, excludeFilePatList, false)
        && FileMatches(name, includeFilePatList, true)) {
        if (parseFile(fullname, name))  {
            fileCount++;
        }
    }

    return fileCount;
}

// ---------------------------------------------------------------------------
// Recurse over directories, locate files.
size_t Dirscan::FindFiles(const lstring& dirname, unsigned depth) {
    Directory_files directory(dirname);
    lstring fullname;
    size_t fileCount = 0;

    
    struct stat filestat;
    try {
        if (stat(dirname, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
            fileCount += FindFile(dirname);
            return fileCount;
        }
    }  catch (exception ex)  {
        // Probably a pattern, let directory scan do its magic.
        cerr << ex.what() << std::endl;
    }

    if (!directory.more()) {
        std::cerr << "Unable to scan directory:" << dirname << std::endl;
        return 0;
    }
    
    while (!Signals::aborted && directory.more()) {
        directory.fullName(fullname);
        if (directory.is_directory()) {
            if ((maxDepth == 0 || depth < maxDepth)
                    && ! FileMatches(fullname, excludeDirPatList, false)
                    && FileMatches(fullname, includeDirPatList, true)) {
                parseDir(fullname);
                if (recurse) {
                    fileCount += FindFiles(fullname, depth + 1);
                }
            }
        } else if (fullname.length() > 0) {
            fileCount += FindFile(fullname);
        }
    }

    return fileCount;
}

