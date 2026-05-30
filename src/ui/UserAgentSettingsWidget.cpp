#include "UserAgentSettingsWidget.h"

#include "core/SettingsManager.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QVBoxLayout>

UserAgentSettingsWidget::UserAgentSettingsWidget(SettingsManager &settings, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
{
    setupUi();
    loadFromManager();
}

void UserAgentSettingsWidget::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    auto *userAgentGroup = new QGroupBox(tr("User-Agent Settings"), this);
    auto *groupLayout = new QVBoxLayout(userAgentGroup);

    auto *modeLabel = new QLabel(tr("User-Agent Mode:"), userAgentGroup);
    groupLayout->addWidget(modeLabel);

    m_defaultModeRadio = new QRadioButton(tr("Default"), userAgentGroup);
    m_presetModeRadio = new QRadioButton(tr("Preset"), userAgentGroup);
    m_customModeRadio = new QRadioButton(tr("Custom"), userAgentGroup);

    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->addButton(m_defaultModeRadio, static_cast<int>(UserAgentMode::Default));
    m_modeGroup->addButton(m_presetModeRadio, static_cast<int>(UserAgentMode::Preset));
    m_modeGroup->addButton(m_customModeRadio, static_cast<int>(UserAgentMode::Custom));

    auto *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(m_defaultModeRadio);
    modeLayout->addWidget(m_presetModeRadio);
    modeLayout->addWidget(m_customModeRadio);
    modeLayout->addStretch();
    groupLayout->addLayout(modeLayout);

    auto *detailsForm = new QFormLayout();
    m_presetCombo = new QComboBox(userAgentGroup);
    for (const UserAgentPreset &preset : UserAgentSettings::presets()) {
        m_presetCombo->addItem(preset.displayName, preset.id);
    }
    detailsForm->addRow(tr("Preset:"), m_presetCombo);

    m_customUserAgentEdit = new QLineEdit(userAgentGroup);
    m_customUserAgentEdit->setClearButtonEnabled(true);
    m_customUserAgentEdit->setPlaceholderText(tr("Enter a custom User-Agent string"));
    detailsForm->addRow(tr("Custom User-Agent:"), m_customUserAgentEdit);
    groupLayout->addLayout(detailsForm);

    rootLayout->addWidget(userAgentGroup);
    rootLayout->addStretch();

    connect(m_modeGroup, &QButtonGroup::idClicked, this, [this](int) {
        onModeChanged();
    });
    connect(m_customModeRadio, &QRadioButton::clicked, this, &UserAgentSettingsWidget::onCustomModeSelected);
}

void UserAgentSettingsWidget::loadFromManager()
{
    m_lastConfirmedMode = m_settings.userAgentMode();
    setSelectedMode(m_lastConfirmedMode);

    const int presetIndex = m_presetCombo->findData(m_settings.userAgentPresetId());
    m_presetCombo->setCurrentIndex(presetIndex >= 0 ? presetIndex : 0);
    m_customUserAgentEdit->setText(m_settings.customUserAgent());

    updateModeControls();
}

void UserAgentSettingsWidget::loadDefaultValues()
{
    setSelectedMode(UserAgentMode::Default);

    const int presetIndex = m_presetCombo->findData(QString::fromLatin1(UserAgentSettings::kDefaultPresetId));
    m_presetCombo->setCurrentIndex(presetIndex >= 0 ? presetIndex : 0);
    m_customUserAgentEdit->clear();

    updateModeControls();
}

UserAgentMode UserAgentSettingsWidget::selectedMode() const
{
    return static_cast<UserAgentMode>(m_modeGroup->checkedId());
}

QString UserAgentSettingsWidget::selectedPresetId() const
{
    return m_presetCombo->currentData().toString();
}

void UserAgentSettingsWidget::setSelectedMode(UserAgentMode mode)
{
    switch (mode) {
    case UserAgentMode::Preset:
        m_presetModeRadio->setChecked(true);
        break;
    case UserAgentMode::Custom:
        m_customModeRadio->setChecked(true);
        break;
    case UserAgentMode::Default:
        m_defaultModeRadio->setChecked(true);
        break;
    }
}

void UserAgentSettingsWidget::updateModeControls()
{
    const UserAgentMode mode = selectedMode();
    m_presetCombo->setEnabled(mode == UserAgentMode::Preset);
    m_customUserAgentEdit->setEnabled(mode == UserAgentMode::Custom);
}

void UserAgentSettingsWidget::onModeChanged()
{
    updateModeControls();
}

void UserAgentSettingsWidget::onCustomModeSelected()
{
    if (!m_customModeRadio->isChecked() || m_settings.userAgentCustomWarningAcknowledged())
        return;

    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("Custom User-Agent"),
        tr("Changing the User-Agent may cause AI services to behave incorrectly.\n"
           "Use this option only if you understand the consequences."),
        QMessageBox::Ok | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (answer != QMessageBox::Ok) {
        setSelectedMode(m_lastConfirmedMode);
        updateModeControls();
        return;
    }

    m_settings.setUserAgentCustomWarningAcknowledged(true);
    m_lastConfirmedMode = UserAgentMode::Custom;
}

bool UserAgentSettingsWidget::validateInputs() const
{
    if (selectedMode() != UserAgentMode::Custom)
        return true;

    if (!m_customUserAgentEdit->text().trimmed().isEmpty())
        return true;

    QMessageBox::warning(const_cast<UserAgentSettingsWidget *>(this),
                         tr("Invalid value"),
                         tr("Custom User-Agent cannot be empty."));
    return false;
}

bool UserAgentSettingsWidget::applyToManager()
{
    if (!validateInputs())
        return false;

    const UserAgentMode mode = selectedMode();
    const QString presetId = selectedPresetId();
    const QString customUserAgent = m_customUserAgentEdit->text();
    const QString previousResolved = m_settings.resolvedUserAgent();
    const QString newResolved = UserAgentSettings::resolve(mode, presetId, customUserAgent);

    if (previousResolved != newResolved) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this,
            tr("Apply User-Agent"),
            tr("User-Agent changes will take effect only after Reload or reopening all AI services.\n"
               "Continue?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (answer != QMessageBox::Yes)
            return false;
    }

    m_settings.setUserAgentMode(mode);
    m_settings.setUserAgentPresetId(presetId);
    m_settings.setCustomUserAgent(customUserAgent);
    m_lastConfirmedMode = mode;
    return true;
}
