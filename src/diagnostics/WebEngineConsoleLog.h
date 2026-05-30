#pragma once

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QWebEnginePage;

struct WebEngineConsoleEntry
{
    QDateTime timestamp;
    int level = 0;
    QString message;
    int lineNumber = 0;
    QString sourceId;
};

class WebEngineConsoleLog : public QObject
{
    Q_OBJECT

public:
    static constexpr int kMaxMessagesPerService = 500;

    explicit WebEngineConsoleLog(QObject *parent = nullptr);

    void bindPage(const QString &serviceId, QWebEnginePage *page);
    void unbindPage(const QString &serviceId);
    void removeService(const QString &serviceId);

    void clear(const QString &serviceId);
    QList<WebEngineConsoleEntry> messages(const QString &serviceId) const;

signals:
    void messageLogged(const QString &serviceId);

private:
    void appendMessage(const QString &serviceId, const WebEngineConsoleEntry &entry);

    QHash<QString, QList<WebEngineConsoleEntry>> m_messages;
};
