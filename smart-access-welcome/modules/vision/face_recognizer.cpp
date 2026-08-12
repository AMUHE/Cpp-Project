#include "saw/vision/face_recognizer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace {
cv::Mat readGray(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    const QByteArray bytes = file.readAll();
    const cv::Mat buffer(1, bytes.size(), CV_8UC1,
                         const_cast<char *>(bytes.constData()));
    return cv::imdecode(buffer, cv::IMREAD_GRAYSCALE);
}
}

namespace saw::vision {

bool FaceRecognizer::loadCascade(const QString &path, QString *error)
{
    if (cascade_.load(path.toStdString())) return true;
    if (error) *error = QStringLiteral("无法加载人脸检测模型：%1").arg(path);
    return false;
}

bool FaceRecognizer::loadModel(const QString &modelPath, const QString &labelsPath, QString *error)
{
    if (!QFile::exists(modelPath) || !QFile::exists(labelsPath)) return false;
    try {
        recognizer_ = cv::face::LBPHFaceRecognizer::create();
        recognizer_->read(modelPath.toStdString());
    } catch (const cv::Exception &exception) {
        recognizer_.release();
        if (error) *error = QString::fromStdString(exception.what());
        return false;
    }
    labels_.clear();
    QFile file(labelsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const int separator = line.indexOf(',');
        if (separator > 0)
            labels_.insert(line.left(separator).toInt(), line.mid(separator + 1));
    }
    return true;
}

bool FaceRecognizer::train(const QString &facesDirectory, const QString &modelPath,
                           const QString &labelsPath, QString *error)
{
    QDir root(facesDirectory);
    const QStringList people = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    std::vector<cv::Mat> images;
    std::vector<int> imageLabels;
    labels_.clear();
    for (int label = 0; label < people.size(); ++label) {
        labels_.insert(label, people[label]);
        QDir person(root.filePath(people[label]));
        for (const QString &name : person.entryList({"*.jpg", "*.png"}, QDir::Files)) {
            cv::Mat image = readGray(person.filePath(name));
            if (image.empty()) continue;
            cv::resize(image, image, cv::Size(100, 100));
            images.push_back(image);
            imageLabels.push_back(label);
        }
    }
    if (images.empty()) {
        if (error) *error = QStringLiteral("没有可训练的人脸样本");
        return false;
    }
    try {
        recognizer_ = cv::face::LBPHFaceRecognizer::create();
        recognizer_->train(images, imageLabels);
        QDir().mkpath(QFileInfo(modelPath).absolutePath());
        recognizer_->save(modelPath.toStdString());
    } catch (const cv::Exception &exception) {
        if (error) *error = QString::fromStdString(exception.what());
        return false;
    }
    QSaveFile file(labelsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = file.errorString();
        return false;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    for (auto it = labels_.cbegin(); it != labels_.cend(); ++it)
        stream << it.key() << ',' << it.value() << '\n';
    return file.commit();
}

std::vector<cv::Rect> FaceRecognizer::detect(const cv::Mat &bgrFrame)
{
    std::vector<cv::Rect> faces;
    if (cascade_.empty() || bgrFrame.empty()) return faces;
    cv::Mat gray;
    cv::cvtColor(bgrFrame, gray, cv::COLOR_BGR2GRAY);
    cascade_.detectMultiScale(gray, faces, 1.1, 5, 0, cv::Size(60, 60));
    return faces;
}

cv::Mat FaceRecognizer::normalizedFace(const cv::Mat &bgrFrame, const cv::Rect &face)
{
    cv::Mat gray;
    if (bgrFrame.channels() == 1) gray = bgrFrame;
    else cv::cvtColor(bgrFrame, gray, cv::COLOR_BGR2GRAY);
    cv::Mat normalized;
    cv::resize(gray(face), normalized, cv::Size(100, 100));
    return normalized;
}

FaceMatch FaceRecognizer::recognize(const cv::Mat &frame, const cv::Rect &face,
                                    double threshold) const
{
    FaceMatch result;
    result.bounds = face;
    if (recognizer_.empty()) return result;
    recognizer_->predict(normalizedFace(frame, face), result.label, result.distance);
    result.accepted = result.label >= 0 && result.distance <= threshold && labels_.contains(result.label);
    result.displayName = result.accepted ? labels_.value(result.label) : QStringLiteral("未知");
    return result;
}

} // namespace saw::vision
