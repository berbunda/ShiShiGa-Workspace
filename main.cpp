#include "platform/WinEnginePaths.h"
#include "profile/ProfileManager.h"
#include "services/AiService.h"

#include <QApplication>
#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineView>

int main(int argc, char *argv[])
{
    if (!WinEnginePaths::setupDllSearchPath())
        return 1;

    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("ShiShiga Workspace"));

    QWebEngineProfile *profile = ProfileManager::instance().profileFor(AiService::ChatGpt);
    if (profile == nullptr)
        return 1;

    auto *page = new QWebEnginePage(profile, &window);
    auto *view = new QWebEngineView(&window);
    view->setPage(page);
    view->load(AiService::urlFor(AiService::ChatGpt));

    window.setCentralWidget(view);
    window.resize(1400, 900);
    window.show();

    return app.exec();
}
