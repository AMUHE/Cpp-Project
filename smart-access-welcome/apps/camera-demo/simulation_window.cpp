#include "simulation_window.h"

#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace {
QImage toImage(const cv::Mat &bgr)
{
    if (bgr.empty()) return {};
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888).copy();
}

QFrame *card(QWidget *parent = nullptr)
{
    auto *frame = new QFrame(parent);
    frame->setProperty("card", true);
    return frame;
}

QLabel *caption(const QString &text, QWidget *parent = nullptr)
{
    auto *label = new QLabel(text, parent);
    label->setProperty("caption", true);
    return label;
}
}

SimulationWindow::SimulationWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Smart Access · 摄像头与门禁仿真 Demo"));
    setMinimumSize(1120, 720);
    resize(1280, 800);

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto *header = new QHBoxLayout;
    auto *brand = new QVBoxLayout;
    auto *title = new QLabel(QStringLiteral("Smart Access Simulation"), central);
    title->setObjectName(QStringLiteral("title"));
    auto *subtitle = new QLabel(
        QStringLiteral("无需人脸模型和门锁硬件 · 支持合成画面、电脑内置摄像头和 USB 摄像头"),
        central);
    subtitle->setProperty("muted", true);
    brand->addWidget(title);
    brand->addWidget(subtitle);
    header->addLayout(brand);
    header->addStretch();
    auto *demoBadge = new QLabel(QStringLiteral("● 仿真环境"), central);
    demoBadge->setObjectName(QStringLiteral("demoBadge"));
    header->addWidget(demoBadge);
    root->addLayout(header);

    auto *content = new QHBoxLayout;
    content->setSpacing(16);

    auto *visionCard = card(central);
    auto *visionLayout = new QVBoxLayout(visionCard);
    visionLayout->setContentsMargins(18, 18, 18, 18);
    visionLayout->setSpacing(12);
    auto *visionHeader = new QHBoxLayout;
    auto *visionTitle = new QLabel(QStringLiteral("实时画面"), visionCard);
    visionTitle->setProperty("sectionTitle", true);
    sourceStatus_ = new QLabel(QStringLiteral("未启动"), visionCard);
    sourceStatus_->setObjectName(QStringLiteral("sourceStatus"));
    visionHeader->addWidget(visionTitle);
    visionHeader->addStretch();
    visionHeader->addWidget(sourceStatus_);
    visionLayout->addLayout(visionHeader);

    preview_ = new QLabel(QStringLiteral("点击“启动画面”开始仿真"), visionCard);
    preview_->setObjectName(QStringLiteral("preview"));
    preview_->setMinimumSize(720, 405);
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setScaledContents(false);
    visionLayout->addWidget(preview_, 1);

    auto *stateCard = new QFrame(visionCard);
    stateCard->setObjectName(QStringLiteral("stateCard"));
    auto *stateLayout = new QVBoxLayout(stateCard);
    stateLayout->setContentsMargins(18, 14, 18, 14);
    stateTitle_ = new QLabel(QStringLiteral("等待启动"), stateCard);
    stateTitle_->setObjectName(QStringLiteral("stateTitle"));
    stateMessage_ = new QLabel(QStringLiteral("仿真流程尚未运行"), stateCard);
    stateMessage_->setProperty("muted", true);
    stateLayout->addWidget(stateTitle_);
    stateLayout->addWidget(stateMessage_);
    visionLayout->addWidget(stateCard);
    content->addWidget(visionCard, 7);

    auto *side = new QVBoxLayout;
    side->setSpacing(12);
    auto *controlCard = card(central);
    auto *controls = new QVBoxLayout(controlCard);
    controls->setContentsMargins(18, 18, 18, 18);
    controls->setSpacing(9);
    auto *controlTitle = new QLabel(QStringLiteral("仿真控制台"), controlCard);
    controlTitle->setProperty("sectionTitle", true);
    controls->addWidget(controlTitle);
    controls->addWidget(caption(QStringLiteral("画面来源"), controlCard));
    sourceMode_ = new QComboBox(controlCard);
    sourceMode_->addItem(QStringLiteral("合成仿真画面（无需摄像头）"));
    sourceMode_->addItem(QStringLiteral("电脑内置或外接摄像头"));
    controls->addWidget(sourceMode_);
    controls->addWidget(caption(QStringLiteral("摄像头索引（通常为 0）"), controlCard));
    cameraIndex_ = new QSpinBox(controlCard);
    cameraIndex_->setRange(0, 32);
    cameraIndex_->setToolTip(QStringLiteral("内置摄像头通常为 0，USB 摄像头可尝试 1、2"));
    controls->addWidget(cameraIndex_);

    auto *sourceActions = new QHBoxLayout;
    startButton_ = new QPushButton(QStringLiteral("启动画面"), controlCard);
    startButton_->setObjectName(QStringLiteral("primaryButton"));
    stopButton_ = new QPushButton(QStringLiteral("停止"), controlCard);
    stopButton_->setObjectName(QStringLiteral("stopButton"));
    sourceActions->addWidget(startButton_);
    sourceActions->addWidget(stopButton_);
    controls->addLayout(sourceActions);

    auto *divider = new QFrame(controlCard);
    divider->setFrameShape(QFrame::HLine);
    controls->addWidget(divider);
    controls->addWidget(caption(QStringLiteral("仿真人员姓名"), controlCard));
    personName_ = new QLineEdit(QStringLiteral("张三"), controlCard);
    personName_->setMaxLength(32);
    controls->addWidget(personName_);
    grantButton_ = new QPushButton(QStringLiteral("✓ 模拟识别通过"), controlCard);
    grantButton_->setObjectName(QStringLiteral("grantButton"));
    denyButton_ = new QPushButton(QStringLiteral("× 模拟识别拒绝"), controlCard);
    denyButton_->setObjectName(QStringLiteral("denyButton"));
    controls->addWidget(grantButton_);
    controls->addWidget(denyButton_);
    side->addWidget(controlCard);

    auto *doorCard = card(central);
    auto *doorLayout = new QVBoxLayout(doorCard);
    doorLayout->setContentsMargins(18, 16, 18, 16);
    doorLayout->addWidget(caption(QStringLiteral("模拟门锁"), doorCard));
    doorStatus_ = new QLabel(QStringLiteral("🔒 已上锁"), doorCard);
    doorStatus_->setObjectName(QStringLiteral("doorStatus"));
    doorLayout->addWidget(doorStatus_);
    side->addWidget(doorCard);

    auto *welcomeCard = card(central);
    auto *welcomeLayout = new QVBoxLayout(welcomeCard);
    welcomeLayout->setContentsMargins(18, 16, 18, 16);
    welcomeLayout->setSpacing(8);
    welcomeLayout->addWidget(caption(QStringLiteral("浏览器欢迎页"), welcomeCard));
    serviceStatus_ = new QLabel(QStringLiteral("正在启动本地服务…"), welcomeCard);
    serviceStatus_->setObjectName(QStringLiteral("serviceStatus"));
    serviceStatus_->setWordWrap(true);
    welcomeLayout->addWidget(serviceStatus_);
    openWelcomeButton_ = new QPushButton(QStringLiteral("在浏览器中打开欢迎页"), welcomeCard);
    openWelcomeButton_->setObjectName(QStringLiteral("openWelcomeButton"));
    welcomeLayout->addWidget(openWelcomeButton_);
    side->addWidget(welcomeCard);

    auto *logCard = card(central);
    auto *logLayout = new QVBoxLayout(logCard);
    logLayout->setContentsMargins(18, 16, 18, 16);
    logLayout->addWidget(caption(QStringLiteral("事件记录"), logCard));
    eventLog_ = new QTextEdit(logCard);
    eventLog_->setReadOnly(true);
    eventLog_->setPlaceholderText(QStringLiteral("仿真事件将在这里显示"));
    logLayout->addWidget(eventLog_);
    side->addWidget(logCard, 1);
    content->addLayout(side, 3);
    root->addLayout(content, 1);
    setCentralWidget(central);

    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget { background: #07111f; color: #e9f3fa; font-family: "Microsoft YaHei UI"; font-size: 14px; }
        QLabel#title { font-size: 24px; font-weight: 700; color: #f7fcff; }
        QLabel[muted="true"] { color: #7892a7; }
        QLabel[sectionTitle="true"] { font-size: 18px; font-weight: 700; color: #f4faff; }
        QLabel[caption="true"] { color: #8ba4b7; font-size: 12px; font-weight: 600; }
        QFrame[card="true"] { background: #0b1929; border: 1px solid #18344d; border-radius: 15px; }
        QLabel#demoBadge, QLabel#sourceStatus { color: #65deb0; background: #123b35; border: 1px solid #1e604f; border-radius: 10px; padding: 7px 11px; font-weight: 700; }
        QLabel#preview { background: #030910; color: #698398; border: 1px solid #1b3850; border-radius: 12px; font-size: 17px; }
        QFrame#stateCard { background: #0e2235; border: 1px solid #1d435e; border-radius: 11px; }
        QLabel#stateTitle { font-size: 21px; font-weight: 700; color: #6fd9ff; }
        QLabel#doorStatus { font-size: 20px; font-weight: 700; color: #9db1c1; padding: 5px 0; }
        QLabel#serviceStatus { color: #69d8ff; font-weight: 600; }
        QComboBox, QSpinBox, QLineEdit, QTextEdit { color: #e9f3fa; background: #0d2234; border: 1px solid #294b63; border-radius: 9px; padding: 8px 10px; selection-background-color: #187da5; }
        QComboBox, QSpinBox, QLineEdit { min-height: 28px; }
        QComboBox QAbstractItemView { color: #e9f3fa; background: #102538; selection-background-color: #176d8e; }
        QTextEdit { font-family: Consolas, "Microsoft YaHei UI"; font-size: 12px; }
        QPushButton { min-height: 40px; border-radius: 9px; padding: 0 13px; font-weight: 700; background: #173349; border: 1px solid #295672; color: #dff4ff; }
        QPushButton:hover { background: #20455f; }
        QPushButton#primaryButton, QPushButton#grantButton { background: #57d3fb; border-color: #75ddff; color: #04141c; }
        QPushButton#primaryButton:hover, QPushButton#grantButton:hover { background: #80e1ff; }
        QPushButton#denyButton, QPushButton#stopButton { color: #ffc1c1; background: #361f28; border-color: #673543; }
        QPushButton:disabled { color: #506678; background: #102030; border-color: #1b3347; }
        QFrame[frameShape="4"] { color: #1d3850; }
        QStatusBar { color: #718ba0; background: #081522; }
    )"));

    frameTimer_.setInterval(33);
    stateResetTimer_.setSingleShot(true);
    doorCountdownTimer_.setInterval(100);
    connect(startButton_, &QPushButton::clicked, this, &SimulationWindow::startSource);
    connect(stopButton_, &QPushButton::clicked, this, &SimulationWindow::stopSource);
    connect(grantButton_, &QPushButton::clicked, this, &SimulationWindow::simulateGranted);
    connect(denyButton_, &QPushButton::clicked, this, &SimulationWindow::simulateDenied);
    connect(openWelcomeButton_, &QPushButton::clicked, this, [this] {
        if (!welcomeUrl_.isEmpty()) QDesktopServices::openUrl(QUrl(welcomeUrl_));
    });
    connect(sourceMode_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this] { updateControls(); });
    connect(&frameTimer_, &QTimer::timeout, this, &SimulationWindow::renderFrame);
    connect(&door_, &saw::device::SimulatedDoorController::stateChanged,
            this, &SimulationWindow::updateDoorState);
    connect(&stateResetTimer_, &QTimer::timeout, this, [this] {
        if (running_ && !door_.isUnlocked())
            setTerminalState(QStringLiteral("等待识别"),
                             QStringLiteral("使用右侧按钮触发模拟识别结果"),
                             QStringLiteral("#6fd9ff"));
        if (running_ && !door_.isUnlocked())
            accessServer_.setTerminalStatus(QStringLiteral("detecting"),
                                             QStringLiteral("请面向摄像头"));
    });
    connect(&doorCountdownTimer_, &QTimer::timeout, this, [this] {
        if (!door_.isUnlocked()) {
            doorCountdownTimer_.stop();
            return;
        }
        const double seconds = qMax<qint64>(0, doorDeadlineMs_ -
            QDateTime::currentMSecsSinceEpoch()) / 1000.0;
        doorStatus_->setText(QStringLiteral("🔓 已开锁 · %1 秒后复位").arg(seconds, 0, 'f', 1));
    });

    startWelcomeServer();
    updateControls();
    appendEvent(QStringLiteral("Demo 已就绪，默认不访问真实硬件"));
}

SimulationWindow::~SimulationWindow()
{
    stopSource();
}

void SimulationWindow::startSource()
{
    stopSource();
    syntheticSource_ = sourceMode_->currentIndex() == 0;
    if (!syntheticSource_) {
        saw::camera::CameraOptions options;
        options.mode = saw::camera::CameraMode::SingleDevice;
        options.primaryIndex = cameraIndex_->value();
        options.frameWidth = 1280;
        options.frameHeight = 720;
        std::string error;
        if (!camera_.open(options, &error)) {
            QMessageBox::warning(this, QStringLiteral("无法打开摄像头"),
                QStringLiteral("索引 %1 打开失败。请确认摄像头未被其他程序占用，或尝试其他索引。\n\n%2")
                    .arg(cameraIndex_->value()).arg(QString::fromStdString(error)));
            appendEvent(QStringLiteral("摄像头索引 %1 打开失败").arg(cameraIndex_->value()));
            updateControls();
            return;
        }
    }

    running_ = true;
    animationFrame_ = 0;
    frameTimer_.start();
    sourceStatus_->setText(syntheticSource_ ? QStringLiteral("合成画面运行中")
                                            : QStringLiteral("摄像头 %1 在线").arg(cameraIndex_->value()));
    setTerminalState(QStringLiteral("等待识别"),
                     QStringLiteral("使用右侧按钮触发模拟识别结果"),
                     QStringLiteral("#6fd9ff"));
    appendEvent(syntheticSource_ ? QStringLiteral("已启动合成仿真画面")
                                 : QStringLiteral("已打开摄像头索引 %1").arg(cameraIndex_->value()));
    updateServiceReadiness();
    accessServer_.setTerminalStatus(QStringLiteral("detecting"),
                                    QStringLiteral("请面向摄像头"));
    statusBar()->showMessage(QStringLiteral("仿真流程正在运行"));
    updateControls();
}

void SimulationWindow::stopSource()
{
    const bool wasRunning = running_;
    frameTimer_.stop();
    stateResetTimer_.stop();
    camera_.close();
    running_ = false;
    preview_->setPixmap({});
    preview_->setText(QStringLiteral("点击“启动画面”开始仿真"));
    sourceStatus_->setText(QStringLiteral("未启动"));
    setTerminalState(QStringLiteral("等待启动"), QStringLiteral("仿真流程尚未运行"),
                     QStringLiteral("#91a6b8"));
    if (wasRunning) appendEvent(QStringLiteral("画面来源已停止"));
    updateServiceReadiness();
    accessServer_.setTerminalStatus(QStringLiteral("idle"), QStringLiteral("仿真画面未启动"));
    updateControls();
}

void SimulationWindow::renderFrame()
{
    cv::Mat frame;
    if (syntheticSource_) {
        constexpr int width = 960;
        constexpr int height = 540;
        frame = cv::Mat(height, width, CV_8UC3, cv::Scalar(24, 16, 9));
        for (int y = 0; y < height; y += 36) {
            const int shade = 20 + y / 30;
            cv::rectangle(frame, cv::Rect(0, y, width, qMin(36, height - y)),
                          cv::Scalar(shade, shade + 5, shade + 8), cv::FILLED);
        }
        const int sway = static_cast<int>(18.0 * std::sin(animationFrame_ / 22.0));
        const cv::Point center(width / 2 + sway, height / 2 - 35);
        cv::circle(frame, center, 66, cv::Scalar(125, 172, 199), cv::FILLED,
                   cv::LINE_AA);
        cv::ellipse(frame, cv::Point(center.x, center.y + 155), cv::Size(125, 105),
                    0, 180, 360, cv::Scalar(82, 126, 153), cv::FILLED, cv::LINE_AA);
        cv::rectangle(frame, cv::Rect(center.x - 92, center.y - 92, 184, 205),
                      cv::Scalar(255, 210, 91), 3, cv::LINE_AA);
        const int scanY = (animationFrame_ * 5) % height;
        cv::line(frame, cv::Point(0, scanY), cv::Point(width, scanY),
                 cv::Scalar(235, 184, 39), 2, cv::LINE_AA);
        ++animationFrame_;
    } else {
        saw::camera::StereoFrame captured;
        std::string error;
        if (!camera_.read(captured, &error)) {
            statusBar()->showMessage(QString::fromStdString(error));
            sourceStatus_->setText(QStringLiteral("视频流异常"));
            appendEvent(QStringLiteral("摄像头视频流读取失败"));
            stopSource();
            return;
        }
        frame = captured.displayFrame;
    }

    QImage image = toImage(frame);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(92, 211, 250, 190), 2));
    const QPoint center(image.width() / 2, image.height() / 2);
    painter.drawLine(center.x() - 25, center.y(), center.x() + 25, center.y());
    painter.drawLine(center.x(), center.y() - 25, center.x(), center.y() + 25);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 11, QFont::Bold));
    painter.drawText(QRect(18, 14, image.width() - 36, 30),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     syntheticSource_ ? QStringLiteral("SIMULATED VIDEO")
                                      : QStringLiteral("CAMERA %1").arg(cameraIndex_->value()));
    painter.end();
    preview_->setPixmap(QPixmap::fromImage(image).scaled(
        preview_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void SimulationWindow::simulateGranted()
{
    QString name = personName_->text().trimmed();
    if (name.isEmpty()) name = QStringLiteral("访客");
    doorDeadlineMs_ = QDateTime::currentMSecsSinceEpoch() + 5000;
    QString error;
    if (!door_.unlock(5000, &error)) {
        QMessageBox::warning(this, QStringLiteral("模拟门锁错误"), error);
        return;
    }
    doorCountdownTimer_.start();
    const QString welcomeText = QStringLiteral("欢迎，%1").arg(name);
    setTerminalState(QStringLiteral("识别通过"), welcomeText,
                     QStringLiteral("#64e6af"));
    appendEvent(QStringLiteral("识别通过：%1；模拟门锁开启 5 秒").arg(name));
    accessServer_.setTerminalStatus(QStringLiteral("granted"), welcomeText);
    accessServer_.publishEvent(QStringLiteral("access.granted"),
        {{QStringLiteral("deviceId"), QStringLiteral("simulation-demo")},
         {QStringLiteral("person"), QJsonObject{
             {QStringLiteral("id"), QStringLiteral("simulated-person")},
             {QStringLiteral("displayName"), name}}},
         {QStringLiteral("welcomeText"), welcomeText},
         {QStringLiteral("doorAction"), QStringLiteral("simulated_unlocked")},
         {QStringLiteral("displayDurationSeconds"), 5}});
}

void SimulationWindow::simulateDenied()
{
    setTerminalState(QStringLiteral("访问被拒绝"),
                     QStringLiteral("未识别人员，门锁保持关闭"),
                     QStringLiteral("#ff9b9b"));
    appendEvent(QStringLiteral("识别拒绝：未知人员；门锁保持关闭"));
    accessServer_.setTerminalStatus(QStringLiteral("denied"),
                                    QStringLiteral("未识别人员，门锁保持关闭"));
    accessServer_.publishEvent(QStringLiteral("access.denied"),
        {{QStringLiteral("deviceId"), QStringLiteral("simulation-demo")},
         {QStringLiteral("reason"), QStringLiteral("unknown_person")},
         {QStringLiteral("message"), QStringLiteral("未识别人员，门锁保持关闭")}});
    stateResetTimer_.start(5000);
}

void SimulationWindow::updateDoorState(bool unlocked)
{
    if (unlocked) {
        doorStatus_->setText(QStringLiteral("🔓 已开锁 · 5.0 秒后复位"));
        doorStatus_->setStyleSheet(QStringLiteral("color: #64e6af;"));
    } else {
        doorCountdownTimer_.stop();
        doorStatus_->setText(QStringLiteral("🔒 已上锁"));
        doorStatus_->setStyleSheet(QStringLiteral("color: #9db1c1;"));
        appendEvent(QStringLiteral("模拟门锁已自动复位"));
        if (running_) {
            setTerminalState(QStringLiteral("等待识别"),
                             QStringLiteral("使用右侧按钮触发模拟识别结果"),
                             QStringLiteral("#6fd9ff"));
            accessServer_.setTerminalStatus(QStringLiteral("detecting"),
                                             QStringLiteral("请面向摄像头"));
        } else {
            accessServer_.setTerminalStatus(QStringLiteral("idle"),
                                             QStringLiteral("仿真画面未启动"));
        }
    }
    updateControls();
}

void SimulationWindow::updateControls()
{
    sourceMode_->setEnabled(!running_);
    cameraIndex_->setEnabled(!running_ && sourceMode_->currentIndex() == 1);
    startButton_->setEnabled(!running_);
    stopButton_->setEnabled(running_);
    grantButton_->setEnabled(running_ && !door_.isUnlocked());
    denyButton_->setEnabled(running_ && !door_.isUnlocked());
    personName_->setEnabled(running_);
}

void SimulationWindow::startWelcomeServer()
{
    accessServer_.configure(QStringLiteral("simulation-demo"),
                            QStringLiteral("仿真门禁终端"));
    updateServiceReadiness();
    for (quint16 port = 8080; port <= 8089; ++port) {
        if (accessServer_.start(QHostAddress::LocalHost, port)) {
            welcomeUrl_ = QStringLiteral("http://127.0.0.1:%1/").arg(port);
            serviceStatus_->setText(welcomeUrl_);
            openWelcomeButton_->setEnabled(true);
            appendEvent(QStringLiteral("浏览器欢迎页已启动：%1").arg(welcomeUrl_));
            accessServer_.setTerminalStatus(QStringLiteral("idle"),
                                             QStringLiteral("仿真画面未启动"));
            return;
        }
    }
    welcomeUrl_.clear();
    serviceStatus_->setText(QStringLiteral("端口 8080–8089 均不可用"));
    serviceStatus_->setStyleSheet(QStringLiteral("color: #ff9b9b;"));
    openWelcomeButton_->setEnabled(false);
    appendEvent(QStringLiteral("浏览器欢迎页启动失败：%1").arg(accessServer_.errorString()));
}

void SimulationWindow::updateServiceReadiness()
{
    accessServer_.setReadiness(running_ ? QStringLiteral("ready")
                                        : QStringLiteral("offline"),
                               QStringLiteral("simulated"),
                               QStringLiteral("disabled"),
                               QStringLiteral("disabled"));
}

void SimulationWindow::setTerminalState(const QString &title, const QString &message,
                                        const QString &color)
{
    stateTitle_->setText(title);
    stateTitle_->setStyleSheet(QStringLiteral("color: %1;").arg(color));
    stateMessage_->setText(message);
}

void SimulationWindow::appendEvent(const QString &message)
{
    eventLog_->append(QStringLiteral("[%1] %2")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")), message));
}
