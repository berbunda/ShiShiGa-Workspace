#pragma once

#include <QString>

class AppMetadata
{
public:
    static constexpr const char *kApplicationDisplayName = "ShiShiga-Workspace";
    static constexpr const char *kGitHubRepositoryUrl =
        "https://github.com/berbunda/ShiShiGa-Workspace";
    static constexpr const char *kGplOfficialUrl =
        "https://www.gnu.org/licenses/gpl-3.0.html";
    static constexpr const char *kLicenseShortName =
        "GNU General Public License v3.0";
    static constexpr const char *kLicenseFileName = "LICENSE";
    static constexpr const char *kLicenseResourcePath = ":/LICENSE";

    static QString applicationVersion();
    static QString gitCommit();
    static QString buildDate();
    static QString platformDescription();
    static QString qtRuntimeVersion();
    static QString compilerLabel();
    static QString cmakeVersionLabel();

    static QString versionInfoClipboardText();
};
