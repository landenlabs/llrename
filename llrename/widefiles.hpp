// Copyright (c) 2026 Dennis Lang
//
//-------------------------------------------------------------------------------------------------
// File: widefiles.hpp  Author: Dennis Lang  Desc: Windows wide-char (UTF-16) directory walk
//
// llrename-specific: not part of llcommon since it's Windows-only and unique to the
// -wide rename mode.
//-------------------------------------------------------------------------------------------------

#pragma once

#ifdef HAVE_WIN
#include <functional>
#include <string>

namespace DirUtil {
    void getWideFiles(const std::wstring& path, std::function<void(const std::wstring&)> callback);
}
#endif
