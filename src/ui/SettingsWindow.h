#pragma once

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QShowEvent;
class QSpinBox;
class SettingsManager;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(SettingsManager &settings, QWidget *parent = nullptr);

signals:
    void settingsApplied();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onApply();
    void onOk();
    void onResetToDefaults();

private:
    void setupUi();
    void loadFromManager();
    void loadDefaultValues();
    bool applyToManager();
    bool validateInputs() const;

    SettingsManager &m_settings;

    QSpinBox *m_fontSizeSpin = nullptr;
    QSpinBox *m_autoUnloadTimeoutSpin = nullptr;
    QCheckBox *m_restoreSessionCheck = nullptr;
    QCheckBox *m_rememberGeometryCheck = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};
