#pragma once

#include <QHostAddress>
#include <QString>

namespace saw::config {

struct AppConfig {
    int schemaVersion{1};
    QString deviceId{QStringLiteral("terminal-demo-01")};
    QString deviceName{QStringLiteral("智能门禁终端")};
    QString cameraMode{QStringLiteral("single_device")};
    int primaryCameraIndex{0};
    int secondaryCameraIndex{1};
    int frameWidth{1280};
    int frameHeight{720};
    int captureIntervalMs{30};
    double confidenceThreshold{80.0};
    double minimumRecognitionAccuracy{20.0};
    int requiredConsecutiveMatches{3};
    int eventCooldownSeconds{10};
    QHostAddress bindAddress{QHostAddress::LocalHost};
    quint16 httpPort{8080};
    QString defaultText{QStringLiteral("请面向摄像头")};
    QString grantedTemplate{QStringLiteral("欢迎，{displayName}")};
    int displayDurationSeconds{5};
    QString databaseFile;
    int retentionDays{90};
    bool passwordAccessEnabled{true};
    QString passwordAccessUsername{QStringLiteral("admin")};
    QString passwordAccessPassword{QStringLiteral("123456")};
    bool speechEnabled{true};
    double speechRate{0.0};
    double speechVolume{1.0};
    QString speechLocale{QStringLiteral("zh_CN")};
    QString grantedSpeech{QStringLiteral("欢迎，{displayName}")};
    QString deniedSpeech{QStringLiteral("未识别，请联系管理员")};
};

class ConfigLoader {
public:
    static bool load(const QString &path, const QString &dataDirectory,
                     AppConfig &config, QString *error = nullptr);
};

} // namespace saw::config
