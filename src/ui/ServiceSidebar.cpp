#include "ServiceSidebar.h"

#include "ServiceButton.h"
#include "SidebarAddButton.h"
#include "core/ServiceManager.h"
#include "core/SettingsManager.h"
#include "services/ServiceRegistry.h"

#include <QFrame>
#include <QMenu>
#include <QScrollArea>
#include <QVBoxLayout>

ServiceSidebar::ServiceSidebar(ServiceManager *serviceManager,
                               SettingsManager &settings,
                               QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_serviceManager(serviceManager)
{
    setFixedWidth(m_settings.sidebarWidth());
    setStyleSheet(QStringLiteral("ServiceSidebar { background: #1e1e1e; border-right: 1px solid #333; }"));

    setupLayout();
    setupAddButton();

    for (const ServiceCatalogEntry &service : m_serviceManager->openServices())
        addServiceButton(service.id);

    connect(m_serviceManager, &ServiceManager::serviceAdded, this, &ServiceSidebar::addServiceButton);
    connect(m_serviceManager, &ServiceManager::serviceRemoved, this, &ServiceSidebar::removeServiceButton);
    connect(m_serviceManager, &ServiceManager::serviceStateChanged, this, &ServiceSidebar::syncButtonState);
    connect(m_serviceManager, &ServiceManager::activeServiceChanged, this, &ServiceSidebar::syncActiveButton);
    connect(m_serviceManager, &ServiceManager::serviceIconChanged, this, &ServiceSidebar::syncButtonIcon);
}

void ServiceSidebar::setupLayout()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_addHeader = new QWidget(this);
    m_addHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_servicesScrollArea = new QScrollArea(this);
    m_servicesScrollArea->setWidgetResizable(true);
    m_servicesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_servicesScrollArea->setFrameShape(QFrame::NoFrame);
    m_servicesScrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"));
    m_servicesScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_servicesContainer = new QWidget(m_servicesScrollArea);
    m_servicesLayout = new QVBoxLayout(m_servicesContainer);
    m_servicesLayout->setContentsMargins(0, 0, 0, 0);
    m_servicesLayout->setSpacing(ServiceButton::kContentSpacing);
    m_servicesLayout->addStretch();

    m_servicesScrollArea->setWidget(m_servicesContainer);

    rootLayout->addWidget(m_addHeader);
    rootLayout->addWidget(m_servicesScrollArea, 1);
}

void ServiceSidebar::setupAddButton()
{
    auto *addLayout = new QVBoxLayout(m_addHeader);
    addLayout->setContentsMargins(ServiceButton::kHorizontalMargin,
                                  ServiceButton::kVerticalMargin,
                                  ServiceButton::kHorizontalMargin,
                                  ServiceButton::kVerticalMargin);
    addLayout->setSpacing(0);

    m_addButton = new SidebarAddButton(m_addHeader);
    addLayout->addWidget(m_addButton, 0, Qt::AlignHCenter);

    const QSize addButtonSize = m_addButton->sizeHint();
    m_addHeader->setFixedHeight(addButtonSize.height() + ServiceButton::kVerticalMargin * 2);

    connect(m_addButton, &SidebarAddButton::clicked, this, &ServiceSidebar::showAddServiceMenu);
}

int ServiceSidebar::nextServiceInsertIndex() const
{
    return qMax(0, m_servicesLayout->count() - 1);
}

void ServiceSidebar::showAddServiceMenu()
{
    QMenu menu(this);

    for (const ServiceCatalogEntry &entry : ServiceRegistry::launchableServices()) {
        QAction *action = menu.addAction(entry.displayName);
        if (m_serviceManager->hasOpenService(entry.id))
            action->setEnabled(false);

        connect(action, &QAction::triggered, this, [this, serviceId = entry.id]() {
            m_serviceManager->openService(serviceId);
        });
    }

    if (menu.isEmpty())
        return;

    menu.exec(m_addButton->mapToGlobal(QPoint(0, m_addButton->height())));
}

void ServiceSidebar::addServiceButton(const QString &serviceId)
{
    if (m_buttons.contains(serviceId))
        return;

    const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(serviceId);
    if (catalog.id.isEmpty())
        return;

    auto *button = new ServiceButton(serviceId, catalog.displayName, m_servicesContainer);
    m_buttons.insert(serviceId, button);

    m_servicesLayout->insertWidget(nextServiceInsertIndex(), button);

    syncButtonState(serviceId);
    syncButtonIcon(serviceId);
    syncActiveButton(m_serviceManager->activeServiceId());

    connect(button, &ServiceButton::activateRequested, m_serviceManager, &ServiceManager::activateService);
    connect(button, &ServiceButton::unloadRequested, m_serviceManager, &ServiceManager::unloadService);
    connect(button, &ServiceButton::closeRequested, m_serviceManager, &ServiceManager::closeService);
}

void ServiceSidebar::removeServiceButton(const QString &serviceId)
{
    ServiceButton *button = m_buttons.take(serviceId);
    if (button == nullptr)
        return;

    m_servicesLayout->removeWidget(button);
    button->deleteLater();
}

void ServiceSidebar::syncButtonState(const QString &serviceId)
{
    ServiceButton *button = m_buttons.value(serviceId);
    if (button == nullptr)
        return;

    button->setServiceState(m_serviceManager->state(serviceId));
}

void ServiceSidebar::syncButtonIcon(const QString &serviceId)
{
    ServiceButton *button = m_buttons.value(serviceId);
    if (button == nullptr)
        return;

    const QIcon icon = m_serviceManager->iconForService(serviceId);
    if (!icon.isNull())
        button->setServiceIcon(icon);
}

void ServiceSidebar::syncActiveButton(const QString &serviceId)
{
    for (auto it = m_buttons.constBegin(); it != m_buttons.constEnd(); ++it)
        it.value()->setActive(it.key() == serviceId);
}
