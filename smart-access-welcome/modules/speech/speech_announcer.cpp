#include "saw/speech/speech_announcer.h"

#include <QLocale>
#include <QMetaObject>
#include <QCoreApplication>
#include <QTextToSpeech>
#include <QThread>

namespace saw::speech {

SpeechAnnouncer::SpeechAnnouncer(QObject *parent) : QObject(parent) {}

SpeechAnnouncer::~SpeechAnnouncer() { stop(); }

bool SpeechAnnouncer::initialize(const SpeechOptions &options, QString *error)
{
    stop();
    enabled_ = options.enabled;
    available_ = false;
    maximumQueueSize_ = qBound(1, options.maximumQueueSize, 100);
    if (!enabled_) return true;

    Q_UNUSED(error)
    workerThread_ = new QThread(this);
    workerContext_ = new QObject;
    workerContext_->moveToThread(workerThread_);
    connect(workerThread_, &QThread::started, workerContext_, [this, options] {
        const QStringList engines = QTextToSpeech::availableEngines();
        if (engines.isEmpty()) {
            enabled_ = false;
            QMetaObject::invokeMethod(this, [this] {
                emit availabilityChanged(false);
                emit errorOccurred(QStringLiteral("系统未安装可用的语音合成引擎"));
            }, Qt::QueuedConnection);
            return;
        }
        engine_ = new QTextToSpeech(engines.first());
        engine_->setRate(qBound(-1.0, options.rate, 1.0));
        engine_->setVolume(qBound(0.0, options.volume, 1.0));
        const QLocale requested(options.locale);
        const auto locales = engine_->availableLocales();
        if (locales.contains(requested)) engine_->setLocale(requested);
        else for (const QLocale &locale : locales) {
            if (locale.language() == requested.language()) {
                engine_->setLocale(locale);
                break;
            }
        }
        connect(engine_, &QTextToSpeech::stateChanged, workerContext_,
                [this](QTextToSpeech::State state) {
            if (state == QTextToSpeech::Ready) speakNext();
            else if (state == QTextToSpeech::BackendError) {
                available_ = false;
                queue_.clear();
                QMetaObject::invokeMethod(this, [this] {
                    emit availabilityChanged(false);
                    emit errorOccurred(QStringLiteral("语音合成引擎发生错误"));
                }, Qt::QueuedConnection);
            }
        });
        available_ = true;
        QMetaObject::invokeMethod(this, [this, name = engines.first()] {
            engineName_ = name;
            emit availabilityChanged(true);
        }, Qt::QueuedConnection);
        speakNext();
    });
    workerThread_->start();
    return true;
}

bool SpeechAnnouncer::isAvailable() const
{
    return enabled_ && available_;
}

void SpeechAnnouncer::announce(const QString &message, bool interrupt)
{
    const QString normalized = message.simplified().left(256);
    if (!enabled_ || normalized.isEmpty() || !workerContext_) return;
    QMetaObject::invokeMethod(workerContext_, [this, normalized, interrupt] {
        if (interrupt) {
            queue_.clear();
            if (engine_) engine_->stop();
        }
        if (queue_.size() >= maximumQueueSize_) queue_.removeFirst();
        queue_.append(normalized);
        speakNext();
    }, Qt::QueuedConnection);
}

void SpeechAnnouncer::speakNext()
{
    if (!engine_ || engine_->state() != QTextToSpeech::Ready || queue_.isEmpty()) return;
    const QString message = queue_.takeFirst();
    emit announcementStarted(message);
    engine_->say(message);
}

void SpeechAnnouncer::stop()
{
    available_ = false;
    if (!workerThread_) {
        queue_.clear();
        return;
    }
    if (workerThread_->isRunning()) {
        QMetaObject::invokeMethod(workerContext_, [this] {
            queue_.clear();
            if (engine_) {
                engine_->stop();
                delete engine_;
                engine_ = nullptr;
            }
            workerContext_->moveToThread(QCoreApplication::instance()->thread());
        }, Qt::BlockingQueuedConnection);
        workerThread_->quit();
        workerThread_->wait(5000);
    }
    delete workerContext_;
    workerContext_ = nullptr;
    delete workerThread_;
    workerThread_ = nullptr;
}

} // namespace saw::speech
