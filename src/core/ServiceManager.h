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

class ServiceManager : public QObject
{
    Q_OBJECT

public:
    static constexpr int kInactivityTimeoutMs = 30 * 60 * 1000;

    explicit ServiceManager(QStackedWidget *stackWidget, QObject *parent = nullptr);

    QList<ServiceDefinition> services() const;
    ServiceState state(const QString &serviceId) const;
    QString activeServiceId() const;
    QWebEngineView *viewFor(const QString &serviceId) const;

public slots:
    void activateService(const QString &serviceId);
    void unloadService(const QString &serviceId);
    void closeService(const QString &serviceId);
    void setIconForService(const QString &serviceId, const QIcon &icon);

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
        QIcon icon;
    };

    ServiceEntry *entryFor(const QString &serviceId);
    const ServiceEntry *entryFor(const QString &serviceId) const;

    void createView(ServiceEntry &entry);
    void destroyView(ServiceEntry &entry);
    void showPlaceholder();
    void updateInactivityTimers();
    void startInactivityTimer(ServiceEntry &entry);
    void stopInactivityTimer(ServiceEntry &entry);
    void requestFavicon(ServiceEntry &entry);

    QStackedWidget *m_stack = nullptr;
    QWidget *m_placeholder = nullptr;
    QHash<QString, ServiceEntry> m_services;
    QString m_activeServiceId;
};
