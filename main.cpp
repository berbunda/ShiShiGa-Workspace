#include "core/SettingsManager.h"
#include "logging/CrashLogger.h"
#include "platform/WinEnginePaths.h"
#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    if (!WinEnginePaths::setupDllSearchPath())
        return 1;

    CrashLogger::instance().install(
        QStringLiteral(SHISHIGA_APP_VERSION));

    QApplication app(argc, argv);

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
