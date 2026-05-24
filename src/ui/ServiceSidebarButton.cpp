#include "ServiceSidebarButton.h"

#include <QHBoxLayout>
#include <QToolButton>

namespace {

constexpr int kServiceIconSize = 40;
constexpr int kStateButtonSize = 24;

} // namespace

ServiceSidebarButton::ServiceSidebarButton(const QString &serviceId,
                                           const QString &displayName,
                                           bool available,
                                           QWidget *parent)
    : QWidget(parent)
    , m_serviceId(serviceId)
    , m_displayName(displayName)
    , m_available(available)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    m_serviceButton = new QToolButton(this);
    m_serviceButton->setToolTip(displayName);
    m_serviceButton->setIconSize(QSize(kServiceIconSize, kServiceIconSize));
    m_serviceButton->setFixedSize(kServiceIconSize + 8, kServiceIconSize + 8);
    m_serviceButton->setAutoRaise(true);
    m_serviceButton->setEnabled(available);
    m_serviceButton->setText(displayName.left(1));
    m_serviceButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_stateButton = new QToolButton(this);
    m_stateButton->setFixedSize(kStateButtonSize, kStateButtonSize);
    m_stateButton->setAutoRaise(true);
    m_stateButton->setVisible(false);

    layout->addWidget(m_serviceButton);
    layout->addWidget(m_stateButton);
    layout->addStretch();

    connect(m_serviceButton, &QToolButton::clicked, this, [this]() {
        emit activateRequested(m_serviceId);
    });

    connect(m_stateButton, &QToolButton::clicked, this, [this]() {
        if (m_state == ServiceState::Loaded)
            emit unloadRequested(m_serviceId);
        else if (m_state == ServiceState::Unloaded)
            emit closeRequested(m_serviceId);
    });

    updateStyles();
    updateStateButton();
}

QString ServiceSidebarButton::serviceId() const
{
    return m_serviceId;
}

void ServiceSidebarButton::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    updateStyles();
}

void ServiceSidebarButton::setServiceState(ServiceState state)
{
    if (m_state == state)
        return;

    m_state = state;
    updateStateButton();
    updateStyles();
}

void ServiceSidebarButton::setServiceIcon(const QIcon &icon)
{
    if (icon.isNull())
        return;

    m_serviceButton->setIcon(icon);
    m_serviceButton->setText({});
}

void ServiceSidebarButton::updateStateButton()
{
    const bool showStateButton = m_available
        && (m_state == ServiceState::Loaded || m_state == ServiceState::Unloaded);
    m_stateButton->setVisible(showStateButton);

    if (m_state == ServiceState::Loaded) {
        m_stateButton->setText(QStringLiteral("⏻"));
        m_stateButton->setToolTip(tr("Unload %1 from memory").arg(m_displayName));
    } else if (m_state == ServiceState::Unloaded) {
        m_stateButton->setText(QStringLiteral("✕"));
        m_stateButton->setToolTip(tr("Close %1 tab").arg(m_displayName));
    }
}

void ServiceSidebarButton::updateStyles()
{
    QString serviceStyle = QStringLiteral(
        "QToolButton { border-radius: 10px; padding: 4px; }"
        "QToolButton:disabled { color: #666; }");

    if (m_active) {
        serviceStyle += QStringLiteral(
            "QToolButton { background: #2d6cdf; }"
            "QToolButton:hover { background: #3b79ea; }");
    } else if (m_available) {
        serviceStyle += QStringLiteral(
            "QToolButton:hover { background: #333; }");
    }

    m_serviceButton->setStyleSheet(serviceStyle);

    m_stateButton->setStyleSheet(QStringLiteral(
        "QToolButton { border-radius: 6px; font-size: 14px; color: #ddd; }"
        "QToolButton:hover { background: #444; }"));
}
