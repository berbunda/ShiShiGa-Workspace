#include "core/SettingsManager.h"
#include "logging/CrashLogger.h"
#include "platform/WinEnginePaths.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QString>

int main(int argc, char *argv[])
{
    if (!WinEnginePaths::setupDllSearchPath())
        return 1;

    // Register the deployed plugin directory before QApplication is created,
    // so Qt can locate the "windows" platform plugin regardless of how the
    // runtime prefix is auto-detected. The path is derived from the running
    // executable, so it stays portable after the archive is extracted.
    const QString pluginsDirectory = WinEnginePaths::pluginsDirectory();
    if (!pluginsDirectory.isEmpty())
        QCoreApplication::addLibraryPath(pluginsDirectory);

    CrashLogger::instance().install(
        QStringLiteral(SHISHIGA_APP_VERSION));

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("ShiShiga Workspace"));

    SettingsManager &settings = SettingsManager::instance();
    settings.load();

    MainWindow window(settings);
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&settings]() {
        settings.save();
    });

    try {
        return app.exec();
    } catch (const std::exception &exception) {
        CrashLogger::instance().logException(exception);
        return 1;
    } catch (...) {
        CrashLogger::instance().logUnknownException();
        return 1;
    }
}
