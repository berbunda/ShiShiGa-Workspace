#include "WinEnginePaths.h"

#include <QString>

#ifdef _WIN32
#include <string>
#include <windows.h>
#endif

namespace WinEnginePaths {

#ifdef _WIN32
namespace {

// The portable Qt runtime is deployed next to the executable in a real
// "<executable>.exe.local" directory. Windows DLL redirection resolves the
// application's import libraries from there without junctions, symlinks,
// PATH or environment changes, so the build keeps working after the archive
// is extracted into an arbitrary directory.
std::wstring runtimeDirectory()
{
    wchar_t exePath[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return {};

    std::wstring path(exePath, length);
    path += L".local";
    return path;
}

} // namespace
#endif

bool setupDllSearchPath()
{
#ifdef _WIN32
    const std::wstring runtimeDir = runtimeDirectory();
    if (runtimeDir.empty())
        return false;

    if (SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS) == FALSE)
        return false;

    return AddDllDirectory(runtimeDir.c_str()) != nullptr;
#else
    return true;
#endif
}

QString pluginsDirectory()
{
#ifdef _WIN32
    const std::wstring runtimeDir = runtimeDirectory();
    if (runtimeDir.empty())
        return {};

    const std::wstring pluginsDir = runtimeDir + L"\\plugins";
    return QString::fromWCharArray(pluginsDir.data(), static_cast<int>(pluginsDir.size()));
#else
    return {};
#endif
}

} // namespace WinEnginePaths
