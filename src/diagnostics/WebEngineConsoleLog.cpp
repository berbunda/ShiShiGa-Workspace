#include "WebEngineConsoleLog.h"

#include "DiagnosticsWebEnginePage.h"

#include <QWebEnginePage>

WebEngineConsoleLog::WebEngineConsoleLog(QObject *parent)
    : QObject(parent)
{
}

void WebEngineConsoleLog::bindPage(const QString &serviceId, QWebEnginePage *page)
{
    if (serviceId.isEmpty() || page == nullptr)
        return;

    auto *diagnosticsPage = qobject_cast<DiagnosticsWebEnginePage *>(page);
    if (diagnosticsPage == nullptr)
        return;

    connect(diagnosticsPage, &DiagnosticsWebEnginePage::consoleMessageLogged, this,
            [this, serviceId](int level, const QString &message, int lineNumber, const QString &sourceId) {
                WebEngineConsoleEntry entry;
                entry.timestamp = QDateTime::currentDateTime();
                entry.level = level;
                entry.message = message;
                entry.lineNumber = lineNumber;
                entry.sourceId = sourceId;
                appendMessage(serviceId, entry);
            });
}

void WebEngineConsoleLog::unbindPage(const QString &serviceId)
{
    Q_UNUSED(serviceId);
}

void WebEngineConsoleLog::removeService(const QString &serviceId)
{
    m_messages.remove(serviceId);
}

void WebEngineConsoleLog::clear(const QString &serviceId)
{
    m_messages.remove(serviceId);
}

QList<WebEngineConsoleEntry> WebEngineConsoleLog::messages(const QString &serviceId) const
{
    return m_messages.value(serviceId);
}

void WebEngineConsoleLog::appendMessage(const QString &serviceId, const WebEngineConsoleEntry &entry)
{
    QList<WebEngineConsoleEntry> &entries = m_messages[serviceId];
    entries.append(entry);
    while (entries.size() > kMaxMessagesPerService)
        entries.removeFirst();

    emit messageLogged(serviceId);
}
