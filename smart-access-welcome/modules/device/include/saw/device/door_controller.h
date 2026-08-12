#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

namespace saw::device {

class SimulatedDoorController final : public QObject {
    Q_OBJECT
public:
    explicit SimulatedDoorController(QObject *parent = nullptr);
    bool unlock(int durationMilliseconds, QString *error = nullptr);
    bool isUnlocked() const { return unlocked_; }

signals:
    void stateChanged(bool unlocked);

private:
    bool unlocked_{false};
    QTimer lockTimer_;
};

} // namespace saw::device
