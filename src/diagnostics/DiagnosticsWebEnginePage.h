#pragma once

#include <QWebEnginePage>

class DiagnosticsWebEnginePage : public QWebEnginePage
{
    Q_OBJECT

public:
    using QWebEnginePage::QWebEnginePage;

signals:
    void consoleMessageLogged(int level,
                              const QString &message,
                              int lineNumber,
                              const QString &sourceId);

protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString &message,
                                  int lineNumber,
                                  const QString &sourceId) override;
};
