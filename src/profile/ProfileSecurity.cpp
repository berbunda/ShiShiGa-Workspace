#include "ProfileSecurity.h"

#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#endif

namespace ProfileSecurity {

bool restrictToCurrentUserAndAdministrators(const QString &directoryPath)
{
#ifdef Q_OS_WIN
    const QString nativePath = QDir::toNativeSeparators(directoryPath);
    const std::wstring path = nativePath.toStdWString();

    HANDLE tokenHandle = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
        return false;

    DWORD tokenInfoLength = 0;
    GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &tokenInfoLength);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(tokenHandle);
        return false;
    }

    QByteArray tokenInfo(tokenInfoLength, Qt::Uninitialized);
    auto *tokenUser = reinterpret_cast<TOKEN_USER *>(tokenInfo.data());
    if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenInfoLength, &tokenInfoLength)) {
        CloseHandle(tokenHandle);
        return false;
    }
    CloseHandle(tokenHandle);

    PSID userSid = tokenUser->User.Sid;

    PSID adminSid = nullptr;
    PSID systemSid = nullptr;
    if (!ConvertStringSidToSidW(L"S-1-5-32-544", &adminSid))
        return false;
    if (!ConvertStringSidToSidW(L"S-1-5-18", &systemSid)) {
        LocalFree(adminSid);
        return false;
    }

    EXPLICIT_ACCESSW accessEntries[3] = {};
    for (auto &entry : accessEntries) {
        entry.grfAccessPermissions = FILE_ALL_ACCESS;
        entry.grfAccessMode = SET_ACCESS;
        entry.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

    accessEntries[0].Trustee.pMultipleTrustee = nullptr;
    accessEntries[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    accessEntries[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    accessEntries[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
    accessEntries[0].Trustee.ptstrName = reinterpret_cast<PWCH>(userSid);

    accessEntries[1].Trustee.pMultipleTrustee = nullptr;
    accessEntries[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    accessEntries[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    accessEntries[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    accessEntries[1].Trustee.ptstrName = reinterpret_cast<PWCH>(adminSid);

    accessEntries[2].Trustee.pMultipleTrustee = nullptr;
    accessEntries[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    accessEntries[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    accessEntries[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
    accessEntries[2].Trustee.ptstrName = reinterpret_cast<PWCH>(systemSid);

    PACL acl = nullptr;
    const DWORD aclResult = SetEntriesInAclW(3, accessEntries, nullptr, &acl);
    LocalFree(adminSid);
    LocalFree(systemSid);

    if (aclResult != ERROR_SUCCESS || acl == nullptr)
        return false;

    const DWORD setResult = SetNamedSecurityInfoW(
        const_cast<PWCH>(path.c_str()),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        acl,
        nullptr);

    LocalFree(acl);
    return setResult == ERROR_SUCCESS;
#else
    Q_UNUSED(directoryPath);
    return true;
#endif
}

} // namespace ProfileSecurity
