#pragma once

#include <QHash>
#include <QIcon>
#include <QObject>
#include <QString>

class QWebEnginePage;

class ServiceFaviconProvider : public QObject
{
    Q_OBJECT

public:
    static constexpr int kLogicalIconSize = 40;

    explicit ServiceFaviconProvider(QObject *parent = nullptr);

    static QIcon placeholderIcon(const QString &displayName, int logicalSize = kLogicalIconSize);
    static QIcon normalizedIcon(const QIcon &icon, int logicalSize = kLogicalIconSize);

    void bindPage(const QString &serviceId, QWebEnginePage *page);
    void unbindPage(const QString &serviceId);

    QIcon cachedIcon(const QString &serviceId) const;
    bool hasCachedIcon(const QString &serviceId) const;

signals:
    void faviconChanged(const QString &serviceId, const QIcon &icon);

private:
    void applyIcon(const QString &serviceId, const QIcon &pageIcon);

    QHash<QString, QIcon> m_cache;
    QHash<QString, QWebEnginePage *> m_pages;
};
