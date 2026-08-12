#pragma once

#include "saw/camera/stereo_camera.h"
#include "saw/device/door_controller.h"
#include "saw/server/access_server.h"

#include <QMainWindow>
#include <QTimer>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTextEdit;

class SimulationWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit SimulationWindow(QWidget *parent = nullptr);
    ~SimulationWindow() override;

private:
    void startSource();
    void stopSource();
    void renderFrame();
    void simulateGranted();
    void simulateDenied();
    void updateDoorState(bool unlocked);
    void updateControls();
    void startWelcomeServer();
    void updateServiceReadiness();
    void setTerminalState(const QString &title, const QString &message,
                          const QString &color);
    void appendEvent(const QString &message);

    QLabel *preview_{};
    QLabel *stateTitle_{};
    QLabel *stateMessage_{};
    QLabel *sourceStatus_{};
    QLabel *doorStatus_{};
    QLabel *serviceStatus_{};
    QComboBox *sourceMode_{};
    QSpinBox *cameraIndex_{};
    QLineEdit *personName_{};
    QPushButton *startButton_{};
    QPushButton *stopButton_{};
    QPushButton *grantButton_{};
    QPushButton *denyButton_{};
    QPushButton *openWelcomeButton_{};
    QTextEdit *eventLog_{};

    QTimer frameTimer_;
    QTimer stateResetTimer_;
    QTimer doorCountdownTimer_;
    saw::camera::StereoCamera camera_;
    saw::device::SimulatedDoorController door_;
    saw::server::AccessServer accessServer_;
    bool running_{false};
    bool syntheticSource_{true};
    int animationFrame_{0};
    qint64 doorDeadlineMs_{0};
    QString welcomeUrl_;
};
