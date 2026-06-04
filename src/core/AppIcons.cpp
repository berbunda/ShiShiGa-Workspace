#include "AppIcons.h"

#include <QFile>
#include <QPixmap>

namespace AppIcons {

namespace {

constexpr char kWindowIconResource[] = ":/icons/ShiShiGa-128.png";
constexpr char kTrayIcon48Resource[] = ":/icons/ShiShiGa-tray-48.png";
constexpr char kTrayIcon16Resource[] = ":/icons/ShiShiGa-tray-16.png";

QPixmap loadPixmap(const QString &resourcePath)
{
    if (!QFile::exists(resourcePath))
        return QPixmap();

    QPixmap pixmap(resourcePath);
    return pixmap;
}

} // namespace

QString windowIconResourcePath()
{
    return QString::fromLatin1(kWindowIconResource);
}

QString trayIcon48ResourcePath()
{
    return QString::fromLatin1(kTrayIcon48Resource);
}

QString trayIcon16ResourcePath()
{
    return QString::fromLatin1(kTrayIcon16Resource);
}

QIcon windowIcon()
{
    const QPixmap pixmap = loadPixmap(windowIconResourcePath());
    if (!pixmap.isNull())
        return QIcon(pixmap);

    return QIcon(windowIconResourcePath());
}

QIcon trayIcon()
{
    const QPixmap pixmap48 = loadPixmap(trayIcon48ResourcePath());
    const QPixmap pixmap16 = loadPixmap(trayIcon16ResourcePath());

    if (pixmap48.isNull() && pixmap16.isNull())
        return QIcon();

    QIcon icon;
    if (!pixmap48.isNull())
        icon.addPixmap(pixmap48);
    if (!pixmap16.isNull())
        icon.addPixmap(pixmap16);

    return icon;
}

} // namespace AppIcons
