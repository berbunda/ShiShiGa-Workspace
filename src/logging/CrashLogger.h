#pragma once

#include <QMessageLogContext>
#include <QString>
#include <QtGlobal>

#include <atomic>
#include <exception>
#include <mutex>

class CrashLogger
{
public:
    static CrashLogger &instance();

    void install(const QString &applicationVersion = QString());

    void logQtMessage(QtMsgType type,
                      const QMessageLogContext &context,
                      const QString &message);

    void logException(const std::exception &exception);
    void logUnknownException();
    void logTerminate();
    void logQtFatal(const QMessageLogContext &context, const QString &message);
    void logOperationalError(const QString &category, const QString &message);

    QString installedVersion() const;
    QString applicationDirectoryPath() const;
    QString crashesDirectoryPath() const;

#ifdef Q_OS_WIN
    void logStructuredException(void *exceptionPointers);
#endif

private:
    CrashLogger() = default;

    enum class ErrorKind {
        StdException,
        UnknownException,
        Terminate,
        QtFatal,
        QtCritical,
        QtWarning,
        QtDebug,
        StructuredException,
    };

    class RecursionGuard;

    QString applicationDirectory() const;
    QString crashesDirectory() const;
    QString ensureCrashesDirectory() const;
    QString runtimeLogPath() const;
    QString createCrashLogPath() const;

    QString applicationVersion() const;
    QString operatingSystemDescription() const;
    QString architectureDescription() const;
    QString qtVersionDescription() const;
    QString currentThreadIdString() const;
    QString errorKindLabel(ErrorKind kind) const;

    QString formatQtContext(const QMessageLogContext &context) const;
    QString captureStackTrace();
    QString buildCrashReport(ErrorKind kind,
                             const QString &message,
                             const QMessageLogContext *context = nullptr) const;

    bool appendToFile(const QString &filePath, const QString &content);
    void writeCrashReport(ErrorKind kind,
                          const QString &message,
                          const QMessageLogContext *context = nullptr);

    QString m_applicationVersion;
    std::mutex m_mutex;
    std::atomic<bool> m_inHandler { false };
    std::atomic<bool> m_installed { false };
};
