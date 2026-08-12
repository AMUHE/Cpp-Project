#pragma once

#include <QObject>
#include <QStringList>
#include <atomic>

class QTextToSpeech;
class QThread;

namespace saw::speech {

struct SpeechOptions {
    bool enabled{true};
    QString locale{QStringLiteral("zh_CN")};
    double rate{0.0};
    double volume{1.0};
    int maximumQueueSize{8};
};

class SpeechAnnouncer final : public QObject {
    Q_OBJECT
public:
    explicit SpeechAnnouncer(QObject *parent = nullptr);
    ~SpeechAnnouncer() override;

    bool initialize(const SpeechOptions &options, QString *error = nullptr);
    bool isAvailable() const;
    void announce(const QString &message, bool interrupt = false);
    void stop();
    QString engineName() const { return engineName_; }

signals:
    void availabilityChanged(bool available);
    void announcementStarted(const QString &message);
    void errorOccurred(const QString &message);

private:
    void speakNext();
    QTextToSpeech *engine_{};
    QThread *workerThread_{};
    QObject *workerContext_{};
    QStringList queue_;
    QString engineName_;
    int maximumQueueSize_{8};
    std::atomic_bool enabled_{false};
    std::atomic_bool available_{false};
};

} // namespace saw::speech
