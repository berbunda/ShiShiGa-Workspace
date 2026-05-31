#pragma once

#include <QDialog>

class QLabel;
class QPushButton;

class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);

private slots:
    void openRepository();
    void copyVersionInfo();

private:
    void setupUi();

    QLabel *m_versionLabel = nullptr;
    QPushButton *m_openRepositoryButton = nullptr;
    QPushButton *m_copyVersionInfoButton = nullptr;
};
