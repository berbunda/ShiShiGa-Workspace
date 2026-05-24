#include "MainWindow.h"

#include "ServiceSidebar.h"
#include "core/ServiceManager.h"
#include "services/AiService.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("ShiShiga Workspace"));
    resize(1400, 900);
    setupUi();
    openDefaultService();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_stack = new QStackedWidget(central);
    m_serviceManager = new ServiceManager(m_stack, this);
    m_sidebar = new ServiceSidebar(m_serviceManager, central);

    layout->addWidget(m_sidebar);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);
}

void MainWindow::openDefaultService()
{
    m_serviceManager->activateService(QString::fromLatin1(AiService::ChatGpt));
}
