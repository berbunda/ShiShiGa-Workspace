#include "SettingsWindow.h"

#include "ProfileManagerWidget.h"
#include "UserAgentSettingsWidget.h"
#include "core/SettingsManager.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(SettingsManager &settings,
                               ServiceManager *serviceManager,
                               QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_serviceManager(serviceManager)
{
    setWindowTitle(tr("Settings"));
    setModal(false);
    resize(760, 520);

    auto *rootLayout = new QVBoxLayout(this);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildGeneralTab(), tr("General"));
    m_profileManager = new ProfileManagerWidget(m_serviceManager, this);
    m_tabs->addTab(m_profileManager, tr("Profile Manager"));
    m_userAgentSettings = new UserAgentSettingsWidget(m_settings, this);
    m_tabs->addTab(m_userAgentSettings, tr("WebEngine"));

    rootLayout->addWidget(m_tabs, 1);

    auto *actionsLayout = new QHBoxLayout();
    auto *resetButton = new QPushButton(tr("Reset to defaults"), this);
    actionsLayout->addWidget(resetButton);
    actionsLayout->addStretch();

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply
                                       | QDialogButtonBox::Ok
                                       | QDialogButtonBox::Cancel,
                                       this);
    actionsLayout->addWidget(m_buttonBox);

    rootLayout->addLayout(actionsLayout);

    connect(resetButton, &QPushButton::clicked, this, &SettingsWindow::onResetToDefaults);
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton *button) {
        switch (m_buttonBox->buttonRole(button)) {
        case QDialogButtonBox::ApplyRole:
            onApply();
            break;
        case QDialogButtonBox::AcceptRole:
            onOk();
            break;
        case QDialogButtonBox::RejectRole:
            reject();
            break;
        default:
            break;
        }
    });

    loadFromManager();
}

QWidget *SettingsWindow::buildGeneralTab()
{
    auto *generalTab = new QWidget(this);

    auto *appearanceGroup = new QGroupBox(tr("Appearance"), generalTab);
    auto *appearanceForm = new QFormLayout(appearanceGroup);

    m_fontSizeSpin = new QSpinBox(appearanceGroup);
    m_fontSizeSpin->setRange(SettingsManager::kMinFontSize, SettingsManager::kMaxFontSize);
    m_fontSizeSpin->setSuffix(tr(" pt"));
    appearanceForm->addRow(tr("Font size:"), m_fontSizeSpin);

    auto *behaviorGroup = new QGroupBox(tr("Behavior"), generalTab);
    auto *behaviorForm = new QFormLayout(behaviorGroup);

    m_autoUnloadTimeoutSpin = new QSpinBox(behaviorGroup);
    m_autoUnloadTimeoutSpin->setRange(SettingsManager::kMinAutoUnloadTimeoutMinutes,
                                      SettingsManager::kMaxAutoUnloadTimeoutMinutes);
    m_autoUnloadTimeoutSpin->setSuffix(tr(" min"));
    behaviorForm->addRow(tr("Auto unload timeout:"), m_autoUnloadTimeoutSpin);

    auto *windowGroup = new QGroupBox(tr("Window"), generalTab);
    auto *windowForm = new QFormLayout(windowGroup);

    m_rememberGeometryCheck = new QCheckBox(tr("Remember main window geometry"), windowGroup);
    windowForm->addRow(m_rememberGeometryCheck);

    auto *tabLayout = new QVBoxLayout(generalTab);
    tabLayout->addWidget(appearanceGroup);
    tabLayout->addWidget(behaviorGroup);
    tabLayout->addWidget(windowGroup);
    tabLayout->addStretch();

    return generalTab;
}

void SettingsWindow::loadFromManager()
{
    m_fontSizeSpin->setValue(m_settings.fontSize());
    m_autoUnloadTimeoutSpin->setValue(m_settings.autoUnloadTimeoutMinutes());
    m_rememberGeometryCheck->setChecked(m_settings.rememberMainWindowGeometry());

    if (m_userAgentSettings != nullptr)
        m_userAgentSettings->loadFromManager();
}

void SettingsWindow::loadDefaultValues()
{
    m_fontSizeSpin->setValue(SettingsManager::kDefaultFontSize);
    m_autoUnloadTimeoutSpin->setValue(SettingsManager::kDefaultAutoUnloadTimeoutMinutes);
    m_rememberGeometryCheck->setChecked(SettingsManager::kDefaultRememberMainWindowGeometry);

    if (m_userAgentSettings != nullptr)
        m_userAgentSettings->loadDefaultValues();
}

bool SettingsWindow::validateInputs() const
{
    if (m_fontSizeSpin->value() < SettingsManager::kMinFontSize
        || m_fontSizeSpin->value() > SettingsManager::kMaxFontSize) {
        QMessageBox::warning(const_cast<SettingsWindow *>(this),
                             tr("Invalid value"),
                             tr("Font size must be between %1 and %2 points.")
                                 .arg(SettingsManager::kMinFontSize)
                                 .arg(SettingsManager::kMaxFontSize));
        return false;
    }

    if (m_autoUnloadTimeoutSpin->value() < SettingsManager::kMinAutoUnloadTimeoutMinutes
        || m_autoUnloadTimeoutSpin->value() > SettingsManager::kMaxAutoUnloadTimeoutMinutes) {
        QMessageBox::warning(const_cast<SettingsWindow *>(this),
                             tr("Invalid value"),
                             tr("Auto unload timeout must be between %1 and %2 minutes.")
                                 .arg(SettingsManager::kMinAutoUnloadTimeoutMinutes)
                                 .arg(SettingsManager::kMaxAutoUnloadTimeoutMinutes));
        return false;
    }

    if (m_userAgentSettings != nullptr && !m_userAgentSettings->validateInputs())
        return false;

    return true;
}

bool SettingsWindow::applyToManager()
{
    if (!validateInputs())
        return false;

    if (m_userAgentSettings != nullptr && !m_userAgentSettings->applyToManager())
        return false;

    m_settings.setFontSize(m_fontSizeSpin->value());
    m_settings.setAutoUnloadTimeoutMinutes(m_autoUnloadTimeoutSpin->value());
    m_settings.setRememberMainWindowGeometry(m_rememberGeometryCheck->isChecked());
    m_settings.save();

    emit settingsApplied();
    return true;
}

void SettingsWindow::onApply()
{
    applyToManager();
}

void SettingsWindow::onOk()
{
    if (applyToManager())
        accept();
}

void SettingsWindow::onResetToDefaults()
{
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("Reset to defaults"),
        tr("Reset all settings on the General tab to their default values?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    loadDefaultValues();
}

void SettingsWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    loadFromManager();

    if (m_profileManager != nullptr) {
        m_profileManager->refreshProfileList();
        m_profileManager->refreshRuntimeStates();
    }
}
