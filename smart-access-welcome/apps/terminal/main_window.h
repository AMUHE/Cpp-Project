#pragma once

#include <QMainWindow>
#include <QTimer>

#include "saw/camera/stereo_camera.h"
#include "saw/access/access_policy.h"
#include "saw/config/app_config.h"
#include "saw/device/door_controller.h"
#include "saw/persistence/access_event_store.h"
#include "saw/server/access_server.h"
#include "saw/speech/speech_announcer.h"
#include "saw/vision/face_recognizer.h"

class QComboBox;
class QLabel;
class QPushButton;
class QProgressBar;
class QSpinBox;
namespace Ui { class MainWindow; }

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void openCamera();
    void closeCamera();
    void startEnrollment();
    void toggleRecognition();
    void openWithPassword();
    void processFrame();
    void setControlsForCamera(bool open);
    QString findCascade() const;
    QString findConfig() const;
    void updateReadiness();
    bool recordDecision(const QString &decision, const QString &reason,
                        const QString &personId, const QString &displayName,
                        double confidence, const QString &doorAction,
                        QString *eventId = nullptr);

    Ui::MainWindow *ui_{};
    QLabel *video_{};
    QComboBox *mode_{};
    QSpinBox *primaryIndex_{};
    QSpinBox *secondaryIndex_{};
    QPushButton *openButton_{};
    QPushButton *closeButton_{};
    QPushButton *enrollButton_{};
    QPushButton *recognizeButton_{};
    QPushButton *passwordAccessButton_{};
    QProgressBar *enrollmentProgress_{};
    QTimer timer_;
    QTimer stateResetTimer_;
    saw::camera::StereoCamera camera_;
    saw::vision::FaceRecognizer recognizer_;
    saw::server::AccessServer accessServer_;
    saw::access::AccessPolicy accessPolicy_;
    saw::config::AppConfig config_;
    saw::device::SimulatedDoorController doorController_;
    saw::persistence::AccessEventStore eventStore_;
    saw::speech::SpeechAnnouncer speech_;
    QString dataDirectory_;
    QString enrollmentDirectory_;
    bool databaseReady_{false};
    bool cascadeReady_{false};
    bool recognizing_{false};
    bool enrolling_{false};
    QString enrollmentName_;
    int enrollmentCount_{0};
    int enrollmentFrameSkip_{0};
    int visionFrameCounter_{0};
    std::vector<cv::Rect> cachedFaces_;
    std::vector<saw::vision::FaceMatch> cachedMatches_;
    static constexpr int EnrollmentTarget = 20;
    static constexpr int RecognitionFrameInterval = 3;
    static constexpr int EnrollmentFrameInterval = 2;
};
