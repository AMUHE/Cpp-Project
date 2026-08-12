#pragma once

#include <QJsonObject>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

namespace saw::persistence {

struct AccessEvent {
    QString id;
    QString deviceId;
    QString personId;
    QString displayName;
    QString decision;
    QString reason;
    double confidence{0.0};
    QString doorAction;
    QString occurredAt;
};

class AccessEventStore {
public:
    AccessEventStore();
    ~AccessEventStore();
    AccessEventStore(const AccessEventStore &) = delete;
    AccessEventStore &operator=(const AccessEventStore &) = delete;

    bool open(const QString &databaseFile, QString *error = nullptr);
    void close();
    bool isOpen() const;
    bool append(const AccessEvent &event, QString *error = nullptr);
    bool updateDoorAction(const QString &eventId, const QString &doorAction,
                          QString *error = nullptr);
    QVector<AccessEvent> recent(int limit, QString *error = nullptr) const;
    bool purgeOlderThan(int retentionDays, QString *error = nullptr);

private:
    bool migrate(QString *error);
    QString connectionName_;
    QSqlDatabase database_;
};

QJsonObject toJson(const AccessEvent &event);

} // namespace saw::persistence
