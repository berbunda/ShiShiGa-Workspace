#include "LicenseText.h"

#include "core/AppMetadata.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

QString LicenseText::licenseFilePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(
        QString::fromLatin1(AppMetadata::kLicenseFileName));
}

QString LicenseText::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    return QString::fromUtf8(file.readAll());
}

QString LicenseText::loadFromEmbeddedResource()
{
    QFile resourceFile(QString::fromLatin1(AppMetadata::kLicenseResourcePath));
    if (!resourceFile.open(QIODevice::ReadOnly))
        return {};

    return QString::fromUtf8(resourceFile.readAll());
}

QString LicenseText::loadFailureMessage()
{
    return QStringLiteral(
        "Unable to load GPL v3 license text. Please check LICENSE file.");
}

QString LicenseText::loadGplV3Text()
{
    const QString fromFile = loadFromFile(licenseFilePath());
    if (!fromFile.isEmpty())
        return fromFile;

    const QString fromResource = loadFromEmbeddedResource();
    if (!fromResource.isEmpty())
        return fromResource;

    return loadFailureMessage();
}
