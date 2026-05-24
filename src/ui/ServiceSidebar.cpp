#include "ServiceSidebar.h"

#include "ServiceSidebarButton.h"
#include "core/ServiceManager.h"

#include <QFrame>
#include <QHash>
#include <QScrollArea>
#include <QVBoxLayout>

ServiceSidebar::ServiceSidebar(ServiceManager *serviceManager, QWidget *parent)
    : QWidget(parent)
    , m_serviceManager(serviceManager)
{
    setFixedWidth(88);
    setStyleSheet(QStringLiteral("ServiceSidebar { background: #1e1e1e; border-right: 1px solid #333; }"));

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"));

    m_buttonContainer = new QWidget(scrollArea);
    auto *buttonLayout = new QVBoxLayout(m_buttonContainer);
    buttonLayout->setContentsMargins(0, 12, 0, 12);
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch();

    scrollArea->setWidget(m_buttonContainer);
    outerLayout->addWidget(scrollArea);

    rebuildButtons();

    connect(m_serviceManager, &ServiceManager::serviceStateChanged, this, &ServiceSidebar::syncButtonState);
    connect(m_serviceManager, &ServiceManager::activeServiceChanged, this, [this](const QString &serviceId) {
        for (auto it = m_buttons.constBegin(); it != m_buttons.constEnd(); ++it)
            it.value()->setActive(it.key() == serviceId);
    });
    connect(m_serviceManager, &ServiceManager::serviceIconChanged, this,
            [this](const QString &serviceId, const QIcon &icon) {
                if (auto *button = m_buttons.value(serviceId))
                    button->setServiceIcon(icon);
            });
}

void ServiceSidebar::rebuildButtons()
{
    auto *buttonLayout = qobject_cast<QVBoxLayout *>(m_buttonContainer->layout());
    if (buttonLayout == nullptr)
        return;

    while (buttonLayout->count() > 1) {
        QLayoutItem *item = buttonLayout->takeAt(0);
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
        delete item;
    }

    m_buttons.clear();

    for (const ServiceDefinition &definition : m_serviceManager->services()) {
        auto *button = new ServiceSidebarButton(definition.id,
                                                definition.displayName,
                                                definition.available,
                                                m_buttonContainer);
        m_buttons.insert(definition.id, button);
        buttonLayout->insertWidget(buttonLayout->count() - 1, button);

        syncButtonState(definition.id);

        connect(button, &ServiceSidebarButton::activateRequested, m_serviceManager,
                &ServiceManager::activateService);
        connect(button, &ServiceSidebarButton::unloadRequested, m_serviceManager,
                &ServiceManager::unloadService);
        connect(button, &ServiceSidebarButton::closeRequested, m_serviceManager,
                &ServiceManager::closeService);
    }
}

void ServiceSidebar::syncButtonState(const QString &serviceId)
{
    ServiceSidebarButton *button = m_buttons.value(serviceId);
    if (button == nullptr)
        return;

    button->setServiceState(m_serviceManager->state(serviceId));
    button->setActive(m_serviceManager->activeServiceId() == serviceId);
}
