#include "main_window.h"
#include "ui_main_window.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QJsonObject>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QProgressBar>
#include <QPixmap>
#include <QSpinBox>
#include <QStandardPaths>
#include <QSaveFile>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QUuid>
#include <QUrl>
#include <QWidget>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace {
QImage toImage(const cv::Mat &bgr)
{
    if (bgr.empty()) return {};
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888).copy();
}

QString safeDirectoryName(QString name)
{
    name = name.trimmed();
    for (const QChar character : QStringLiteral("<>:\"/\\|?*"))
        name.replace(character, QLatin1Char('_'));
    while (name.endsWith(QLatin1Char('.')) || name.endsWith(QLatin1Char(' ')))
        name.chop(1);
    return name.left(64);
}

bool saveJpeg(const QString &path, const cv::Mat &image, QString *error)
{
    std::vector<uchar> encoded;
    if (!cv::imencode(".jpg", image, encoded)) {
        if (error) *error = QStringLiteral("人脸样本编码失败");
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(reinterpret_cast<const char *>(encoded.data()),
                   static_cast<qint64>(encoded.size())) != static_cast<qint64>(encoded.size()) ||
        !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}

bool copyDirectoryIfMissing(const QString &sourcePath, const QString &targetPath)
{
    QDir source(sourcePath);
    if (!source.exists()) return true;
    if (!QDir().mkpath(targetPath)) return false;
    for (const QFileInfo &entry : source.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        const QString target = QDir(targetPath).filePath(entry.fileName());
        if (entry.isDir()) {
            if (!copyDirectoryIfMissing(entry.absoluteFilePath(), target)) return false;
        } else if (!QFileInfo::exists(target) && !QFile::copy(entry.absoluteFilePath(), target)) {
            return false;
        }
    }
    return true;
}

QString stablePersonId(const QString &displayName)
{
    static const QUuid identityNamespace(QStringLiteral("{6d2e58a1-70a7-4eb0-a33e-5dc63b9fb870}"));
    return QUuid::createUuidV5(identityNamespace, displayName.toUtf8())
        .toString(QUuid::WithoutBraces);
}

void setStatusPill(QLabel *label, const QString &text, const char *status)
{
    label->setText(text);
    label->setProperty("status", QLatin1String(status));
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->update();
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    video_ = ui_->videoLabel;
    mode_ = ui_->modeComboBox;
    primaryIndex_ = ui_->primarySpinBox;
    secondaryIndex_ = ui_->secondarySpinBox;
    openButton_ = ui_->openButton;
    closeButton_ = ui_->closeButton;
    enrollButton_ = ui_->enrollButton;
    recognizeButton_ = ui_->recognizeButton;
    passwordAccessButton_ = ui_->passwordAccessButton;
    enrollmentProgress_ = ui_->enrollmentProgressBar;
    ui_->headerClockLabel->setText(QDateTime::currentDateTime().toString(
        QStringLiteral("yyyy-MM-dd  HH:mm:ss")));
    auto *clockTimer = new QTimer(this);
    clockTimer->setInterval(1000);
    connect(clockTimer, &QTimer::timeout, this, [this] {
        ui_->headerClockLabel->setText(QDateTime::currentDateTime().toString(
            QStringLiteral("yyyy-MM-dd  HH:mm:ss")));
    });
    clockTimer->start();

    dataDirectory_ = qEnvironmentVariable("SAW_DATA_DIR").trimmed();
    if (dataDirectory_.isEmpty())
        dataDirectory_ = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    dataDirectory_ = QDir(dataDirectory_).absolutePath();
    enrollmentDirectory_ = QDir(dataDirectory_).filePath(QStringLiteral("biometrics"));
    QDir().mkpath(QDir(enrollmentDirectory_).filePath(QStringLiteral("faces")));
    // Non-destructive migration from the layout used by earlier releases.
    copyDirectoryIfMissing(QDir(dataDirectory_).filePath(QStringLiteral("faces")),
                           QDir(enrollmentDirectory_).filePath(QStringLiteral("faces")));
    const QString previousEnrollmentDirectory =
        QDir(dataDirectory_).filePath(QStringLiteral("database/enrollment"));
    copyDirectoryIfMissing(QDir(previousEnrollmentDirectory).filePath(QStringLiteral("faces")),
                           QDir(enrollmentDirectory_).filePath(QStringLiteral("faces")));
    for (const QString &name : {QStringLiteral("model.yml"), QStringLiteral("labels.csv")}) {
        const QString oldPath = QDir(dataDirectory_).filePath(name);
        const QString newPath = QDir(enrollmentDirectory_).filePath(name);
        if (QFileInfo::exists(oldPath) && !QFileInfo::exists(newPath)) QFile::copy(oldPath, newPath);
        const QString previousPath = QDir(previousEnrollmentDirectory).filePath(name);
        if (QFileInfo::exists(previousPath) && !QFileInfo::exists(newPath))
            QFile::copy(previousPath, newPath);
    }
    QString error;
    const QString configPath = findConfig();
    if (!saw::config::ConfigLoader::load(configPath, dataDirectory_, config_, &error))
        QMessageBox::warning(this, QStringLiteral("配置错误"), error);
    accessPolicy_ = saw::access::AccessPolicy({config_.requiredConsecutiveMatches,
        static_cast<std::int64_t>(config_.eventCooldownSeconds) * 1000});
    int cameraModeIndex = 0;
    if (config_.cameraMode == QStringLiteral("single_stereo_frame"))
        cameraModeIndex = 1;
    else if (config_.cameraMode == QStringLiteral("separate_devices"))
        cameraModeIndex = 2;
    mode_->setCurrentIndex(cameraModeIndex);
    primaryIndex_->setValue(config_.primaryCameraIndex);
    secondaryIndex_->setValue(config_.secondaryCameraIndex);

    const QString cascade = findCascade();
    if (!cascade.isEmpty()) cascadeReady_ = recognizer_.loadCascade(cascade, &error);
    const QString facesPath = QDir(enrollmentDirectory_).filePath(QStringLiteral("faces"));
    const QString modelPath = QDir(enrollmentDirectory_).filePath(QStringLiteral("model.yml"));
    const QString labelsPath = QDir(enrollmentDirectory_).filePath(QStringLiteral("labels.csv"));
    bool modelReady = recognizer_.loadModel(modelPath, labelsPath, &error);
    if (!modelReady && !QDir(facesPath).entryList(
            QDir::Dirs | QDir::NoDotAndDotDot).isEmpty()) {
        modelReady = recognizer_.train(facesPath, modelPath, labelsPath, &error);
        if (modelReady)
            statusBar()->showMessage(QStringLiteral("已从保存的人脸样本恢复识别模型"));
    }
    if (cascade.isEmpty())
        statusBar()->showMessage(QStringLiteral("未找到检测模型，请设置 SAW_CASCADE_PATH"));
    else
        statusBar()->showMessage(QStringLiteral("视觉模块就绪"));

    databaseReady_ = eventStore_.open(config_.databaseFile, &error);
    if (!databaseReady_)
        statusBar()->showMessage(QStringLiteral("审计数据库不可用：%1").arg(error));
    else
        eventStore_.purgeOlderThan(config_.retentionDays, nullptr);

    saw::speech::SpeechOptions speechOptions;
    speechOptions.enabled = config_.speechEnabled;
    speechOptions.locale = config_.speechLocale;
    speechOptions.rate = config_.speechRate;
    speechOptions.volume = config_.speechVolume;
    if (!speech_.initialize(speechOptions, &error) && config_.speechEnabled)
        statusBar()->showMessage(QStringLiteral("语音播报不可用：%1").arg(error));
    connect(&speech_, &saw::speech::SpeechAnnouncer::availabilityChanged,
            this, [this](bool) { updateReadiness(); });
    connect(&speech_, &saw::speech::SpeechAnnouncer::errorOccurred,
            this, [this](const QString &message) {
                statusBar()->showMessage(QStringLiteral("语音播报：%1").arg(message));
            });

    accessServer_.configure(config_.deviceId, config_.deviceName);
    updateReadiness();
    if (!accessServer_.start(config_.bindAddress, config_.httpPort)) {
        setStatusPill(ui_->serviceStatusLabel, QStringLiteral("● 本地服务异常"), "warning");
        statusBar()->showMessage(QStringLiteral("本地服务启动失败：%1").arg(accessServer_.errorString()));
    } else {
        setStatusPill(ui_->serviceStatusLabel,
                      QStringLiteral("● 本地服务 %1").arg(config_.httpPort), "ready");
        const QUrl welcomeUrl(QStringLiteral("http://127.0.0.1:%1/").arg(config_.httpPort));
        QTimer::singleShot(0, this, [this, welcomeUrl] {
            if (!QDesktopServices::openUrl(welcomeUrl))
                statusBar()->showMessage(
                    QStringLiteral("无法自动打开欢迎页：%1").arg(welcomeUrl.toString()));
        });
    }
    updateReadiness();

    connect(openButton_, &QPushButton::clicked, this, &MainWindow::openCamera);
    connect(closeButton_, &QPushButton::clicked, this, &MainWindow::closeCamera);
    connect(enrollButton_, &QPushButton::clicked, this, &MainWindow::startEnrollment);
    connect(recognizeButton_, &QPushButton::clicked, this, &MainWindow::toggleRecognition);
    connect(passwordAccessButton_, &QPushButton::clicked, this, &MainWindow::openWithPassword);
    connect(&timer_, &QTimer::timeout, this, &MainWindow::processFrame);
    stateResetTimer_.setSingleShot(true);
    connect(&stateResetTimer_, &QTimer::timeout, this, [this] {
        if (camera_.isOpen()) {
            accessServer_.setTerminalStatus(recognizing_ ? QStringLiteral("verifying")
                                                         : QStringLiteral("detecting"),
                                            recognizing_ ? QStringLiteral("正在验证身份")
                                                         : config_.defaultText);
            setStatusPill(ui_->mainStateBadge,
                          recognizing_ ? QStringLiteral("● 身份验证中") : QStringLiteral("● 设备在线"),
                          recognizing_ ? "active" : "ready");
        }
    });
    connect(mode_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { secondaryIndex_->setEnabled(index == 2); });
    setControlsForCamera(false);
}

QString MainWindow::findConfig() const
{
    const QStringList candidates = {
        qEnvironmentVariable("SAW_CONFIG_PATH"),
        QDir(QCoreApplication::applicationDirPath()).filePath("config.json"),
        QDir::current().filePath("config/config.json")
    };
    for (const QString &path : candidates)
        if (!path.isEmpty() && QFileInfo::exists(path)) return path;
    return {};
}

void MainWindow::updateReadiness()
{
    accessServer_.setReadiness(camera_.isOpen() ? QStringLiteral("ready") : QStringLiteral("offline"),
                               recognizer_.ready() ? QStringLiteral("ready") :
                                 (cascadeReady_ ? QStringLiteral("model_missing") : QStringLiteral("unavailable")),
                               databaseReady_ ? QStringLiteral("ready") : QStringLiteral("unavailable"),
                               speech_.isAvailable() ? QStringLiteral("ready") :
                                 (config_.speechEnabled ? QStringLiteral("unavailable") : QStringLiteral("disabled")));
    ui_->cameraStatusValue->setText(camera_.isOpen() ? QStringLiteral("在线") : QStringLiteral("离线"));
    ui_->recognizerStatusValue->setText(recognizer_.ready()
        ? QStringLiteral("模型就绪 · 最低 %1%").arg(config_.minimumRecognitionAccuracy, 0, 'f', 0) :
        (cascadeReady_ ? QStringLiteral("等待录入") : QStringLiteral("检测器缺失")));
    ui_->databaseStatusValue->setText(databaseReady_ ? QStringLiteral("SQLite 就绪") : QStringLiteral("不可用"));
    ui_->speechStatusValue->setText(speech_.isAvailable() ? QStringLiteral("中文引擎就绪") :
        (config_.speechEnabled ? QStringLiteral("初始化中/不可用") : QStringLiteral("已关闭")));
}

bool MainWindow::recordDecision(const QString &decision, const QString &reason,
                                const QString &personId, const QString &displayName,
                                double confidence, const QString &doorAction,
                                QString *eventId)
{
    saw::persistence::AccessEvent event;
    event.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    event.deviceId = config_.deviceId;
    event.personId = personId;
    event.displayName = displayName;
    event.decision = decision;
    event.reason = reason;
    event.confidence = confidence;
    event.doorAction = doorAction;
    event.occurredAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString error;
    if (!databaseReady_) error = QStringLiteral("数据库未就绪");
    if (!databaseReady_ || !eventStore_.append(event, &error)) {
        statusBar()->showMessage(QStringLiteral("审计事件写入失败：%1").arg(error));
        return false;
    }
    if (eventId) *eventId = event.id;
    return true;
}

MainWindow::~MainWindow()
{
    closeCamera();
    delete ui_;
}

QString MainWindow::findCascade() const
{
    const QStringList candidates = {
        qEnvironmentVariable("SAW_CASCADE_PATH"),
        QDir(QCoreApplication::applicationDirPath()).filePath("haarcascade_frontalface_default.xml"),
        QDir(QCoreApplication::applicationDirPath()).filePath("cascades/haarcascade_frontalface_default.xml")
    };
    for (const QString &path : candidates)
        if (!path.isEmpty() && QFileInfo::exists(path)) return path;
    return {};
}

void MainWindow::openCamera()
{
    saw::camera::CameraOptions options;
    switch (mode_->currentIndex()) {
    case 1:
        options.mode = saw::camera::CameraMode::SideBySideDevice;
        break;
    case 2:
        options.mode = saw::camera::CameraMode::SeparateDevices;
        break;
    default:
        options.mode = saw::camera::CameraMode::SingleDevice;
        break;
    }
    options.primaryIndex = primaryIndex_->value();
    options.secondaryIndex = secondaryIndex_->value();
    options.frameWidth = config_.frameWidth;
    options.frameHeight = config_.frameHeight;
    std::string error;
    if (!camera_.open(options, &error)) {
        QMessageBox::warning(this, QStringLiteral("摄像头错误"), QString::fromStdString(error));
        return;
    }
    timer_.start(config_.captureIntervalMs);
    visionFrameCounter_ = 0;
    cachedFaces_.clear();
    cachedMatches_.clear();
    setControlsForCamera(true);
    statusBar()->showMessage(QStringLiteral("摄像头已开启"));
    accessServer_.setTerminalStatus(QStringLiteral("detecting"), config_.defaultText);
    setStatusPill(ui_->mainStateBadge, QStringLiteral("● 设备在线"), "ready");
    updateReadiness();
}

void MainWindow::closeCamera()
{
    timer_.stop();
    camera_.close();
    recognizing_ = false;
    enrolling_ = false;
    visionFrameCounter_ = 0;
    cachedFaces_.clear();
    cachedMatches_.clear();
    enrollmentProgress_->setVisible(false);
    accessPolicy_.reset();
    video_->setPixmap({});
    video_->setText(QStringLiteral("摄像头未开启"));
    setControlsForCamera(false);
    accessServer_.setTerminalStatus(QStringLiteral("idle"), QStringLiteral("摄像头未开启"));
    setStatusPill(ui_->mainStateBadge, QStringLiteral("● 等待设备"), "offline");
    updateReadiness();
}

void MainWindow::setControlsForCamera(bool open)
{
    mode_->setEnabled(!open);
    primaryIndex_->setEnabled(!open);
    secondaryIndex_->setEnabled(!open && mode_->currentIndex() == 2);
    openButton_->setEnabled(!open);
    closeButton_->setEnabled(open);
    // Keep both actions clickable so they can start the camera themselves and
    // report missing prerequisites instead of silently appearing unresponsive.
    enrollButton_->setEnabled(true);
    recognizeButton_->setEnabled(true);
    passwordAccessButton_->setEnabled(config_.passwordAccessEnabled);
    recognizeButton_->setText(QStringLiteral("开始身份识别"));
}

void MainWindow::startEnrollment()
{
    if (!cascadeReady_) {
        QMessageBox::warning(this, QStringLiteral("录入不可用"),
                             QStringLiteral("未找到人脸检测器，请配置 SAW_CASCADE_PATH。"));
        return;
    }
    if (!camera_.isOpen()) {
        openCamera();
        if (!camera_.isOpen()) return;
    }
    bool accepted = false;
    const QString input = QInputDialog::getText(this, QStringLiteral("录入人脸"),
                                                QStringLiteral("请输入显示姓名："),
                                                QLineEdit::Normal, {}, &accepted);
    if (!accepted) return;
    enrollmentName_ = safeDirectoryName(input);
    if (enrollmentName_.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("姓名无效"),
                                 QStringLiteral("请输入有效姓名。"));
        return;
    }
    const QString personDirectory = QDir(enrollmentDirectory_).filePath("faces/" + enrollmentName_);
    if (!QDir().mkpath(personDirectory)) {
        QMessageBox::warning(this, QStringLiteral("录入失败"),
                             QStringLiteral("无法创建样本目录。"));
        return;
    }
    recognizing_ = false;
    enrolling_ = true;
    accessPolicy_.reset();
    enrollmentCount_ = 0;
    enrollmentFrameSkip_ = 0;
    visionFrameCounter_ = 0;
    cachedFaces_.clear();
    cachedMatches_.clear();
    enrollmentProgress_->setRange(0, EnrollmentTarget);
    enrollmentProgress_->setValue(0);
    enrollmentProgress_->setVisible(true);
    recognizeButton_->setText(QStringLiteral("开始身份识别"));
    statusBar()->showMessage(QStringLiteral("正在录入 %1：0/%2，请缓慢转动头部")
                             .arg(enrollmentName_).arg(EnrollmentTarget));
    accessServer_.setTerminalStatus(QStringLiteral("enrolling"),
                                    QStringLiteral("正在录入 %1").arg(enrollmentName_));
    setStatusPill(ui_->mainStateBadge, QStringLiteral("● 正在录入"), "warning");
}

void MainWindow::toggleRecognition()
{
    if (recognizing_) {
        recognizing_ = false;
        cachedFaces_.clear();
        cachedMatches_.clear();
        accessPolicy_.reset();
        recognizeButton_->setText(QStringLiteral("开始身份识别"));
        accessServer_.setTerminalStatus(QStringLiteral("detecting"), config_.defaultText);
        setStatusPill(ui_->mainStateBadge, QStringLiteral("● 设备在线"), "ready");
        return;
    }
    if (!cascadeReady_) {
        QMessageBox::warning(this, QStringLiteral("识别不可用"),
                             QStringLiteral("未找到人脸检测器，请配置 SAW_CASCADE_PATH。"));
        return;
    }
    if (!recognizer_.ready()) {
        QMessageBox::information(this, QStringLiteral("尚未录入人员"),
                                 QStringLiteral("请先点击“录入新人员”完成人脸录入。"));
        return;
    }
    if (!camera_.isOpen()) {
        openCamera();
        if (!camera_.isOpen()) return;
    }
    enrolling_ = false;
    enrollmentProgress_->setVisible(false);
    recognizing_ = true;
    visionFrameCounter_ = 0;
    cachedFaces_.clear();
    cachedMatches_.clear();
    accessPolicy_.reset();
    recognizeButton_->setText(recognizing_ ? QStringLiteral("停止身份识别") : QStringLiteral("开始身份识别"));
    accessServer_.setTerminalStatus(recognizing_ ? QStringLiteral("verifying")
                                                 : QStringLiteral("detecting"),
                                    recognizing_ ? QStringLiteral("正在验证身份")
                                                 : QStringLiteral("请面向摄像头"));
    setStatusPill(ui_->mainStateBadge,
                  recognizing_ ? QStringLiteral("● 身份验证中") : QStringLiteral("● 设备在线"),
                  recognizing_ ? "active" : "ready");
}

void MainWindow::openWithPassword()
{
    if (!config_.passwordAccessEnabled) return;

    bool accepted = false;
    const QString username = QInputDialog::getText(
        this, QStringLiteral("账号密码登录"), QStringLiteral("账号"),
        QLineEdit::Normal, {}, &accepted).trimmed();
    if (!accepted) return;
    const QString password = QInputDialog::getText(
        this, QStringLiteral("账号密码登录"), QStringLiteral("密码"),
        QLineEdit::Password, {}, &accepted);
    if (!accepted) return;
    if (username != config_.passwordAccessUsername || password != config_.passwordAccessPassword) {
        recordDecision(QStringLiteral("denied"), QStringLiteral("invalid_password"),
                       {}, username, 0.0, QStringLiteral("not_requested"));
        QMessageBox::warning(this, QStringLiteral("开门失败"), QStringLiteral("账号或密码错误。"));
        return;
    }

    const QString personId = QStringLiteral("password:%1").arg(username);
    QString eventId;
    if (!recordDecision(QStringLiteral("granted"), QStringLiteral("password_login"),
                        personId, username, 0.0, QStringLiteral("requested"), &eventId)) {
        QMessageBox::warning(this, QStringLiteral("开门失败"),
                             QStringLiteral("审计记录写入失败，门锁保持关闭。"));
        return;
    }
    QString doorError;
    const bool unlocked = doorController_.unlock(config_.displayDurationSeconds * 1000, &doorError);
    QString auditError;
    eventStore_.updateDoorAction(eventId,
        unlocked ? QStringLiteral("simulated_unlocked") : QStringLiteral("failed"), &auditError);
    if (!unlocked) {
        QMessageBox::warning(this, QStringLiteral("开门失败"), doorError);
        return;
    }
    const QString welcomeText = QStringLiteral("账号验证通过，门锁已开启");
    accessServer_.setTerminalStatus(QStringLiteral("granted"), welcomeText);
    accessServer_.publishEvent(QStringLiteral("access.granted"),
        {{"deviceId", config_.deviceId}, {"welcomeText", welcomeText},
         {"doorAction", "simulated_unlocked"},
         {"displayDurationSeconds", config_.displayDurationSeconds}});
    setStatusPill(ui_->mainStateBadge, QStringLiteral("● 账号验证通过"), "ready");
    speech_.announce(welcomeText, true);
    stateResetTimer_.start(config_.displayDurationSeconds * 1000);
}

void MainWindow::processFrame()
{
    saw::camera::StereoFrame frame;
    std::string error;
    if (!camera_.read(frame, &error)) {
        statusBar()->showMessage(QString::fromStdString(error));
        setStatusPill(ui_->mainStateBadge, QStringLiteral("● 视频流异常"), "warning");
        return;
    }
    const bool visionActive = enrolling_ || recognizing_;
    const int visionInterval = enrolling_ ? EnrollmentFrameInterval : RecognitionFrameInterval;
    const bool runVision = visionActive && ++visionFrameCounter_ >= visionInterval;
    if (runVision) {
        visionFrameCounter_ = 0;
        cachedFaces_ = recognizer_.detect(frame.recognitionFrame);
        cachedMatches_.clear();
        cachedMatches_.reserve(cachedFaces_.size());
        for (const cv::Rect &face : cachedFaces_) {
            saw::vision::FaceMatch match;
            if (recognizing_)
                match = recognizer_.recognize(frame.recognitionFrame, face,
                                              config_.confidenceThreshold,
                                              config_.minimumRecognitionAccuracy);
            else
                match.bounds = face;
            cachedMatches_.push_back(std::move(match));
        }
    } else if (!visionActive) {
        cachedFaces_.clear();
        cachedMatches_.clear();
        visionFrameCounter_ = 0;
    }
    const auto &faces = cachedFaces_;

    if (enrolling_ && runVision && !faces.empty() && ++enrollmentFrameSkip_ >= 5) {
        enrollmentFrameSkip_ = 0;
        const cv::Mat sample = saw::vision::FaceRecognizer::normalizedFace(
            frame.recognitionFrame, faces.front());
        const QString personDirectory = QDir(enrollmentDirectory_).filePath("faces/" + enrollmentName_);
        const QString samplePath = QDir(personDirectory).filePath(
            QStringLiteral("sample_%1.jpg").arg(QDateTime::currentMSecsSinceEpoch()));
        QString saveError;
        if (!saveJpeg(samplePath, sample, &saveError)) {
            enrolling_ = false;
            enrollmentProgress_->setVisible(false);
            QMessageBox::warning(this, QStringLiteral("样本保存失败"), saveError);
        } else {
            ++enrollmentCount_;
            enrollmentProgress_->setValue(enrollmentCount_);
            statusBar()->showMessage(QStringLiteral("正在录入 %1：%2/%3")
                                     .arg(enrollmentName_).arg(enrollmentCount_).arg(EnrollmentTarget));
        }
    }
    if (enrolling_ && enrollmentCount_ >= EnrollmentTarget) {
        enrolling_ = false;
        QString trainError;
        const bool trained = recognizer_.train(
            QDir(enrollmentDirectory_).filePath("faces"),
            QDir(enrollmentDirectory_).filePath("model.yml"),
            QDir(enrollmentDirectory_).filePath("labels.csv"), &trainError);
        recognizeButton_->setEnabled(trained);
        passwordAccessButton_->setEnabled(config_.passwordAccessEnabled);
        if (trained) {
            statusBar()->showMessage(QStringLiteral("%1 录入完成，识别模型已更新").arg(enrollmentName_));
            QMessageBox::information(this, QStringLiteral("录入完成"),
                                     QStringLiteral("已采集 %1 张样本并更新模型。")
                                     .arg(EnrollmentTarget));
            accessServer_.setTerminalStatus(QStringLiteral("detecting"), QStringLiteral("请面向摄像头"));
            ui_->recognizerStatusValue->setText(
                QStringLiteral("模型就绪 · 最低 %1%").arg(
                    config_.minimumRecognitionAccuracy, 0, 'f', 0));
            setStatusPill(ui_->mainStateBadge, QStringLiteral("● 录入完成"), "ready");
            enrollmentProgress_->setFormat(QStringLiteral("录入完成 %v/%m"));
            QTimer::singleShot(2500, enrollmentProgress_, [this] {
                if (!enrolling_) enrollmentProgress_->setVisible(false);
            });
        } else {
            enrollmentProgress_->setVisible(false);
            QMessageBox::warning(this, QStringLiteral("训练失败"), trainError);
        }
    }
    QImage image = toImage(frame.displayFrame);
    QPainter painter(&image);
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 11, QFont::Bold));
    bool hasAcceptedMatch = false;
    saw::vision::FaceMatch acceptedMatch;
    for (const saw::vision::FaceMatch &match : cachedMatches_) {
        const cv::Rect &face = match.bounds;
        if (match.accepted && !hasAcceptedMatch) {
            acceptedMatch = match;
            hasAcceptedMatch = true;
        }
        painter.setPen(QPen(match.accepted ? QColor("#22c55e") : QColor("#f59e0b"), 2));
        painter.drawRect(face.x, face.y, face.width, face.height);
        if (recognizing_)
            painter.drawText(face.x, qMax(18, face.y - 5),
                QStringLiteral("%1  准确度 %2%").arg(match.displayName)
                    .arg(match.accuracy, 0, 'f', 1));
    }

    if (recognizing_ && runVision && !faces.empty()) {
        saw::access::Observation observation;
        // Multiple simultaneous faces are ambiguous: fail closed instead of
        // selecting whichever accepted result happened to be encountered first.
        observation.matched = hasAcceptedMatch && faces.size() == 1;
        if (hasAcceptedMatch) {
            const QString personId = stablePersonId(acceptedMatch.displayName);
            observation.personId = personId.toStdString();
            observation.displayName = acceptedMatch.displayName.toStdString();
        }
        const auto decision = accessPolicy_.observe(
            observation, QDateTime::currentMSecsSinceEpoch());
        if (decision.decision == saw::access::Decision::Granted) {
            const QString personId = QString::fromStdString(decision.personId);
            const QString displayName = QString::fromStdString(decision.displayName);
            const QString welcomeText = QString(config_.grantedTemplate).replace(
                QStringLiteral("{displayName}"), displayName);
            QString eventId;
            if (!recordDecision(QStringLiteral("granted"), QStringLiteral("recognized"),
                                personId, displayName, acceptedMatch.accuracy,
                                QStringLiteral("requested"), &eventId)) {
                accessServer_.setTerminalStatus(QStringLiteral("denied"),
                    QStringLiteral("系统审计不可用，请联系管理员"));
                setStatusPill(ui_->mainStateBadge, QStringLiteral("● 审计服务异常"), "warning");
                speech_.announce(QStringLiteral("系统暂不可用，请联系管理员"), true);
                stateResetTimer_.start(config_.displayDurationSeconds * 1000);
                return;
            }
            QString doorError;
            const bool unlocked = doorController_.unlock(config_.displayDurationSeconds * 1000,
                                                          &doorError);
            QString auditError;
            if (!eventStore_.updateDoorAction(eventId,
                    unlocked ? QStringLiteral("simulated_unlocked") : QStringLiteral("failed"),
                    &auditError))
                statusBar()->showMessage(QStringLiteral("门锁结果回写失败：%1").arg(auditError));
            accessServer_.setTerminalStatus(QStringLiteral("granted"), welcomeText);
            setStatusPill(ui_->mainStateBadge, QStringLiteral("● 验证通过"), "ready");
            accessServer_.publishEvent(QStringLiteral("access.granted"),
                {{"deviceId", config_.deviceId},
                 {"person", QJsonObject{{"id", personId}, {"displayName", displayName}}},
                 {"welcomeText", welcomeText},
                 {"doorAction", unlocked ? "simulated_unlocked" : "failed"},
                 {"displayDurationSeconds", config_.displayDurationSeconds}});
            speech_.announce(QString(config_.grantedSpeech).replace(
                                 QStringLiteral("{displayName}"), displayName), true);
            stateResetTimer_.start(config_.displayDurationSeconds * 1000);
        } else if (decision.decision == saw::access::Decision::Denied) {
            accessServer_.setTerminalStatus(QStringLiteral("denied"),
                                            QStringLiteral("未识别，请联系管理员"));
            setStatusPill(ui_->mainStateBadge, QStringLiteral("● 验证未通过"), "warning");
            accessServer_.publishEvent(QStringLiteral("access.denied"),
                {{"deviceId", config_.deviceId}, {"reason", "unknown_person"},
                 {"message", QStringLiteral("未识别，请联系管理员")}});
            recordDecision(QStringLiteral("denied"), QStringLiteral("unknown_person"), {}, {},
                           0.0, QStringLiteral("not_requested"));
            speech_.announce(config_.deniedSpeech, true);
            stateResetTimer_.start(config_.displayDurationSeconds * 1000);
        }
    } else if (recognizing_ && runVision) {
        accessPolicy_.reset();
    }
    painter.end();
    video_->setPixmap(QPixmap::fromImage(image).scaled(video_->size(), Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation));
}
