#include "LicenseWindow.h"

#include "core/AppMetadata.h"
#include "core/LicenseText.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFont>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

LicenseWindow::LicenseWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("GNU GPL v3 License"));
    setModal(false);
    setMinimumSize(640, 480);
    resize(720, 560);
    setupUi();
}

void LicenseWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    m_licenseText = new QPlainTextEdit(this);
    m_licenseText->setReadOnly(true);
    m_licenseText->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_licenseText->setPlainText(loadLicenseText());
    {
        QFont monoFont = m_licenseText->font();
        monoFont.setFamily(QStringLiteral("Consolas"));
        m_licenseText->setFont(monoFont);
    }

    auto *actionsLayout = new QHBoxLayout();
    auto *copyButton = new QPushButton(tr("Copy License"), this);
    auto *openWebsiteButton = new QPushButton(tr("Open Official Website"), this);
    auto *closeButton = new QPushButton(tr("Close"), this);

    actionsLayout->addWidget(copyButton);
    actionsLayout->addWidget(openWebsiteButton);
    actionsLayout->addStretch();
    actionsLayout->addWidget(closeButton);

    rootLayout->addWidget(m_licenseText, 1);
    rootLayout->addLayout(actionsLayout);

    connect(copyButton, &QPushButton::clicked, this, &LicenseWindow::copyLicense);
    connect(openWebsiteButton, &QPushButton::clicked, this, &LicenseWindow::openOfficialWebsite);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}

QString LicenseWindow::loadLicenseText()
{
    return LicenseText::loadGplV3Text();
}

void LicenseWindow::copyLicense()
{
    QApplication::clipboard()->setText(m_licenseText->toPlainText());
}

void LicenseWindow::openOfficialWebsite()
{
    QDesktopServices::openUrl(QUrl(QString::fromLatin1(AppMetadata::kGplOfficialUrl)));
}
