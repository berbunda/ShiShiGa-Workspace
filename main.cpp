#include <QApplication>
#include <QMainWindow>
#include <QWebEngineView>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;

    auto *view = new QWebEngineView(&window);
    view->load(QUrl("https://chatgpt.com"));

    window.setCentralWidget(view);
    window.resize(1400, 900);
    window.show();

    return app.exec();
}
