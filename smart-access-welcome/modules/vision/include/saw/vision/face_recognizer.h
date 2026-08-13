#pragma once

#include <QMap>
#include <QString>
#include <opencv2/core.hpp>
#include <opencv2/face.hpp>
#include <opencv2/objdetect.hpp>

#include <vector>

namespace saw::vision {

struct FaceMatch {
    cv::Rect bounds;
    int label{-1};
    QString displayName;
    double distance{0.0};
    double accuracy{0.0};
    bool accepted{false};
};

class FaceRecognizer {
public:
    bool loadCascade(const QString &path, QString *error = nullptr);
    bool loadModel(const QString &modelPath, const QString &labelsPath, QString *error = nullptr);
    bool train(const QString &facesDirectory, const QString &modelPath,
               const QString &labelsPath, QString *error = nullptr);
    std::vector<cv::Rect> detect(const cv::Mat &bgrFrame);
    FaceMatch recognize(const cv::Mat &bgrFrame, const cv::Rect &face,
                        double threshold, double minimumAccuracy) const;
    static cv::Mat normalizedFace(const cv::Mat &bgrFrame, const cv::Rect &face);
    bool ready() const { return !recognizer_.empty(); }

private:
    cv::CascadeClassifier cascade_;
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer_;
    QMap<int, QString> labels_;
};

} // namespace saw::vision
