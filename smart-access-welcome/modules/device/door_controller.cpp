#include "saw/device/door_controller.h"

namespace saw::device {

SimulatedDoorController::SimulatedDoorController(QObject *parent) : QObject(parent)
{
    lockTimer_.setSingleShot(true);
    connect(&lockTimer_, &QTimer::timeout, this, [this] {
        unlocked_ = false;
        emit stateChanged(false);
    });
}

bool SimulatedDoorController::unlock(int durationMilliseconds, QString *error)
{
    if (durationMilliseconds < 100 || durationMilliseconds > 60000) {
        if (error) *error = QStringLiteral("开门持续时间必须在 100 到 60000 毫秒之间");
        return false;
    }
    unlocked_ = true;
    lockTimer_.start(durationMilliseconds);
    emit stateChanged(true);
    return true;
}

} // namespace saw::device
