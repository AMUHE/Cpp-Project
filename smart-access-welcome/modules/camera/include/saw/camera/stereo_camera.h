#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>

#include <string>

namespace saw::camera {

enum class CameraMode { SingleDevice, SideBySideDevice, SeparateDevices };

struct CameraOptions {
    CameraMode mode{CameraMode::SingleDevice};
    int primaryIndex{0};
    int secondaryIndex{1};
    int frameWidth{1280};
    int frameHeight{720};
};

struct StereoFrame {
    cv::Mat recognitionFrame;
    cv::Mat displayFrame;
};

class StereoCamera {
public:
    bool open(const CameraOptions &options, std::string *error = nullptr);
    void close();
    bool isOpen() const;
    bool read(StereoFrame &frame, std::string *error = nullptr);

private:
    static bool openDevice(cv::VideoCapture &capture, int index);
    CameraOptions options_;
    cv::VideoCapture primary_;
    cv::VideoCapture secondary_;
};

} // namespace saw::camera
