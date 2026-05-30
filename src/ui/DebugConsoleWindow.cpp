#include "DebugConsoleWindow.h"

#include "core/ServiceManager.h"
#include "diagnostics/DiagnosticsInfo.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFont>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QUrl>
#include <QVBoxLayout>

DebugConsoleWindow::DebugConsoleWindow(ServiceManager *serviceManager, QWidget *parent)
    : QDialog(parent)
    , m_serviceManager(serviceManager)
{
    setWindowTitle(tr("Debug Console"));
    setModal(false);
    setMinimumSize(640, 480);
    resize(680, 520);
    setupUi();
    connectSignals();
    refresh();
}

void DebugConsoleWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setLineWrapMode(QPlainTextEdit::NoWrap);
    {
        QFont monoFont = m_output->font();
        monoFont.setFamily(QStringLiteral("Consolas"));
        m_output->setFont(monoFont);
    }

    auto *actionsLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_copyButton = new QPushButton(tr("Copy Debug Info"), this);
    m_openProfileButton = new QPushButton(tr("Open Profile Folder"), this);
    m_openCrashesButton = new QPushButton(tr("Open Crashes Folder"), this);
    m_closeButton = new QPushButton(tr("Close"), this);

    actionsLayout->addWidget(m_refreshButton);
    actionsLayout->addWidget(m_copyButton);
    actionsLayout->addWidget(m_openProfileButton);
    actionsLayout->addWidget(m_openCrashesButton);
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_closeButton);

    rootLayout->addWidget(m_output, 1);
    rootLayout->addLayout(actionsLayout);

    connect(m_refreshButton, &QPushButton::clicked, this, &DebugConsoleWindow::refresh);
    connect(m_copyButton, &QPushButton::clicked, this, &DebugConsoleWindow::copyDebugInfo);
    connect(m_openProfileButton, &QPushButton::clicked, this, &DebugConsoleWindow::openProfileFolder);
    connect(m_openCrashesButton, &QPushButton::clicked, this, &DebugConsoleWindow::openCrashesFolder);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void DebugConsoleWindow::connectSignals()
{
    if (m_serviceManager == nullptr)
        return;

    connect(m_serviceManager, &ServiceManager::activeServiceChanged,
            this, &DebugConsoleWindow::onActiveServiceChanged);
    connect(m_serviceManager, &ServiceManager::serviceStateChanged,
            this, [this](const QString &, ServiceState) { refresh(); });
}

void DebugConsoleWindow::refresh()
{
    const DebugDiagnosticsSnapshot snapshot = DiagnosticsInfo::collect(m_serviceManager);
    m_output->setPlainText(DiagnosticsInfo::formatDebugReport(snapshot));
}

void DebugConsoleWindow::copyDebugInfo()
{
    QApplication::clipboard()->setText(m_output->toPlainText());
}

void DebugConsoleWindow::openProfileFolder()
{
    const QString folderPath = DiagnosticsInfo::profileFolderToOpen(m_serviceManager);
    if (folderPath.isEmpty() || !QDir(folderPath).exists()) {
        QMessageBox::warning(this,
                             tr("Open Profile Folder"),
                             tr("Profile folder is unavailable."));
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath))) {
        QMessageBox::warning(this,
                             tr("Open Profile Folder"),
                             tr("Unable to open folder:\n%1").arg(folderPath));
    }
}

void DebugConsoleWindow::openCrashesFolder()
{
    const DebugDiagnosticsSnapshot snapshot = DiagnosticsInfo::collect(m_serviceManager);
    const QString folderPath = snapshot.runtime.crashLogsDirectory;
    if (folderPath.isEmpty() || folderPath == QStringLiteral("N/A")) {
        QMessageBox::warning(this,
                             tr("Open Crashes Folder"),
                             tr("Crash logs folder is unavailable."));
        return;
    }

    QDir().mkpath(folderPath);
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath))) {
        QMessageBox::warning(this,
                             tr("Open Crashes Folder"),
                             tr("Unable to open folder:\n%1").arg(folderPath));
    }
}

void DebugConsoleWindow::onActiveServiceChanged(const QString &serviceId)
{
    Q_UNUSED(serviceId);
    refresh();
}

void DebugConsoleWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    refresh();
}
