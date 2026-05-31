#pragma once

#include <QDialog>

class QPlainTextEdit;
class QPushButton;

class LicenseWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LicenseWindow(QWidget *parent = nullptr);

private slots:
    void copyLicense();
    void openOfficialWebsite();

private:
    void setupUi();
    static QString loadLicenseText();

    QPlainTextEdit *m_licenseText = nullptr;
};
