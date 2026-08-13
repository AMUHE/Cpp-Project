#include "saw/camera/stereo_camera.h"

#include <opencv2/imgproc.hpp>

namespace saw::camera {

bool StereoCamera::openDevice(cv::VideoCapture &capture, int index)
{
#ifdef _WIN32
    if (capture.open(index, cv::CAP_DSHOW)) {
        capture.set(cv::CAP_PROP_BUFFERSIZE, 1);
        return true;
    }
#endif
    if (!capture.open(index)) return false;
    capture.set(cv::CAP_PROP_BUFFERSIZE, 1);
    return true;
}

bool StereoCamera::open(const CameraOptions &options, std::string *error)
{
    close();
    options_ = options;
    if (!openDevice(primary_, options.primaryIndex)) {
        if (error) *error = "cannot open primary camera";
        return false;
    }
    if (options.mode == CameraMode::SeparateDevices) {
        if (options.secondaryIndex == options.primaryIndex ||
            !openDevice(secondary_, options.secondaryIndex)) {
            close();
            if (error) *error = "cannot open distinct secondary camera";
            return false;
        }
        primary_.set(cv::CAP_PROP_FRAME_WIDTH, options.frameWidth / 2);
        secondary_.set(cv::CAP_PROP_FRAME_WIDTH, options.frameWidth / 2);
        primary_.set(cv::CAP_PROP_FRAME_HEIGHT, options.frameHeight);
        secondary_.set(cv::CAP_PROP_FRAME_HEIGHT, options.frameHeight);
    } else {
        primary_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        primary_.set(cv::CAP_PROP_FRAME_WIDTH, options.frameWidth);
        primary_.set(cv::CAP_PROP_FRAME_HEIGHT, options.frameHeight);
    }
    return true;
}

void StereoCamera::close()
{
    primary_.release();
    secondary_.release();
}

bool StereoCamera::isOpen() const
{
    return primary_.isOpened() &&
           (options_.mode != CameraMode::SeparateDevices || secondary_.isOpened());
}

bool StereoCamera::read(StereoFrame &frame, std::string *error)
{
    cv::Mat primary;
    if (!primary_.read(primary) || primary.empty()) {
        if (error) *error = "primary camera frame unavailable";
        return false;
    }
    frame.recognitionFrame = primary;
    if (options_.mode == CameraMode::SeparateDevices) {
        cv::Mat secondary;
        if (!secondary_.read(secondary) || secondary.empty()) {
            if (error) *error = "secondary camera frame unavailable";
            return false;
        }
        if (secondary.rows != primary.rows)
            cv::resize(secondary, secondary,
                       cv::Size(secondary.cols * primary.rows / secondary.rows, primary.rows));
        cv::hconcat(primary, secondary, frame.displayFrame);
    } else {
        frame.displayFrame = primary;
        if (options_.mode == CameraMode::SideBySideDevice &&
            primary.cols >= primary.rows * 2 && primary.cols % 2 == 0)
            frame.recognitionFrame = primary(cv::Rect(0, 0, primary.cols / 2, primary.rows));
    }
    return true;
}

} // namespace saw::camera
