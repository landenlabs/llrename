//-------------------------------------------------------------------------------------------------
// File: Dirscan.hpp
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

#pragma once

#include "ll_stdhdr.hpp"

#include <regex>

#ifdef HAVE_WIN
#endif

typedef std::vector<std::regex> PatternList;
typedef bool (*ParseDir_t)(const lstring& filepath, bool onEntry);
typedef bool (*ParseFile_t)(const lstring& filepath, const lstring& filename);

class Dirscan {
    ParseDir_t parseDir;
    ParseFile_t parseFile;
    
public:
    bool recurse = false;
    
public:
    Dirscan(ParseDir_t _parseDir, ParseFile_t _parseFile) : parseDir(_parseDir), parseFile(_parseFile) {
    }
    // ~Dirscan();
    
    PatternList includeFilePatList;
    PatternList excludeFilePatList;
    PatternList includeDirPatList;
    PatternList excludeDirPatList;
    unsigned maxDepth = 0;  // 0 no limit
    
    size_t FindFile(const lstring& dirname);
    size_t FindFiles(const lstring& dirname, unsigned depth);
};

