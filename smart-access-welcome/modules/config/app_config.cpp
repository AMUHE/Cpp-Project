#include "saw/config/app_config.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
template<typename T>
T bounded(const QJsonObject &object, const char *key, T fallback, T minimum, T maximum)
{
    const QJsonValue value = object.value(QLatin1String(key));
    if (!value.isDouble()) return fallback;
    const T converted = static_cast<T>(value.toDouble());
    return converted >= minimum && converted <= maximum ? converted : fallback;
}

QString text(const QJsonObject &object, const char *key, const QString &fallback)
{
    const QString value = object.value(QLatin1String(key)).toString().trimmed();
    return value.isEmpty() ? fallback : value;
}
}

namespace saw::config {

bool ConfigLoader::load(const QString &path, const QString &dataDirectory,
                        AppConfig &config, QString *error)
{
    config.databaseFile = QDir(dataDirectory).filePath(QStringLiteral("access.db"));
    if (path.isEmpty() || !QFile::exists(path)) return true;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("无法读取配置文件：%1").arg(file.errorString());
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (document.isNull() || !document.isObject()) {
        if (error) *error = QStringLiteral("配置文件不是有效 JSON：%1").arg(parseError.errorString());
        return false;
    }
    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("schemaVersion")).toInt(1) != 1) {
        if (error) *error = QStringLiteral("不支持的配置 schemaVersion");
        return false;
    }

    const QJsonObject device = root.value(QStringLiteral("device")).toObject();
    config.deviceId = text(device, "id", config.deviceId);
    config.deviceName = text(device, "displayName", config.deviceName);

    const QJsonObject camera = root.value(QStringLiteral("camera")).toObject();
    config.cameraMode = text(camera, "mode", config.cameraMode);
    config.primaryCameraIndex = bounded(camera, "primaryIndex", 0, 0, 32);
    config.secondaryCameraIndex = bounded(camera, "secondaryIndex", 1, 0, 32);
    config.frameWidth = bounded(camera, "frameWidth", 1280, 320, 7680);
    config.frameHeight = bounded(camera, "frameHeight", 720, 240, 4320);
    config.captureIntervalMs = bounded(camera, "captureIntervalMs", 30, 10, 1000);

    const QJsonObject recognition = root.value(QStringLiteral("recognition")).toObject();
    config.confidenceThreshold = bounded(recognition, "confidenceThreshold", 80.0, 1.0, 255.0);
    config.requiredConsecutiveMatches = bounded(recognition, "requiredConsecutiveMatches", 3, 1, 100);
    config.eventCooldownSeconds = bounded(recognition, "eventCooldownSeconds", 10, 0, 3600);

    const QJsonObject server = root.value(QStringLiteral("server")).toObject();
    const QHostAddress requested(text(server, "bindAddress", QStringLiteral("127.0.0.1")));
    if (requested.isNull()) {
        if (error) *error = QStringLiteral("server.bindAddress 无效");
        return false;
    }
    config.bindAddress = requested;
    config.httpPort = static_cast<quint16>(bounded(server, "httpPort", 8080, 1, 65535));

    const QJsonObject welcome = root.value(QStringLiteral("welcome")).toObject();
    config.defaultText = text(welcome, "defaultText", config.defaultText);
    config.grantedTemplate = text(welcome, "grantedTemplate", config.grantedTemplate);
    config.displayDurationSeconds = bounded(welcome, "displayDurationSeconds", 5, 1, 300);

    const QJsonObject storage = root.value(QStringLiteral("storage")).toObject();
    config.retentionDays = bounded(storage, "retentionDays", 90, 1, 3650);
    const QString database = storage.value(QStringLiteral("databaseFile")).toString().trimmed();
    if (!database.isEmpty())
        config.databaseFile = QDir::isAbsolutePath(database) ? database : QDir(dataDirectory).filePath(database);

    const QJsonObject speech = root.value(QStringLiteral("speech")).toObject();
    config.speechEnabled = speech.value(QStringLiteral("enabled")).toBool(true);
    config.speechRate = bounded(speech, "rate", 0.0, -1.0, 1.0);
    config.speechVolume = bounded(speech, "volume", 1.0, 0.0, 1.0);
    config.speechLocale = text(speech, "locale", config.speechLocale);
    config.grantedSpeech = text(speech, "grantedTemplate", config.grantedSpeech);
    config.deniedSpeech = text(speech, "deniedText", config.deniedSpeech);
    return true;
}

} // namespace saw::config
