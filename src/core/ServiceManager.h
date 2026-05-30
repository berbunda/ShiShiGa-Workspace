#pragma once

#include "services/ServiceRegistry.h"
#include "services/ServiceState.h"

#include <QDateTime>
#include <QHash>
#include <QIcon>
#include <QObject>
#include <QString>

class QStackedWidget;
class QTimer;
class QWebEnginePage;
class QWebEngineView;
class ServiceFaviconProvider;
class WebEngineConsoleLog;

class ServiceManager : public QObject
{
    Q_OBJECT

public:
    explicit ServiceManager(QStackedWidget *stackWidget, QObject *parent = nullptr);

    QList<ServiceCatalogEntry> openServices() const;
    bool hasOpenService(const QString &serviceId) const;
    ServiceState state(const QString &serviceId) const;
    QString activeServiceId() const;
    QWebEngineView *viewFor(const QString &serviceId) const;
    QWebEnginePage *pageFor(const QString &serviceId) const;
    QIcon iconForService(const QString &serviceId) const;

    int loadedServiceCount() const;
    int unloadedServiceCount() const;
    QString activeDocumentLoadStatusLabel() const;
    WebEngineConsoleLog *webEngineConsoleLog() const;
    void reloadActivePage();

    void refreshInactivityTimeouts();

    QString profileRuntimeStateLabel(const QString &catalogServiceId) const;
    bool clearServiceProfile(const QString &catalogServiceId, QString *errorMessage = nullptr);
    void signInToService(const QString &catalogServiceId);

public slots:
    bool openService(const QString &catalogServiceId);
    void activateService(const QString &serviceId);
    void unloadService(const QString &serviceId);
    void closeService(const QString &serviceId);

signals:
    void serviceAdded(const QString &serviceId);
    void serviceRemoved(const QString &serviceId);
    void serviceStateChanged(const QString &serviceId, ServiceState state);
    void activeServiceChanged(const QString &serviceId);
    void serviceIconChanged(const QString &serviceId, const QIcon &icon);

private:
    struct ServiceEntry
    {
        ServiceCatalogEntry catalog;
        ServiceState state = ServiceState::Closed;
        QWebEngineView *view = nullptr;
        QWebEnginePage *page = nullptr;
        QUrl lastUrl;
        QDateTime lastActiveTime;
        QTimer *inactivityTimer = nullptr;
        QString documentLoadStatus = QStringLiteral("Unknown");
    };

    ServiceEntry *entryFor(const QString &serviceId);
    const ServiceEntry *entryFor(const QString &serviceId) const;

    void createView(ServiceEntry &entry);
    void destroyView(ServiceEntry &entry);
    void showEmptyWorkspace();
    void updateInactivityTimers();
    void startInactivityTimer(ServiceEntry &entry);
    void stopInactivityTimer(ServiceEntry &entry);
    void clearActiveServiceIfNeeded(const QString &serviceId);

    QStackedWidget *m_stack = nullptr;
    QWidget *m_emptyWorkspace = nullptr;
    ServiceFaviconProvider *m_faviconProvider = nullptr;
    WebEngineConsoleLog *m_consoleLog = nullptr;
    QHash<QString, ServiceEntry> m_services;
    QString m_activeServiceId;
};
