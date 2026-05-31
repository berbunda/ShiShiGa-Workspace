#pragma once

#include <QString>

class LicenseText
{
public:
    static QString loadGplV3Text();
    static QString licenseFilePath();

private:
    static QString loadFromFile(const QString &path);
    static QString loadFromEmbeddedResource();
    static QString loadFailureMessage();
};
