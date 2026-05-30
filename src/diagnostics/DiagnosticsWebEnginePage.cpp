#include "DiagnosticsWebEnginePage.h"

void DiagnosticsWebEnginePage::javaScriptConsoleMessage(const JavaScriptConsoleMessageLevel level,
                                                        const QString &message,
                                                        const int lineNumber,
                                                        const QString &sourceId)
{
    emit consoleMessageLogged(static_cast<int>(level), message, lineNumber, sourceId);
}
