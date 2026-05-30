#include "DevConsoleWindow.h"

#include "core/ServiceManager.h"
#include "diagnostics/WebEngineConsoleLog.h"
#include "diagnostics/WebEngineDiagnosticsInfo.h"

#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

DevConsoleWindow::DevConsoleWindow(ServiceManager *serviceManager, QWidget *parent)
    : QDialog(parent)
    , m_serviceManager(serviceManager)
{
    setWindowTitle(tr("Dev Console"));
    setModal(false);
    setMinimumSize(720, 560);
    resize(780, 600);
    setupUi();
    connectSignals();
    refresh();
}

void DevConsoleWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    auto *webEngineGroup = new QGroupBox(tr("WebEngine / Network"), this);
    auto *webEngineLayout = new QVBoxLayout(webEngineGroup);
    m_webEngineOutput = new QPlainTextEdit(webEngineGroup);
    m_webEngineOutput->setReadOnly(true);
    m_webEngineOutput->setLineWrapMode(QPlainTextEdit::NoWrap);
    {
        QFont monoFont = m_webEngineOutput->font();
        monoFont.setFamily(QStringLiteral("Consolas"));
        m_webEngineOutput->setFont(monoFont);
    }
    m_webEngineOutput->setMaximumBlockCount(200);
    webEngineLayout->addWidget(m_webEngineOutput);

    auto *consoleGroup = new QGroupBox(tr("Console Messages"), this);
    auto *consoleLayout = new QVBoxLayout(consoleGroup);
    m_consoleOutput = new QPlainTextEdit(consoleGroup);
    m_consoleOutput->setReadOnly(true);
    m_consoleOutput->setLineWrapMode(QPlainTextEdit::NoWrap);
    {
        QFont monoFont = m_consoleOutput->font();
        monoFont.setFamily(QStringLiteral("Consolas"));
        m_consoleOutput->setFont(monoFont);
    }
    consoleLayout->addWidget(m_consoleOutput);

    auto *actionsLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_clearButton = new QPushButton(tr("Clear Console"), this);
    m_copyButton = new QPushButton(tr("Copy Console"), this);
    m_reloadButton = new QPushButton(tr("Reload Page"), this);
    m_closeButton = new QPushButton(tr("Close"), this);

    actionsLayout->addWidget(m_refreshButton);
    actionsLayout->addWidget(m_clearButton);
    actionsLayout->addWidget(m_copyButton);
    actionsLayout->addWidget(m_reloadButton);
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_closeButton);

    rootLayout->addWidget(webEngineGroup);
    rootLayout->addWidget(consoleGroup, 1);
    rootLayout->addLayout(actionsLayout);

    connect(m_refreshButton, &QPushButton::clicked, this, &DevConsoleWindow::refresh);
    connect(m_clearButton, &QPushButton::clicked, this, &DevConsoleWindow::clearConsole);
    connect(m_copyButton, &QPushButton::clicked, this, &DevConsoleWindow::copyConsole);
    connect(m_reloadButton, &QPushButton::clicked, this, &DevConsoleWindow::reloadPage);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void DevConsoleWindow::connectSignals()
{
    if (m_serviceManager == nullptr)
        return;

    connect(m_serviceManager, &ServiceManager::activeServiceChanged,
            this, &DevConsoleWindow::onActiveServiceChanged);

    if (WebEngineConsoleLog *consoleLog = m_serviceManager->webEngineConsoleLog(); consoleLog != nullptr) {
        connect(consoleLog, &WebEngineConsoleLog::messageLogged,
                this, &DevConsoleWindow::onConsoleMessageLogged);
    }
}

void DevConsoleWindow::refresh()
{
    updateAvailability();
    refreshWebEngineSection();
    refreshConsoleSection();
}

void DevConsoleWindow::updateAvailability()
{
    const bool hasActiveService = m_serviceManager != nullptr
        && !m_serviceManager->activeServiceId().isEmpty();

    m_clearButton->setEnabled(hasActiveService);
    m_copyButton->setEnabled(hasActiveService);
    m_reloadButton->setEnabled(hasActiveService
                             && m_serviceManager->viewFor(m_serviceManager->activeServiceId()) != nullptr);
}

void DevConsoleWindow::refreshWebEngineSection()
{
    const WebEngineDiagnosticsSnapshot snapshot = WebEngineDiagnosticsInfo::collect(m_serviceManager);
    m_webEngineOutput->setPlainText(WebEngineDiagnosticsInfo::formatWebEngineSection(snapshot));
}

void DevConsoleWindow::refreshConsoleSection()
{
    m_consoleOutput->setPlainText(WebEngineDiagnosticsInfo::formatConsoleMessages(m_serviceManager));
}

void DevConsoleWindow::clearConsole()
{
    if (m_serviceManager == nullptr || m_serviceManager->activeServiceId().isEmpty())
        return;

    if (WebEngineConsoleLog *consoleLog = m_serviceManager->webEngineConsoleLog(); consoleLog != nullptr)
        consoleLog->clear(m_serviceManager->activeServiceId());

    refreshConsoleSection();
}

void DevConsoleWindow::copyConsole()
{
    QApplication::clipboard()->setText(m_consoleOutput->toPlainText());
}

void DevConsoleWindow::reloadPage()
{
    if (m_serviceManager == nullptr)
        return;

    m_serviceManager->reloadActivePage();
    refreshWebEngineSection();
}

void DevConsoleWindow::onActiveServiceChanged(const QString &serviceId)
{
    Q_UNUSED(serviceId);
    refresh();
}

void DevConsoleWindow::onConsoleMessageLogged(const QString &serviceId)
{
    if (m_serviceManager == nullptr || serviceId != m_serviceManager->activeServiceId())
        return;

    refreshConsoleSection();
}

void DevConsoleWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    refresh();
}
