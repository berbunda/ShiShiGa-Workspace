#include "AboutWindow.h"

#include "core/AppMetadata.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About %1").arg(QString::fromLatin1(AppMetadata::kApplicationDisplayName)));
    setModal(false);
    setMinimumSize(480, 520);
    resize(520, 580);
    setupUi();
}

void AboutWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel(
        QString::fromLatin1("<h2>%1</h2>").arg(AppMetadata::kApplicationDisplayName), this);
    titleLabel->setTextFormat(Qt::RichText);

    m_versionLabel = new QLabel(this);
    m_versionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_versionLabel->setText(
        QStringLiteral(
            "Version: %1<br>"
            "Commit: %2<br>"
            "Build date: %3<br>"
            "Platform: %4")
            .arg(AppMetadata::applicationVersion(),
                 AppMetadata::gitCommit(),
                 AppMetadata::buildDate(),
                 AppMetadata::platformDescription()));

    auto *repositoryGroup = new QGroupBox(tr("GitHub Repository"), this);
    auto *repositoryLayout = new QVBoxLayout(repositoryGroup);
    m_openRepositoryButton = new QPushButton(tr("Open Repository"), repositoryGroup);
    repositoryLayout->addWidget(m_openRepositoryButton);
    connect(m_openRepositoryButton, &QPushButton::clicked, this, &AboutWindow::openRepository);

    auto *technologiesGroup = new QGroupBox(tr("Technologies"), this);
    auto *technologiesLayout = new QVBoxLayout(technologiesGroup);
    auto *technologiesLabel = new QLabel(
        QStringLiteral(
            "<ul>"
            "<li>C++23</li>"
            "<li>Qt %1</li>"
            "<li>%2</li>"
            "<li>%3</li>"
            "</ul>")
            .arg(AppMetadata::qtRuntimeVersion(),
                 AppMetadata::compilerLabel(),
                 AppMetadata::cmakeVersionLabel()),
        technologiesGroup);
    technologiesLabel->setTextFormat(Qt::RichText);
    technologiesLayout->addWidget(technologiesLabel);

    auto *aiGroup = new QGroupBox(tr("AI-assisted development"), this);
    auto *aiLayout = new QVBoxLayout(aiGroup);
    auto *aiLabel = new QLabel(
        tr("Developed with AI-assisted development tools:") + QStringLiteral(
            "<ul>"
            "<li>Cursor</li>"
            "<li>ChatGPT</li>"
            "</ul>"),
        aiGroup);
    aiLabel->setTextFormat(Qt::RichText);
    aiLayout->addWidget(aiLabel);

    auto *licenseGroup = new QGroupBox(tr("License"), this);
    auto *licenseLayout = new QVBoxLayout(licenseGroup);
    auto *licenseLabel = new QLabel(QString::fromLatin1(AppMetadata::kLicenseShortName),
                                    licenseGroup);
    licenseLayout->addWidget(licenseLabel);

    m_copyVersionInfoButton = new QPushButton(tr("Copy Version Info"), this);
    connect(m_copyVersionInfoButton, &QPushButton::clicked, this, &AboutWindow::copyVersionInfo);

    auto *actionsLayout = new QHBoxLayout();
    actionsLayout->addStretch();
    auto *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    actionsLayout->addWidget(m_copyVersionInfoButton);
    actionsLayout->addWidget(closeButton);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(m_versionLabel);
    rootLayout->addWidget(repositoryGroup);
    rootLayout->addWidget(technologiesGroup);
    rootLayout->addWidget(aiGroup);
    rootLayout->addWidget(licenseGroup);
    rootLayout->addStretch();
    rootLayout->addLayout(actionsLayout);
}

void AboutWindow::openRepository()
{
    QDesktopServices::openUrl(QUrl(QString::fromLatin1(AppMetadata::kGitHubRepositoryUrl)));
}

void AboutWindow::copyVersionInfo()
{
    QApplication::clipboard()->setText(AppMetadata::versionInfoClipboardText());
}
