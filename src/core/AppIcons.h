#pragma once

#include <QIcon>
#include <QString>

namespace AppIcons {

QString windowIconResourcePath();
QString trayIcon48ResourcePath();
QString trayIcon16ResourcePath();

QIcon windowIcon();
QIcon trayIcon();

} // namespace AppIcons
