#pragma once
#include <windows.h>
#include <cstdint>
#include <string>

namespace EverythingClient {
    // Queries Everything index for folder size in bytes.
    // Returns UINT64_MAX if not found or unindexed.
    uint64_t GetFolderSize(const wchar_t* folderPath);

    // Formats byte size into human readable string (e.g. 1.45 MB / 2.3 GB)
    std::wstring FormatSize(uint64_t bytes, bool useIec = false);
}
