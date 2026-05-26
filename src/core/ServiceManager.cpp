#include "ServiceManager.h"

#include "core/ServiceFaviconProvider.h"
#include "core/SettingsManager.h"
#include "profile/ProfileManager.h"

#include <QDateTime>
#include <QLabel>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineView>

namespace {

QWidget *createPlaceholderWidget(QWidget *parent)
{
    auto *placeholder = new QWidget(parent);
    auto *layout = new QVBoxLayout(placeholder);
    auto *label = new QLabel(QObject::tr("Select a service from the sidebar"), placeholder);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("color: #888; font-size: 16px;"));
    layout->addWidget(label);
    return placeholder;
}

} // namespace

ServiceManager::ServiceManager(QStackedWidget *stackWidget, QObject *parent)
    : QObject(parent)
    , m_stack(stackWidget)
    , m_faviconProvider(new ServiceFaviconProvider(this))
{
    m_placeholder = createPlaceholderWidget(m_stack);
    m_stack->addWidget(m_placeholder);
    m_stack->setCurrentWidget(m_placeholder);

    for (const ServiceDefinition &definition : ServiceRegistry::allServices()) {
        ServiceEntry entry;
        entry.definition = definition;
        entry.state = ServiceState::Closed;
        entry.lastUrl = definition.defaultUrl;
        m_services.insert(definition.id, std::move(entry));
    }

    connect(m_faviconProvider, &ServiceFaviconProvider::faviconChanged, this,
            &ServiceManager::serviceIconChanged);
}

QList<ServiceDefinition> ServiceManager::services() const
{
    QList<ServiceDefinition> definitions;
    definitions.reserve(m_services.size());
    for (const ServiceEntry &entry : m_services)
        definitions.append(entry.definition);
    return definitions;
}

ServiceState ServiceManager::state(const QString &serviceId) const
{
    const ServiceEntry *entry = entryFor(serviceId);
    return entry != nullptr ? entry->state : ServiceState::Closed;
}

QString ServiceManager::activeServiceId() const
{
    return m_activeServiceId;
}

QWebEngineView *ServiceManager::viewFor(const QString &serviceId) const
{
    const ServiceEntry *entry = entryFor(serviceId);
    return entry != nullptr ? entry->view : nullptr;
}

QIcon ServiceManager::iconForService(const QString &serviceId) const
{
    return m_faviconProvider->cachedIcon(serviceId);
}

void ServiceManager::activateService(const QString &serviceId)
{
    ServiceEntry *entry = entryFor(serviceId);
    if (entry == nullptr || !entry->definition.available)
        return;

    if (entry->state == ServiceState::Closed || entry->state == ServiceState::Unloaded)
        createView(*entry);

    if (entry->view == nullptr)
        return;

    entry->state = ServiceState::Loaded;
    entry->lastActiveTime = QDateTime::currentDateTime();
    m_activeServiceId = serviceId;
    m_stack->setCurrentWidget(entry->view);

    updateInactivityTimers();

    if (m_faviconProvider->hasCachedIcon(serviceId))
        emit serviceIconChanged(serviceId, m_faviconProvider->cachedIcon(serviceId));

    emit activeServiceChanged(serviceId);
    emit serviceStateChanged(serviceId, ServiceState::Loaded);
}

void ServiceManager::unloadService(const QString &serviceId)
{
    ServiceEntry *entry = entryFor(serviceId);
    if (entry == nullptr || entry->state != ServiceState::Loaded)
        return;

    if (entry->view != nullptr)
        entry->lastUrl = entry->view->url();

    destroyView(*entry);
    entry->state = ServiceState::Unloaded;
    stopInactivityTimer(*entry);

    if (m_activeServiceId == serviceId) {
        m_activeServiceId.clear();
        showPlaceholder();
    }

    emit serviceStateChanged(serviceId, ServiceState::Unloaded);
}

void ServiceManager::closeService(const QString &serviceId)
{
    ServiceEntry *entry = entryFor(serviceId);
    if (entry == nullptr)
        return;

    if (entry->state == ServiceState::Loaded) {
        if (entry->view != nullptr)
            entry->lastUrl = entry->view->url();
        destroyView(*entry);
        stopInactivityTimer(*entry);
    }

    entry->state = ServiceState::Closed;

    if (m_activeServiceId == serviceId) {
        m_activeServiceId.clear();
        showPlaceholder();
    }

    emit serviceStateChanged(serviceId, ServiceState::Closed);
}

ServiceManager::ServiceEntry *ServiceManager::entryFor(const QString &serviceId)
{
    auto it = m_services.find(serviceId);
    return it != m_services.end() ? &(*it) : nullptr;
}

const ServiceManager::ServiceEntry *ServiceManager::entryFor(const QString &serviceId) const
{
    auto it = m_services.constFind(serviceId);
    return it != m_services.constEnd() ? &(*it) : nullptr;
}

void ServiceManager::createView(ServiceEntry &entry)
{
    if (entry.view != nullptr)
        return;

    QWebEngineProfile *profile = ProfileManager::instance().profileFor(entry.definition.id);
    if (profile == nullptr)
        return;

    entry.page = new QWebEnginePage(profile, m_stack);
    entry.view = new QWebEngineView(m_stack);
    entry.view->setPage(entry.page);

    const QString serviceId = entry.definition.id;
    connect(entry.view, &QWebEngineView::urlChanged, this, [this, serviceId](const QUrl &url) {
        ServiceEntry *trackedEntry = entryFor(serviceId);
        if (trackedEntry != nullptr && url.isValid())
            trackedEntry->lastUrl = url;
    });

    m_stack->addWidget(entry.view);
    m_faviconProvider->bindPage(serviceId, entry.page);

    const QUrl targetUrl = entry.lastUrl.isValid() ? entry.lastUrl : entry.definition.defaultUrl;
    entry.view->load(targetUrl);
}

void ServiceManager::destroyView(ServiceEntry &entry)
{
    if (entry.view == nullptr)
        return;

    m_faviconProvider->unbindPage(entry.definition.id);

    m_stack->removeWidget(entry.view);
    entry.view->deleteLater();
    entry.page->deleteLater();
    entry.view = nullptr;
    entry.page = nullptr;
}

void ServiceManager::showPlaceholder()
{
    m_stack->setCurrentWidget(m_placeholder);
}

void ServiceManager::updateInactivityTimers()
{
    for (auto it = m_services.begin(); it != m_services.end(); ++it) {
        ServiceEntry &entry = it.value();
        if (entry.state == ServiceState::Loaded && it.key() != m_activeServiceId)
            startInactivityTimer(entry);
        else
            stopInactivityTimer(entry);
    }
}

void ServiceManager::startInactivityTimer(ServiceEntry &entry)
{
    if (entry.inactivityTimer == nullptr) {
        entry.inactivityTimer = new QTimer(this);
        entry.inactivityTimer->setSingleShot(true);
        entry.inactivityTimer->setInterval(SettingsManager::instance().autoUnloadTimeoutMs());
        connect(entry.inactivityTimer, &QTimer::timeout, this, [this, serviceId = entry.definition.id]() {
            if (m_activeServiceId == serviceId)
                return;
            if (state(serviceId) == ServiceState::Loaded)
                unloadService(serviceId);
        });
    }

    entry.inactivityTimer->start();
}

void ServiceManager::stopInactivityTimer(ServiceEntry &entry)
{
    if (entry.inactivityTimer != nullptr)
        entry.inactivityTimer->stop();
}

void ServiceManager::refreshInactivityTimeouts()
{
    const int timeoutMs = SettingsManager::instance().autoUnloadTimeoutMs();

    for (auto it = m_services.begin(); it != m_services.end(); ++it) {
        ServiceEntry &entry = it.value();
        if (entry.inactivityTimer == nullptr)
            continue;

        entry.inactivityTimer->setInterval(timeoutMs);
        if (entry.inactivityTimer->isActive())
            entry.inactivityTimer->start();
    }
}
