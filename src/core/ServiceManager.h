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

class ServiceManager : public QObject
{
    Q_OBJECT

public:
    explicit ServiceManager(QStackedWidget *stackWidget, QObject *parent = nullptr);

    QList<ServiceDefinition> services() const;
    ServiceState state(const QString &serviceId) const;
    QString activeServiceId() const;
    QWebEngineView *viewFor(const QString &serviceId) const;
    QIcon iconForService(const QString &serviceId) const;

    void refreshInactivityTimeouts();

public slots:
    void activateService(const QString &serviceId);
    void unloadService(const QString &serviceId);
    void closeService(const QString &serviceId);

signals:
    void serviceStateChanged(const QString &serviceId, ServiceState state);
    void activeServiceChanged(const QString &serviceId);
    void serviceIconChanged(const QString &serviceId, const QIcon &icon);

private:
    struct ServiceEntry
    {
        ServiceDefinition definition;
        ServiceState state = ServiceState::Closed;
        QWebEngineView *view = nullptr;
        QWebEnginePage *page = nullptr;
        QUrl lastUrl;
        QDateTime lastActiveTime;
        QTimer *inactivityTimer = nullptr;
    };

    ServiceEntry *entryFor(const QString &serviceId);
    const ServiceEntry *entryFor(const QString &serviceId) const;

    void createView(ServiceEntry &entry);
    void destroyView(ServiceEntry &entry);
    void showPlaceholder();
    void updateInactivityTimers();
    void startInactivityTimer(ServiceEntry &entry);
    void stopInactivityTimer(ServiceEntry &entry);

    QStackedWidget *m_stack = nullptr;
    QWidget *m_placeholder = nullptr;
    ServiceFaviconProvider *m_faviconProvider = nullptr;
    QHash<QString, ServiceEntry> m_services;
    QString m_activeServiceId;
};
