#include "CrashLogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSysInfo>
#include <QTextStream>
#include <QThread>

#include <cstdlib>
#include <exception>
#include <iterator>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <dbghelp.h>
#endif

namespace {

#ifdef Q_OS_WIN
#if defined(_M_ARM64)
constexpr USHORT kStackWalkMachine = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_X64)
constexpr USHORT kStackWalkMachine = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_IX86)
constexpr USHORT kStackWalkMachine = IMAGE_FILE_MACHINE_I386;
#endif

QString formatWindowsPath(const wchar_t *path)
{
    return QDir::fromNativeSeparators(QString::fromWCharArray(path));
}

QString captureWindowsStackTrace()
{
    void *frames[64] = {};
    const USHORT frameCount = CaptureStackBackTrace(0, static_cast<ULONG>(std::size(frames)), frames, nullptr);
    if (frameCount == 0)
        return QStringLiteral("(stack trace unavailable)\n");

    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    if (SymInitialize(process, nullptr, TRUE) == FALSE)
        return QStringLiteral("(symbol engine initialization failed)\n");

    QString trace;
    QTextStream stream(&trace);
    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {};
    auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbolBuffer);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    for (USHORT index = 0; index < frameCount; ++index) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[index]);
        stream << "#" << index << " 0x"
               << QString::number(address, 16).toUpper().rightJustified(16, QLatin1Char('0'));

        DWORD64 displacement = 0;
        if (SymFromAddr(process, address, &displacement, symbol) == TRUE) {
            stream << " " << symbol->Name;
            if (displacement != 0)
                stream << "+0x" << QString::number(displacement, 16);
        }

        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddr64(process, address, &lineDisplacement, &line) == TRUE)
            stream << " (" << QString::fromLocal8Bit(line.FileName) << ":" << line.LineNumber << ")";

        HMODULE moduleHandle = nullptr;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                   | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(frames[index]),
                               &moduleHandle) != 0) {
            wchar_t modulePath[MAX_PATH] = {};
            if (GetModuleFileNameW(moduleHandle, modulePath, MAX_PATH) != 0)
                stream << " [" << QFileInfo(formatWindowsPath(modulePath)).fileName() << "]";
        }

        stream << "\n";
    }

    SymCleanup(process);
    return trace;
}

QString captureWindowsExceptionStackTrace(void *exceptionPointers)
{
#ifdef _M_ARM64
    Q_UNUSED(exceptionPointers);
    return captureWindowsStackTrace();
#else
    if (exceptionPointers == nullptr)
        return captureWindowsStackTrace();

    auto *pointers = static_cast<EXCEPTION_POINTERS *>(exceptionPointers);
    if (pointers->ContextRecord == nullptr)
        return captureWindowsStackTrace();

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    if (SymInitialize(process, nullptr, TRUE) == FALSE)
        return QStringLiteral("(symbol engine initialization failed)\n");

    STACKFRAME64 frame = {};
#if defined(_M_X64)
    frame.AddrPC.Offset = pointers->ContextRecord->Rip;
    frame.AddrFrame.Offset = pointers->ContextRecord->Rbp;
    frame.AddrStack.Offset = pointers->ContextRecord->Rsp;
#elif defined(_M_IX86)
    frame.AddrPC.Offset = pointers->ContextRecord->Eip;
    frame.AddrFrame.Offset = pointers->ContextRecord->Ebp;
    frame.AddrStack.Offset = pointers->ContextRecord->Esp;
#endif
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    QString trace;
    QTextStream stream(&trace);

    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {};
    auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbolBuffer);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    for (int index = 0; index < 64; ++index) {
        if (StackWalk64(kStackWalkMachine,
                        process,
                        thread,
                        &frame,
                        pointers->ContextRecord,
                        nullptr,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        nullptr) == FALSE) {
            break;
        }

        if (frame.AddrPC.Offset == 0)
            break;

        stream << "#" << index << " 0x"
               << QString::number(frame.AddrPC.Offset, 16).toUpper().rightJustified(16, QLatin1Char('0'));

        DWORD64 displacement = 0;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol) == TRUE) {
            stream << " " << symbol->Name;
            if (displacement != 0)
                stream << "+0x" << QString::number(displacement, 16);
        }

        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;
        if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &lineDisplacement, &line) == TRUE)
            stream << " (" << QString::fromLocal8Bit(line.FileName) << ":" << line.LineNumber << ")";

        stream << "\n";
    }

    SymCleanup(process);
    return trace;
#endif
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS *pointers)
{
    CrashLogger::instance().logStructuredException(pointers);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

CrashLogger *g_loggerInstance = nullptr;

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (g_loggerInstance != nullptr)
        g_loggerInstance->logQtMessage(type, context, message);
}

void terminateHandler()
{
    if (g_loggerInstance != nullptr)
        g_loggerInstance->logTerminate();
    std::abort();
}

} // namespace

class CrashLogger::RecursionGuard
{
public:
    explicit RecursionGuard(std::atomic<bool> &flag)
        : m_flag(flag)
        , m_active(false)
    {
        bool expected = false;
        m_active = m_flag.compare_exchange_strong(expected, true);
    }

    ~RecursionGuard()
    {
        if (m_active)
            m_flag.store(false);
    }

    [[nodiscard]] bool active() const { return m_active; }

private:
    std::atomic<bool> &m_flag;
    bool m_active;
};

CrashLogger &CrashLogger::instance()
{
    static CrashLogger logger;
    g_loggerInstance = &logger;
    return logger;
}

void CrashLogger::install(const QString &applicationVersion)
{
    if (m_installed.exchange(true))
        return;

    m_applicationVersion = applicationVersion;
    ensureCrashesDirectory();

    qInstallMessageHandler(qtMessageHandler);
    std::set_terminate(terminateHandler);

#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
#endif
}

void CrashLogger::logQtMessage(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message)
{
    RecursionGuard guard(m_inHandler);
    if (!guard.active())
        return;

    const QDateTime timestamp = QDateTime::currentDateTime();
    const QString line = QStringLiteral("[%1] [%2] [thread %3] %4\n")
                             .arg(timestamp.toString(Qt::ISODateWithMs),
                                  qFormatLogMessage(type, context, message),
                                  currentThreadIdString(),
                                  formatQtContext(context));

    appendToFile(runtimeLogPath(), line);

    if (type == QtFatalMsg)
        writeCrashReport(ErrorKind::QtFatal, message, &context);

    const QtMessageHandler defaultHandler = qInstallMessageHandler(nullptr);
    if (defaultHandler != nullptr)
        defaultHandler(type, context, message);
    qInstallMessageHandler(qtMessageHandler);
}

void CrashLogger::logException(const std::exception &exception)
{
    writeCrashReport(ErrorKind::StdException, QString::fromUtf8(exception.what()));
}

void CrashLogger::logUnknownException()
{
    writeCrashReport(ErrorKind::UnknownException, QStringLiteral("Unknown C++ exception"));
}

void CrashLogger::logTerminate()
{
    writeCrashReport(ErrorKind::Terminate, QStringLiteral("std::terminate was called"));
}

void CrashLogger::logQtFatal(const QMessageLogContext &context, const QString &message)
{
    writeCrashReport(ErrorKind::QtFatal, message, &context);
}

#ifdef Q_OS_WIN
void CrashLogger::logStructuredException(void *exceptionPointers)
{
    auto *pointers = static_cast<EXCEPTION_POINTERS *>(exceptionPointers);
    QString message = QStringLiteral("Structured exception");

    if (pointers != nullptr && pointers->ExceptionRecord != nullptr) {
        message = QStringLiteral("Structured exception code: 0x%1 at 0x%2")
                      .arg(QString::number(pointers->ExceptionRecord->ExceptionCode, 16),
                           QString::number(reinterpret_cast<quintptr>(
                                               pointers->ExceptionRecord->ExceptionAddress),
                                           16));
    }

    RecursionGuard guard(m_inHandler);
    if (!guard.active())
        return;

    const QString report = buildCrashReport(ErrorKind::StructuredException, message, nullptr);
    const QString stackTrace = captureWindowsExceptionStackTrace(exceptionPointers);
    const QString fullReport = report + QStringLiteral("\n--- Stack Trace ---\n") + stackTrace;

    appendToFile(createCrashLogPath(), fullReport);
}
#endif

QString CrashLogger::applicationDirectory() const
{
    if (QCoreApplication::instance() != nullptr)
        return QCoreApplication::applicationDirPath();

#ifdef Q_OS_WIN
    wchar_t pathBuffer[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return QDir::currentPath();

    QString path = formatWindowsPath(pathBuffer);
    return QFileInfo(path).absolutePath();
#else
    return QDir::currentPath();
#endif
}

QString CrashLogger::crashesDirectory() const
{
    return QDir(applicationDirectory()).filePath(QStringLiteral("crashes"));
}

QString CrashLogger::ensureCrashesDirectory() const
{
    const QString directory = crashesDirectory();
    QDir().mkpath(directory);
    return directory;
}

QString CrashLogger::runtimeLogPath() const
{
    const QString date = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd"));
    return QDir(ensureCrashesDirectory()).filePath(QStringLiteral("runtime_%1.log").arg(date));
}

QString CrashLogger::createCrashLogPath() const
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    return QDir(ensureCrashesDirectory()).filePath(QStringLiteral("crash_%1.log").arg(timestamp));
}

QString CrashLogger::applicationVersion() const
{
    if (!m_applicationVersion.isEmpty())
        return m_applicationVersion;
#ifdef SHISHIGA_APP_VERSION
    return QStringLiteral(SHISHIGA_APP_VERSION);
#else
    return QStringLiteral("unknown");
#endif
}

QString CrashLogger::operatingSystemDescription() const
{
    return QStringLiteral("%1 (%2 %3)")
        .arg(QSysInfo::prettyProductName(),
             QSysInfo::kernelType(),
             QSysInfo::kernelVersion());
}

QString CrashLogger::architectureDescription() const
{
#if defined(Q_PROCESSOR_ARM_64)
    return QStringLiteral("arm64");
#elif defined(Q_PROCESSOR_X86_64)
    return QStringLiteral("x86_64");
#elif defined(Q_PROCESSOR_X86_32)
    return QStringLiteral("x86");
#else
    return QSysInfo::currentCpuArchitecture();
#endif
}

QString CrashLogger::qtVersionDescription() const
{
    return QStringLiteral("%1 (%2)").arg(qVersion(), QT_VERSION_STR);
}

QString CrashLogger::currentThreadIdString() const
{
    return QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
}

QString CrashLogger::errorKindLabel(const ErrorKind kind) const
{
    switch (kind) {
    case ErrorKind::StdException:
        return QStringLiteral("std::exception");
    case ErrorKind::UnknownException:
        return QStringLiteral("unknown exception");
    case ErrorKind::Terminate:
        return QStringLiteral("std::terminate");
    case ErrorKind::QtFatal:
        return QStringLiteral("Qt fatal");
    case ErrorKind::QtCritical:
        return QStringLiteral("Qt critical");
    case ErrorKind::QtWarning:
        return QStringLiteral("Qt warning");
    case ErrorKind::QtDebug:
        return QStringLiteral("Qt debug");
    case ErrorKind::StructuredException:
        return QStringLiteral("structured exception");
    }
    return QStringLiteral("unknown");
}

QString CrashLogger::formatQtContext(const QMessageLogContext &context) const
{
    const QString file = context.file != nullptr ? QString::fromUtf8(context.file) : QStringLiteral("unknown");
    const QString function = context.function != nullptr ? QString::fromUtf8(context.function)
                                                           : QStringLiteral("unknown");

    return QStringLiteral("file=%1 line=%2 function=%3 category=%4")
        .arg(file)
        .arg(context.line)
        .arg(function)
        .arg(context.category != nullptr ? QString::fromUtf8(context.category) : QStringLiteral("default"));
}

QString CrashLogger::captureStackTrace()
{
#ifdef Q_OS_WIN
    return captureWindowsStackTrace();
#else
    return QStringLiteral("(stack trace not implemented for this platform)\n");
#endif
}

QString CrashLogger::buildCrashReport(const ErrorKind kind,
                                      const QString &message,
                                      const QMessageLogContext *context) const
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    QString report;
    QTextStream stream(&report);

    stream << "=== ShiShiga Workspace Crash Log ===\n";
    stream << "Timestamp: " << timestamp.toString(Qt::ISODateWithMs) << "\n";
    stream << "Application Version: " << applicationVersion() << "\n";
    stream << "OS Version: " << operatingSystemDescription() << "\n";
    stream << "Architecture: " << architectureDescription() << "\n";
    stream << "Qt Version: " << qtVersionDescription() << "\n";
    stream << "Thread ID: " << currentThreadIdString() << "\n";
    stream << "Error Type: " << errorKindLabel(kind) << "\n";
    stream << "Exception Message: " << message << "\n";

    if (context != nullptr) {
        stream << "\n--- Qt Context ---\n";
        stream << formatQtContext(*context) << "\n";
    }

    return report;
}

bool CrashLogger::appendToFile(const QString &filePath, const QString &content)
{
    std::scoped_lock lock(m_mutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    stream << content;
    return true;
}

void CrashLogger::writeCrashReport(const ErrorKind kind,
                                     const QString &message,
                                     const QMessageLogContext *context)
{
    RecursionGuard guard(m_inHandler);
    if (!guard.active())
        return;

    QString report = buildCrashReport(kind, message, context);
    report += QStringLiteral("\n--- Stack Trace ---\n");
    report += captureStackTrace();

    appendToFile(createCrashLogPath(), report);
}
