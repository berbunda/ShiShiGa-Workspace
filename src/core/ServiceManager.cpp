#include "ServiceManager.h"

#include "core/ServiceFaviconProvider.h"
#include "core/SettingsManager.h"
#include "core/UserAgentSettings.h"
#include "diagnostics/DiagnosticsWebEnginePage.h"
#include "diagnostics/WebEngineConsoleLog.h"
#include "logging/CrashLogger.h"
#include "profile/ProfileManager.h"

#include <QDateTime>
#include <QLabel>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace {

QWidget *createEmptyWorkspaceWidget(QWidget *parent)
{
    auto *widget = new QWidget(parent);
    auto *layout = new QVBoxLayout(widget);
    auto *label = new QLabel(QObject::tr("Click + to add a service"), widget);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("color: #888; font-size: 16px;"));
    layout->addWidget(label);
    return widget;
}

} // namespace

ServiceManager::ServiceManager(QStackedWidget *stackWidget, QObject *parent)
    : QObject(parent)
    , m_stack(stackWidget)
    , m_faviconProvider(new ServiceFaviconProvider(this))
    , m_consoleLog(new WebEngineConsoleLog(this))
{
    m_emptyWorkspace = createEmptyWorkspaceWidget(m_stack);
    m_stack->addWidget(m_emptyWorkspace);
    m_stack->setCurrentWidget(m_emptyWorkspace);

    connect(m_faviconProvider, &ServiceFaviconProvider::faviconChanged, this,
            &ServiceManager::serviceIconChanged);
}

QList<ServiceCatalogEntry> ServiceManager::openServices() const
{
    QList<ServiceCatalogEntry> services;
    services.reserve(m_services.size());
    for (const ServiceEntry &entry : m_services)
        services.append(entry.catalog);
    return services;
}

bool ServiceManager::hasOpenService(const QString &serviceId) const
{
    return m_services.contains(serviceId);
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

QWebEnginePage *ServiceManager::pageFor(const QString &serviceId) const
{
    const ServiceEntry *entry = entryFor(serviceId);
    return entry != nullptr ? entry->page : nullptr;
}

int ServiceManager::loadedServiceCount() const
{
    int count = 0;
    for (auto it = m_services.constBegin(); it != m_services.constEnd(); ++it) {
        if (it->state == ServiceState::Loaded)
            ++count;
    }
    return count;
}

int ServiceManager::unloadedServiceCount() const
{
    int count = 0;
    for (auto it = m_services.constBegin(); it != m_services.constEnd(); ++it) {
        if (it->state == ServiceState::Unloaded)
            ++count;
    }
    return count;
}

QString ServiceManager::activeDocumentLoadStatusLabel() const
{
    if (m_activeServiceId.isEmpty())
        return QStringLiteral("N/A");

    const ServiceEntry *entry = entryFor(m_activeServiceId);
    return entry != nullptr ? entry->documentLoadStatus : QStringLiteral("N/A");
}

WebEngineConsoleLog *ServiceManager::webEngineConsoleLog() const
{
    return m_consoleLog;
}

void ServiceManager::reloadActivePage()
{
    if (m_activeServiceId.isEmpty())
        return;

    ServiceEntry *entry = entryFor(m_activeServiceId);
    if (entry == nullptr || entry->view == nullptr)
        return;

    if (QWebEngineProfile *profile = entry->page != nullptr ? entry->page->profile() : nullptr; profile != nullptr)
        UserAgentSettings::applyToProfile(profile);

    entry->documentLoadStatus = QStringLiteral("Loading");
    entry->view->reload();
}

QIcon ServiceManager::iconForService(const QString &serviceId) const
{
    const ServiceEntry *entry = entryFor(serviceId);
    if (entry == nullptr)
        return {};

    if (m_faviconProvider->hasCachedIcon(serviceId))
        return m_faviconProvider->cachedIcon(serviceId);

    return ServiceRegistry::faviconFor(entry->catalog);
}

bool ServiceManager::openService(const QString &catalogServiceId)
{
    if (!ServiceRegistry::isLaunchable(catalogServiceId))
        return false;

    if (hasOpenService(catalogServiceId)) {
        activateService(catalogServiceId);
        return true;
    }

    const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(catalogServiceId);
    if (catalog.id.isEmpty())
        return false;

    ServiceEntry entry;
    entry.catalog = catalog;
    entry.state = ServiceState::Unloaded;
    entry.lastUrl = catalog.defaultUrl;
    m_services.insert(catalogServiceId, std::move(entry));

    emit serviceAdded(catalogServiceId);
    emit serviceIconChanged(catalogServiceId, iconForService(catalogServiceId));

    activateService(catalogServiceId);
    return true;
}

void ServiceManager::activateService(const QString &serviceId)
{
    ServiceEntry *entry = entryFor(serviceId);
    if (entry == nullptr)
        return;

    if (entry->state == ServiceState::Unloaded)
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

    clearActiveServiceIfNeeded(serviceId);

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

    clearActiveServiceIfNeeded(serviceId);

    m_consoleLog->removeService(serviceId);
    m_services.remove(serviceId);
    emit serviceRemoved(serviceId);
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

    const QString profileId = entry.catalog.profileId.isEmpty()
        ? entry.catalog.id
        : entry.catalog.profileId;

    QWebEngineProfile *profile = ProfileManager::instance().profileFor(profileId);
    if (profile == nullptr)
        return;

    UserAgentSettings::applyToProfile(profile);

    const QString serviceId = entry.catalog.id;
    entry.page = new DiagnosticsWebEnginePage(profile, m_stack);
    entry.view = new QWebEngineView(m_stack);
    entry.view->setPage(entry.page);

    connect(entry.view, &QWebEngineView::urlChanged, this, [this, serviceId](const QUrl &url) {
        ServiceEntry *trackedEntry = entryFor(serviceId);
        if (trackedEntry != nullptr && url.isValid())
            trackedEntry->lastUrl = url;
    });

    connect(entry.view, &QWebEngineView::loadStarted, this, [this, serviceId]() {
        ServiceEntry *trackedEntry = entryFor(serviceId);
        if (trackedEntry != nullptr)
            trackedEntry->documentLoadStatus = QStringLiteral("Loading");
    });

    connect(entry.view, &QWebEngineView::loadFinished, this, [this, serviceId](bool ok) {
        ServiceEntry *trackedEntry = entryFor(serviceId);
        if (trackedEntry != nullptr)
            trackedEntry->documentLoadStatus = ok ? QStringLiteral("Loaded") : QStringLiteral("Failed");
    });

    m_consoleLog->bindPage(serviceId, entry.page);

    m_stack->addWidget(entry.view);
    m_faviconProvider->bindPage(serviceId, entry.page);

    const QUrl targetUrl = entry.lastUrl.isValid() ? entry.lastUrl : entry.catalog.defaultUrl;
    entry.view->load(targetUrl);
}

void ServiceManager::destroyView(ServiceEntry &entry)
{
    if (entry.view == nullptr)
        return;

    m_faviconProvider->unbindPage(entry.catalog.id);
    m_consoleLog->unbindPage(entry.catalog.id);

    m_stack->removeWidget(entry.view);
    entry.view->deleteLater();
    entry.page->deleteLater();
    entry.view = nullptr;
    entry.page = nullptr;
}

void ServiceManager::showEmptyWorkspace()
{
    m_stack->setCurrentWidget(m_emptyWorkspace);
}

void ServiceManager::clearActiveServiceIfNeeded(const QString &serviceId)
{
    if (m_activeServiceId != serviceId)
        return;

    m_activeServiceId.clear();
    showEmptyWorkspace();
    emit activeServiceChanged(QString());
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
        connect(entry.inactivityTimer, &QTimer::timeout, this, [this, serviceId = entry.catalog.id]() {
            if (m_activeServiceId == serviceId)
                return;
            if (state(serviceId) == ServiceState::Loaded)
                unloadService(serviceId);
        });
    } else {
        entry.inactivityTimer->setInterval(SettingsManager::instance().autoUnloadTimeoutMs());
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

QString ServiceManager::profileRuntimeStateLabel(const QString &catalogServiceId) const
{
    const ServiceEntry *entry = entryFor(catalogServiceId);
    if (entry == nullptr)
        return QStringLiteral("Unloaded");

    switch (entry->state) {
    case ServiceState::Loaded:
        return QStringLiteral("Loaded");
    case ServiceState::Unloaded:
        return QStringLiteral("Unloaded");
    case ServiceState::Closed:
        return QStringLiteral("Unloaded");
    }

    return QStringLiteral("Unloaded");
}

bool ServiceManager::clearServiceProfile(const QString &catalogServiceId, QString *errorMessage)
{
    const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(catalogServiceId);
    if (catalog.id.isEmpty()) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Unknown service.");
        return false;
    }

    const QString profileId = catalog.profileId.isEmpty() ? catalog.id : catalog.profileId;

    if (hasOpenService(catalogServiceId)) {
        ServiceEntry *entry = entryFor(catalogServiceId);
        if (entry == nullptr) {
            if (errorMessage != nullptr)
                *errorMessage = QStringLiteral("Service entry is unavailable.");
            return false;
        }

        if (entry->state == ServiceState::Loaded)
            unloadService(catalogServiceId);

        if (entry->view != nullptr)
            destroyView(*entry);
    }

    ProfileManager::instance().releaseProfile(profileId);

    if (!ProfileManager::instance().clearProfileData(profileId, errorMessage)) {
        CrashLogger::instance().logOperationalError(
            QStringLiteral("profile"),
            QStringLiteral("Failed to clear profile %1 for service %2: %3")
                .arg(profileId, catalogServiceId, errorMessage != nullptr ? *errorMessage : QString()));
        return false;
    }

    m_faviconProvider->clearCacheForService(catalogServiceId);
    return true;
}

void ServiceManager::signInToService(const QString &catalogServiceId)
{
    if (!ServiceRegistry::isLaunchable(catalogServiceId))
        return;

    const ServiceCatalogEntry catalog = ServiceRegistry::entryFor(catalogServiceId);
    if (catalog.id.isEmpty())
        return;

    if (!hasOpenService(catalogServiceId)) {
        openService(catalogServiceId);
        return;
    }

    ServiceEntry *entry = entryFor(catalogServiceId);
    if (entry == nullptr)
        return;

    entry->lastUrl = catalog.defaultUrl;
    if (entry->view != nullptr)
        entry->view->load(catalog.defaultUrl);

    activateService(catalogServiceId);
}
