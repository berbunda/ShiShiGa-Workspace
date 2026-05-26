#include "ServiceButton.h"

#include "core/ServiceFaviconProvider.h"

#include <QHBoxLayout>
#include <QToolButton>

namespace {

constexpr int kStateButtonSize = 24;

} // namespace

ServiceButton::ServiceButton(const QString &serviceId,
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
    m_serviceButton->setIconSize(QSize(kIconLogicalSize, kIconLogicalSize));
    m_serviceButton->setFixedSize(kIconLogicalSize + 8, kIconLogicalSize + 8);
    m_serviceButton->setAutoRaise(true);
    m_serviceButton->setEnabled(available);
    m_serviceButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_stateButton = new QToolButton(this);
    m_stateButton->setFixedSize(kStateButtonSize, kStateButtonSize);
    m_stateButton->setAutoRaise(true);
    m_stateButton->setVisible(false);

    layout->addWidget(m_serviceButton);
    layout->addWidget(m_stateButton);
    layout->addStretch();

    applyPlaceholderIcon();

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

QString ServiceButton::serviceId() const
{
    return m_serviceId;
}

void ServiceButton::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;
    updateStyles();
}

void ServiceButton::setServiceState(ServiceState state)
{
    if (m_state == state)
        return;

    m_state = state;
    updateStateButton();
    updateStyles();
}

void ServiceButton::setServiceIcon(const QIcon &icon)
{
    if (icon.isNull())
        return;

    const qint64 cacheKey = icon.cacheKey();
    if (cacheKey == m_iconCacheKey)
        return;

    m_iconCacheKey = cacheKey;
    m_serviceButton->setIcon(icon);
}

void ServiceButton::applyPlaceholderIcon()
{
    m_iconCacheKey = 0;
    m_serviceButton->setIcon(ServiceFaviconProvider::placeholderIcon(m_displayName, kIconLogicalSize));
}

void ServiceButton::updateStateButton()
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

void ServiceButton::updateStyles()
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
