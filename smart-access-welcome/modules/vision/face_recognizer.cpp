#include "saw/vision/face_recognizer.h"

#include <QDir>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
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

QByteArray fileHash(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex();
}

QString manifestPath(const QString &modelPath)
{
    return modelPath + QStringLiteral(".manifest.json");
}

bool verifyModelBundle(const QString &modelPath, const QString &labelsPath, QString *error)
{
    QFile file(manifestPath(modelPath));
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("模型一致性清单缺失");
        return false;
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (error) *error = QStringLiteral("模型一致性清单无效");
        return false;
    }
    const QJsonObject manifest = document.object();
    const QByteArray modelDigest = fileHash(modelPath);
    const QByteArray labelsDigest = fileHash(labelsPath);
    const bool valid = !modelDigest.isEmpty() && !labelsDigest.isEmpty() &&
        manifest.value(QStringLiteral("modelSha256")).toString().toLatin1() == modelDigest &&
        manifest.value(QStringLiteral("labelsSha256")).toString().toLatin1() == labelsDigest;
    if (!valid && error) *error = QStringLiteral("模型与人员标签不匹配，已拒绝加载");
    return valid;
}

bool saveModelManifest(const QString &modelPath, const QString &labelsPath, QString *error)
{
    const QByteArray modelDigest = fileHash(modelPath);
    const QByteArray labelsDigest = fileHash(labelsPath);
    if (modelDigest.isEmpty() || labelsDigest.isEmpty()) {
        if (error) *error = QStringLiteral("无法校验模型文件");
        return false;
    }
    const QJsonObject manifest{{QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("modelSha256"), QString::fromLatin1(modelDigest)},
        {QStringLiteral("labelsSha256"), QString::fromLatin1(labelsDigest)}};
    const QByteArray bytes = QJsonDocument(manifest).toJson(QJsonDocument::Compact);
    QSaveFile file(manifestPath(modelPath));
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
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
    if (!verifyModelBundle(modelPath, labelsPath, error)) return false;
    QFile modelFile(modelPath);
    if (!modelFile.open(QIODevice::ReadOnly)) {
        if (error) *error = modelFile.errorString();
        return false;
    }
    const QByteArray modelBytes = modelFile.readAll();
    try {
        cv::FileStorage storage(std::string(modelBytes.constData(), modelBytes.size()),
                                cv::FileStorage::READ | cv::FileStorage::MEMORY);
        if (!storage.isOpened()) {
            if (error) *error = QStringLiteral("识别模型格式无效");
            return false;
        }
        recognizer_ = cv::face::LBPHFaceRecognizer::create();
        recognizer_->read(storage.getFirstTopLevelNode());
    } catch (const cv::Exception &exception) {
        recognizer_.release();
        if (error) *error = QString::fromStdString(exception.what());
        return false;
    }
    labels_.clear();
    QFile file(labelsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
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
        cv::FileStorage storage(".yml", cv::FileStorage::WRITE | cv::FileStorage::MEMORY);
        storage << recognizer_->getDefaultName() << "{";
        recognizer_->write(storage);
        storage << "}";
        const std::string serialized = storage.releaseAndGetString();
        QSaveFile modelFile(modelPath);
        if (!modelFile.open(QIODevice::WriteOnly) ||
            modelFile.write(serialized.data(), static_cast<qint64>(serialized.size())) !=
                static_cast<qint64>(serialized.size()) || !modelFile.commit()) {
            if (error) *error = modelFile.errorString();
            recognizer_.release();
            return false;
        }
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
    if (!file.commit() || !saveModelManifest(modelPath, labelsPath, error)) {
        recognizer_.release();
        labels_.clear();
        return false;
    }
    return true;
}

std::vector<cv::Rect> FaceRecognizer::detect(const cv::Mat &bgrFrame)
{
    std::vector<cv::Rect> faces;
    if (cascade_.empty() || bgrFrame.empty()) return faces;
    cv::Mat gray;
    if (bgrFrame.channels() == 1) gray = bgrFrame;
    else cv::cvtColor(bgrFrame, gray, cv::COLOR_BGR2GRAY);

    constexpr int DetectionWidth = 640;
    const double scale = gray.cols > DetectionWidth
        ? static_cast<double>(DetectionWidth) / gray.cols : 1.0;
    cv::Mat detectionImage;
    if (scale < 1.0)
        cv::resize(gray, detectionImage, {}, scale, scale, cv::INTER_AREA);
    else
        detectionImage = gray;
    cv::equalizeHist(detectionImage, detectionImage);
    cascade_.detectMultiScale(detectionImage, faces, 1.1, 5, 0,
                              cv::Size(qMax(30, qRound(60 * scale)),
                                       qMax(30, qRound(60 * scale))));
    if (scale < 1.0) {
        for (cv::Rect &face : faces) {
            face.x = qRound(face.x / scale);
            face.y = qRound(face.y / scale);
            face.width = qRound(face.width / scale);
            face.height = qRound(face.height / scale);
            face &= cv::Rect(0, 0, bgrFrame.cols, bgrFrame.rows);
        }
    }
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
                                    double threshold, double minimumAccuracy) const
{
    FaceMatch result;
    result.bounds = face;
    if (recognizer_.empty()) return result;
    recognizer_->predict(normalizedFace(frame, face), result.label, result.distance);
    // LBPH distance is not a probability. This normalized score is intentionally
    // conservative and is used only as a configurable acceptance gate.
    result.accuracy = qBound(0.0, 100.0 - result.distance, 100.0);
    result.accepted = result.label >= 0 && result.distance <= threshold &&
        result.accuracy >= minimumAccuracy && labels_.contains(result.label);
    result.displayName = result.accepted ? labels_.value(result.label) : QStringLiteral("未知");
    return result;
}

} // namespace saw::vision
