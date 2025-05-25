//-------------------------------------------------------------------------------------------------
//
//  llrename      Nov-2024       Dennis Lang
//
//  Rename files
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// https://landenlabs.com/
//
// ----- License ----
//
// Copyright (c) 2024 Dennis Lang
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

// 4291 - No matching operator delete found
#pragma warning( disable : 4291 )
#define _CRT_SECURE_NO_WARNINGS

// Project files
#include "ll_stdhdr.hpp"
#include "signals.hpp"
#include "dirscan.hpp"
#include "directory.hpp"
#include "parseutil.hpp"

#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <exception>
#include <assert.h>

using namespace std;

#ifdef HAVE_WIN

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define chdir _chdir
#define getcwd _getcwd
#define stricmp _stricmp
#else
const size_t MAX_PATH = __DARWIN_MAXPATHLEN;
#endif

#define VERSION  "v2.8"

// Helper types
typedef unsigned int uint;
typedef std::vector<lstring> StringList;

// Runtime options
static char CWD_BUF[MAX_PATH];
static unsigned CWD_LEN = 0;
const unsigned START_NUM = 1;
static unsigned num = START_NUM;

static bool showFile = false;
static bool verbose = false;
static bool dryRun = false;
static bool fullPath = false;
static bool invert = false;
static bool smartQuote = false; // only quote if spaces
static bool force = false;      // delete target if same name

static char casefold = '-';
static lstring parts;
static bool doDirectories = false;

static lstring logPrefix = "";
static lstring logSep = ", ";
static lstring logEndl = "\n";

static fstream inListStream;
static lstring inListPath;
static fstream outListStream;
static lstring outListPath;

class Substitute { 
public:
    regex subregFrom;
    lstring subregTo;
};
typedef std::vector<Substitute> SubstituteList;
static SubstituteList substituteList;
 

// ---------------------------------------------------------------------------
static char QUOTE_BUF[MAX_PATH];
static const char* quote(const lstring& path, unsigned rootDirLen) {
    if (smartQuote && path.find(' ', rootDirLen) == string::npos) {
        return path + rootDirLen;
    }

    QUOTE_BUF[0] = '"';
    strcpy(QUOTE_BUF + 1, path + rootDirLen);
    strcpy(QUOTE_BUF + 1 + path.length() - rootDirLen, "\"");
    return QUOTE_BUF;
}

// ---------------------------------------------------------------------------
lstring lastChdir;
static void doChdir(const char* dir) {
    if (lastChdir == dir) {
        return;
    }

    if (chdir(dir) == 0) {
        lastChdir = dir;
        if (verbose)
            std::cerr << "chdir to " << dir << std::endl;
    } else {
        Colors::showError(strerror(errno), " Failed to change directory to ", dir);
    }
}

// ---------------------------------------------------------------------------
static int doRenameC(const char* oldName, const char* newName) {
    if (stricmp(oldName, newName) == 0 || !DirUtil::fileExists(newName)) {
        return (dryRun) ? 0 : rename(oldName, newName);
    } else {
         Colors::showError("New file already exists:", newName);
         return 0;
    }
}

// ---------------------------------------------------------------------------
static bool doRenameB(const char* oldName, const char* newName) {
    int code = 0;
    const char* errMsg = "";
    const char* action = " ";
 
    lstring dir1, dir2;
    DirUtil::getDir(dir1, oldName);
    DirUtil::getDir(dir2, newName);
    size_t dirLen = 0;
    
    if (dir1 == dir2) {
        if (force && DirUtil::fileExists(newName)) {
            DirUtil::deleteFile(dryRun, newName);
        }
#ifdef HAVE_WIN
        // TODO - test if windows can do absolute file rename.
        code = doRenameC(oldName, newName);  // rename absolute path
#else
        doChdir(dir1);
        dirLen = dir1.empty() ? 0 : dir1.length() +1; // +1 skip trailing slash
        code = doRenameC(oldName + dirLen, newName + dirLen);  // rename relative path, see doChdir()
#endif
        errMsg = (code == 0) ? "" : strerror(errno);
        action = " rename ";
    } else {
        Colors::showError("Something went wrong, base directory changed ", dir1);
    }
    

    if (verbose || code != 0) {
        Colors::showError(errMsg, action, oldName, " to ", newName);
    }

    return (code == 0);
}

// ---------------------------------------------------------------------------
static bool doRenameA(const char* oldName, const char* newName) {
    return invert ? doRenameB(newName, oldName) : doRenameB(oldName, newName);
}

// ---------------------------------------------------------------------------
static void removeQuote(lstring& word) {
    if (word.length() > 1) {
        word = word.substr(1, word.length() - 2);
    }
}

// ---------------------------------------------------------------------------
static void renameFromStream(istream& inStream) {
    string line;
    while (std::getline(inStream, line)) {
        size_t divider = line.find("\",\"");
        if (divider == string::npos) {
            divider = line.find(',');
        } else {
            divider++;  // skip over trailing quote
        }

        lstring file1 = line.substr(0, divider);
        lstring file2 = line.substr(divider + 1);
        removeQuote(file1);
        removeQuote(file2);

        // Skip identical names.
        if ((file1 != file2) && doRenameA(file1, file2))
            num++;
    }
}

// ---------------------------------------------------------------------------
// Handle "part" renaming. 
static const lstring& getPartRename(lstring& outPath, const lstring& dir, lstring& name, unsigned num) {
    outPath = dir;
    if (parts.empty()) {
        outPath += name;
    } else {
        lstring extn;
        DirUtil::getExt(extn, name);
        name.resize(name.length() - extn.length() - 1);
        lstring part;
        outPath += ParseUtil::getParts(part, parts, name, extn, num);
    }
    return outPath;
}
    
// ---------------------------------------------------------------------------
// Open, read and parse file.
static bool doRename(const lstring& filepath, const lstring& filename) {
    lstring dirWithSlash, newFile;
    
    lstring tmpFile = filename;
    if (casefold == 'c')
        tmpFile = tmpFile.toLower();
    else if (casefold == 'C')
        tmpFile = tmpFile.toUpper();
    
    for(auto item : substituteList) {
        tmpFile = regex_replace(tmpFile, item.subregFrom, item.subregTo);
    }
    
    DirUtil::getDir(dirWithSlash, filepath);
    if (!dirWithSlash.empty()) dirWithSlash += Directory_files::SLASH_CHAR;
    getPartRename(newFile, dirWithSlash, tmpFile, num);

    if (outListPath.size() > 0 && outListStream.good()) {
        unsigned strOffset = fullPath ? 0 : CWD_LEN;
        lstring qOldFile = quote((dirWithSlash + filename) + strOffset, 0);
        lstring qNewFile = quote(newFile + strOffset, 0);
        if (invert)
            outListStream << logPrefix << qNewFile << logSep << qOldFile << logEndl;
        else
            outListStream << logPrefix << qOldFile << logSep << qNewFile << logEndl;
    }

    if (showFile)
        std::cout << filepath << std::endl;
    
    if (verbose && !dryRun) {
        std::cout << "Rename from=" << filepath << " to=" << newFile << std::endl;
    }
    bool okay = (filepath != newFile) && doRenameA(filepath, newFile);
    if (okay)
       num++;
    
    return okay;
}

// ---------------------------------------------------------------------------
// Open, read and parse file.
static bool HandleFile(const lstring& filepath, const lstring& filename) {
    if (!doDirectories) {
        return doRename(filepath, filename);
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
static bool HandleDir(const lstring& filepath) {
    if (doDirectories) {
        lstring dir, name;
        DirUtil::getDir(dir, filepath);
        DirUtil::getName(name, filepath);
        return doRename(filepath, name);
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
void showHelp(const char* arg0) {
    const char* helpMsg =
        "  Dennis Lang "  VERSION  " (LandenLabs.com)_X_ " __DATE__ "\n\n"
        "\nDes: Rename file names\n"
        "Use: llrename [options] directories...   or  files\n"
        "\n"
        " _p_Options (only first unique characters required, options can be repeated):\n"
        "   -_y_includeItem=<filePattern>   ; Include files or dirs by regex match \n"
        "   -_y_excludeItem=<filePattern>   ; Exclude files or dirs by regex match \n"
        "   -_y_IncludePath=<pathPattern>   ; Include path by regex match \n"
        "   -_y_ExcludePath=<pathPattern>   ; Exclude path by regex match \n"
        "   -_y_D                           ; Rename directory \n"
        "   -_y_c/C                         ; lowercase or Uppercase \n"
        "   -_y_sub=<regexp>                ; substitute regexpression \n"
        "                                      /fromRexex/toRegex/ \n"
        "   -_y_parts=<fileParts>           ; See fileParts note below\n"
        "   -_y_startNum=1000               ; Start number, def=1 \n"
        "   -_y_no                          ; No rename, dry run \n"
        "   -_y_force                       ; Deleted target if same name \n"
        "   -_y_recurse                     ; Recurse into directories \n"
        "\n"
        "   -_y_toList=<write_fileName>     ; Output List of 'old','new' \n"
        "   -_y_fromList=<read_fileName>    ; Read List rename pair per line \n"
        " _P_Used with -fromList _X_ \n"
        "   -_y_1       [default]           ; Rename 'old' to 'new' \n"
        "   -_y_2                           ; Rename 'new' to 'old' \n"
        "   -_y_full                        ; Log full path, default is relative \n"
        "   -_y_smartQuote                  ; Only add quotes if spaces in path \n"
        "   -_y_logStart=\"foo.csh \"       ; Log prefix string, def none \n"
        "   -_y_logSep=\", \"               ; Log separator string, def=\", \" \n"
        "   -_y_logEnd=\"\\n\"              ; Log end of line, def=\"\\n\" \n"
        "\n"
        " _p_Options for -_y_parts  (default=N.E)\n"
        "     N=name, E=extension, #=number (note uppercase N and E)\n"
        "     N-#.E \n"
        "     N_####.E \n"
        "     N.'foo' \n"
    
        "\n"
        " _p_Debug:\n"
        "   -_y_showfiles                   ; Display files found \n"
        "   -_y_verbose                     ; Only dump parsed json\n"
        "\n"
        " _p_Example: \n"
#ifdef HAVE_WIN
        "  llrename -_y_c -_y_r -_y_sub=\"/ /_/\" -_y_inc=*.png -_y_exc=foo.png dir1 dir2 \n"
        "  llrename -_y_C -_y_r -_y_start=1000 -_y_part=\"N_####.E\" -_y_inc=*.jpg dir1 Foo*.png Bar*.jpg  \n"
#else
        "  llrename -_y_c -_y_r -_y_sub=\"/ /_/\" -_y_inc=\\*.png -_y_exc=foo.png dir1 dir2 \n"
        "  llrename -_y_C -_y_r -_y_start=1000 -_y_part=\"N_####.E\" -_y_inc=\\*.jpg -_y_inc=\\*.png dir1 Foo*.png Bar*.jpg \n"
#endif
        "  llrename -_y_sub=\"/([^_]+)_([.*])/$2-$1/\" dir1 \n"
        " _p_Warning with regular expression: \n"
        "   Remember to escape special characters, such as ., ( and [ \n"
        "   For example to remove all  \"copy (2) of\" use \n"
        "   -sub=\"/copy [(][0-9][)] of//\" \n"
        "\n"
        "   The include/exclude regular expression internally converts \n"
        "      * to .*   and  ?  to . \n"
        "     Ex:  *.png  is internally .*.png \n"
        "\n";
    
    std::cerr << Colors::colorize("\n_W_") << arg0 << Colors::colorize(helpMsg);
}

// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    Signals::init();
    ParseUtil parser;
    Dirscan dirscan(HandleDir, HandleFile);
    StringList extraDirList;
    
    if (argc == 1) {
        showHelp(argv[0]);
        return 0;
    } else {
        getcwd(CWD_BUF, sizeof(CWD_BUF));
        CWD_LEN = (unsigned)strlen(CWD_BUF) + 1;

        bool doParseCmds = true;
        string endCmds = "--";
        for (int argn = 1; argn < argc; argn++) {
            if (*argv[argn] == '-' && doParseCmds) {
                lstring argStr(argv[argn]);
                Split cmdValue(argStr, "=", 2);
                if (cmdValue.size() == 2)  {
                    lstring cmd = cmdValue[0];
                    lstring value = cmdValue[1];
                    
                    if (cmd.length() > 1 && cmd[0] == '-')
                        cmd.erase(0);   // allow -- prefix on commands
                    
                    const char* cmdName = cmd+1;
                    switch (*cmdName) {
                    case 'e':   // -excludeItem=<pat>
                        parser.validPattern(dirscan.excludeFilePatList, value, "excludeItem", cmdName);
                        break;
                    case 'E':   // -ExcludePath=<pat>
                        parser.validPattern(dirscan.excludeDirPatList, value, "ExcludePath", cmdName);
                        break;
                    case 'i':   // -includeItem=<pat>
                        parser.validPattern(dirscan.includeFilePatList, value, "includeItem", cmdName);
                        break;
                    case 'I':   // -IncludePath=<pat>
                        parser.validPattern(dirscan.includeDirPatList, value, "IncludePath", cmdName);
                        break;
                    case 'f':   // -fromList=<filepath>
                        parser.validFile(inListStream, std::ios::in, inListPath=value, "fromList", cmdName);
                        break;
                    case 't':   // -toList=<filepath>
                        parser.validFile(outListStream, std::ios::out, outListPath=value, "tolist", cmdName);
                        break;
                    case 'l':
                        if (parser.validOption("logStart", cmdName, false)) {
                            logPrefix = ParseUtil::convertSpecialChar(value);
                        } else if (parser.validOption("logSep", cmdName, false)) {
                            logSep = ParseUtil::convertSpecialChar(value);
                        } else if (parser.validOption("logEnd", cmdName, false)) {
                            logEndl = ParseUtil::convertSpecialChar(value);
                        }
                        break;
                    case 'p':   // -parts="<format/sector>"
                        if (parser.validOption("parts", cmdName)) {
                            parts = ParseUtil::convertSpecialChar(value);
                        }
                        break;
                    case 's':   // substitute regexp, -sub=/fromPat/toPat/
                        if (parser.validOption("start", cmdName, false)) {
                            char* endStr;
                            num = (unsigned)std::strtol(value, &endStr, 10);
                        } else if (parser.validOption("substitute", cmdName)) {
                            Split parts(value.substr(1), value.substr(0, 1));
                            if (parts.size() == 3) { 
                                Substitute item;
                                item.subregFrom = parser.getRegEx(parts[0]);
                                item.subregTo = parts[1];
                                substituteList.push_back(item);
                            } else {
                                Colors::showError("Substitute needs two parts split with a unique character, ex -sub=/pat1/replaceWith/\n",
                                    "The first character is used to find the splits.\n"
                                    "This substitute does not follow that rule:", value);
                            }
                        }
                        break;
                    default:
                        parser.showUnknown(cmd);
                        break;
                    }
                } else {
                    const char* cmdName = argStr + 1;
                    switch (*cmdName) {
                    case 'v':   // verbose
                        verbose = true;
                        break;
                    case 'n':   // dryRun
                        if (parser.validOption("noaction", cmdName)) {
                            std::cerr << "DryRun\n";
                            verbose = dryRun = true;
                        }
                        break;
                            
                    case 'c':   // -c = lowercase
                    case 'C':   // -C = uppercase
                        casefold = *cmdName;
                        break;
                    case 'D':   // -Directory = rename only directories
                        doDirectories = true;
                        std::cerr << "Renaming directories\n";
                        break;
                            
                    case 'f':   // full path
                        if (parser.validOption("fullpath", cmdName, false)) {
                            fullPath = true;
                        } else if (parser.validOption("force", cmdName)) {
                            force = true;
                        }
                        break;
                            
                    case 'h':
                        if (parser.validOption("help", cmdName)) {
                            showHelp(argv[0]);
                            return 0;
                        }
                        break;
                    case 'r':   // -recurse
                        dirscan.recurse = true;
                        break;
                                
                    case 's': // SmartQuotes
                        if (parser.validOption("showFiles", cmdName, false)) {
                            showFile = true;
                        } else if (parser.validOption("smartQuotes", cmdName)) {
                            smartQuote = true;
                        }
                        break;
                    case '1':   // old to new
                        invert = false;
                        break;
                    case '2':   // new to old
                        invert = true;
                        break;
     
                    case '?':
                        showHelp(argv[0]);
                        return 0;
                    default:
                        parser.showUnknown(argStr);
                    }

                    if (endCmds == argv[argn]) {
                        doParseCmds = false;
                    }
                }
            } else {
                // Collect extra arguments for scanning below.
                extraDirList.push_back(argv[argn]);
            }
        }

        if (verbose) {
            std::cout << "--- Settings ---\n";
            if (showFile) std::cout << "ShowFile\n";
            if (dryRun) std::cout << "Dry run\n";
            if (fullPath) std::cout << "show full path\n";
            if (invert) std::cout << "Invert list?\n";
            if (smartQuote) std::cout << "Smart Quotes\n";
            if (force) std::cout << "Force delete\n";
            if (doDirectories) std::cout << "Do directories\n";
            if (dirscan.recurse) std:cout << "Recurse\n";
            if (casefold != '-') std::cout << "CaseFold=" << casefold << std::endl;
            
            std::cout << "Parts=" <<  parts << std::endl;
            for (auto item : substituteList) {
                // static regex subregFrom;
                std::cout << "SubTo=" << item.subregTo << std::endl;
            }
            
            std::cout << "LogPrefix=" << logPrefix << std::endl;
            std::cout << "logSep=" << logSep << std::endl;
            std::cout << "logEndl=" << logEndl << std::endl;
            std::cout << "--- End Settings ---\n";
        }
        
        if (parser.patternErrCnt == 0 && parser.optionErrCnt == 0) {
            for (auto const& filePath : extraDirList)  {
                dirscan.FindFiles(filePath, 0);
            }

            if (inListStream)  {
                renameFromStream(inListStream);
            }
        }

        Colors::showError(doDirectories ? " Directories=" : " Files=", ( num - START_NUM ), " renamed");
    }

    return 0;
}
