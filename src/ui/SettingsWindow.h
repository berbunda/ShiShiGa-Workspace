#pragma once

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QShowEvent;
class QSpinBox;
class QTabWidget;
class ProfileManagerWidget;
class ServiceManager;
class SettingsManager;
class UserAgentSettingsWidget;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(SettingsManager &settings,
                            ServiceManager *serviceManager,
                            QWidget *parent = nullptr);

signals:
    void settingsApplied();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onApply();
    void onOk();
    void onResetToDefaults();

private:
    QWidget *buildGeneralTab();
    void loadFromManager();
    void loadDefaultValues();
    bool applyToManager();
    bool validateInputs() const;

    SettingsManager &m_settings;
    ServiceManager *m_serviceManager = nullptr;

    QTabWidget *m_tabs = nullptr;
    ProfileManagerWidget *m_profileManager = nullptr;
    UserAgentSettingsWidget *m_userAgentSettings = nullptr;

    QSpinBox *m_fontSizeSpin = nullptr;
    QSpinBox *m_autoUnloadTimeoutSpin = nullptr;
    QCheckBox *m_rememberGeometryCheck = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};
