#include "saw/logging/json_logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <cstdlib>

namespace {
QFile *logFile{};
QMutex logMutex;
constexpr qint64 MaximumLogBytes = 5 * 1024 * 1024;
constexpr int RetainedFiles = 5;

QString levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg: return QStringLiteral("debug");
    case QtInfoMsg: return QStringLiteral("info");
    case QtWarningMsg: return QStringLiteral("warning");
    case QtCriticalMsg: return QStringLiteral("critical");
    case QtFatalMsg: return QStringLiteral("fatal");
    }
    return QStringLiteral("unknown");
}

void rotate(const QString &path)
{
    QFile::remove(path + QStringLiteral(".%1").arg(RetainedFiles));
    for (int index = RetainedFiles - 1; index >= 1; --index)
        if (QFile::exists(path + QStringLiteral(".%1").arg(index)))
            QFile::rename(path + QStringLiteral(".%1").arg(index),
                          path + QStringLiteral(".%1").arg(index + 1));
    QFile::rename(path, path + QStringLiteral(".1"));
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker locker(&logMutex);
    if (logFile && logFile->isOpen()) {
        QJsonObject entry{{"timestamp", QDateTime::currentDateTime().toString(Qt::ISODateWithMs)},
                          {"level", levelName(type)},
                          {"category", QString::fromUtf8(context.category ? context.category : "default")},
                          {"message", message.left(2048)}};
        logFile->write(QJsonDocument(entry).toJson(QJsonDocument::Compact));
        logFile->write("\n");
        logFile->flush();
    }
    if (type == QtFatalMsg) abort();
}
}

namespace saw::logging {

bool JsonLogger::install(const QString &directory, QString *error)
{
    shutdown();
    if (!QDir().mkpath(directory)) {
        if (error) *error = QStringLiteral("无法创建日志目录");
        return false;
    }
    const QString path = QDir(directory).filePath(QStringLiteral("application.jsonl"));
    if (QFileInfo(path).size() >= MaximumLogBytes) rotate(path);
    logFile = new QFile(path);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (error) *error = logFile->errorString();
        delete logFile;
        logFile = nullptr;
        return false;
    }
    qInstallMessageHandler(messageHandler);
    return true;
}

void JsonLogger::shutdown()
{
    qInstallMessageHandler(nullptr);
    QMutexLocker locker(&logMutex);
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
}

} // namespace saw::logging
