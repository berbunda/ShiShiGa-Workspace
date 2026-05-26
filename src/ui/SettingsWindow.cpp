#include "SettingsWindow.h"

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
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(SettingsManager &settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Settings"));
    setModal(false);
    resize(420, 320);

    setupUi();
    loadFromManager();
}

void SettingsWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    auto *appearanceGroup = new QGroupBox(tr("Appearance"), this);
    auto *appearanceForm = new QFormLayout(appearanceGroup);

    m_fontSizeSpin = new QSpinBox(appearanceGroup);
    m_fontSizeSpin->setRange(SettingsManager::kMinFontSize, SettingsManager::kMaxFontSize);
    m_fontSizeSpin->setSuffix(tr(" pt"));
    appearanceForm->addRow(tr("Font size:"), m_fontSizeSpin);

    auto *behaviorGroup = new QGroupBox(tr("Behavior"), this);
    auto *behaviorForm = new QFormLayout(behaviorGroup);

    m_autoUnloadTimeoutSpin = new QSpinBox(behaviorGroup);
    m_autoUnloadTimeoutSpin->setRange(SettingsManager::kMinAutoUnloadTimeoutMinutes,
                                      SettingsManager::kMaxAutoUnloadTimeoutMinutes);
    m_autoUnloadTimeoutSpin->setSuffix(tr(" min"));
    behaviorForm->addRow(tr("Auto unload timeout:"), m_autoUnloadTimeoutSpin);

    m_restoreSessionCheck = new QCheckBox(tr("Restore previous session on startup"), behaviorGroup);
    behaviorForm->addRow(m_restoreSessionCheck);

    auto *windowGroup = new QGroupBox(tr("Window"), this);
    auto *windowForm = new QFormLayout(windowGroup);

    m_rememberGeometryCheck = new QCheckBox(tr("Remember main window geometry"), windowGroup);
    windowForm->addRow(m_rememberGeometryCheck);

    rootLayout->addWidget(appearanceGroup);
    rootLayout->addWidget(behaviorGroup);
    rootLayout->addWidget(windowGroup);
    rootLayout->addStretch();

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
}

void SettingsWindow::loadFromManager()
{
    m_fontSizeSpin->setValue(m_settings.fontSize());
    m_autoUnloadTimeoutSpin->setValue(m_settings.autoUnloadTimeoutMinutes());
    m_restoreSessionCheck->setChecked(m_settings.restorePreviousSession());
    m_rememberGeometryCheck->setChecked(m_settings.rememberMainWindowGeometry());
}

void SettingsWindow::loadDefaultValues()
{
    m_fontSizeSpin->setValue(SettingsManager::kDefaultFontSize);
    m_autoUnloadTimeoutSpin->setValue(SettingsManager::kDefaultAutoUnloadTimeoutMinutes);
    m_restoreSessionCheck->setChecked(SettingsManager::kDefaultRestorePreviousSession);
    m_rememberGeometryCheck->setChecked(SettingsManager::kDefaultRememberMainWindowGeometry);
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

    return true;
}

bool SettingsWindow::applyToManager()
{
    if (!validateInputs())
        return false;

    m_settings.setFontSize(m_fontSizeSpin->value());
    m_settings.setAutoUnloadTimeoutMinutes(m_autoUnloadTimeoutSpin->value());
    m_settings.setRestorePreviousSession(m_restoreSessionCheck->isChecked());
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
        tr("Reset all settings on this page to their default values?"),
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
}
