#include "WinEnginePaths.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace WinEnginePaths {

bool setupDllSearchPath()
{
#ifdef Q_OS_WIN
    wchar_t exePath[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;

    wchar_t *lastSeparator = wcsrchr(exePath, L'\\');
    if (lastSeparator == nullptr)
        return false;
    *(lastSeparator + 1) = L'\0';

    wchar_t enginePath[MAX_PATH] = {};
    if (wcscpy_s(enginePath, exePath) != 0)
        return false;
    if (wcscat_s(enginePath, L"engine") != 0)
        return false;

    if (SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS) == FALSE)
        return false;

    return AddDllDirectory(enginePath) != nullptr;
#else
    return true;
#endif
}

} // namespace WinEnginePaths
