#pragma once

#include "core/UserAgentSettings.h"

#include <QWidget>

class QButtonGroup;
class QComboBox;
class QLineEdit;
class QRadioButton;
class SettingsManager;

class UserAgentSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserAgentSettingsWidget(SettingsManager &settings, QWidget *parent = nullptr);

    void loadFromManager();
    void loadDefaultValues();
    bool validateInputs() const;
    bool applyToManager();

private slots:
    void onModeChanged();
    void onCustomModeSelected();

private:
    void setupUi();
    void updateModeControls();
    UserAgentMode selectedMode() const;
    QString selectedPresetId() const;
    void setSelectedMode(UserAgentMode mode);

    SettingsManager &m_settings;
    UserAgentMode m_lastConfirmedMode = UserAgentMode::Default;

    QRadioButton *m_defaultModeRadio = nullptr;
    QRadioButton *m_presetModeRadio = nullptr;
    QRadioButton *m_customModeRadio = nullptr;
    QButtonGroup *m_modeGroup = nullptr;
    QComboBox *m_presetCombo = nullptr;
    QLineEdit *m_customUserAgentEdit = nullptr;
};
