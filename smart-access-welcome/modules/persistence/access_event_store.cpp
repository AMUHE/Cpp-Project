#include "saw/persistence/access_event_store.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace saw::persistence {

AccessEventStore::AccessEventStore()
    : connectionName_(QStringLiteral("saw-events-%1").arg(
          QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

AccessEventStore::~AccessEventStore() { close(); }

bool AccessEventStore::open(const QString &databaseFile, QString *error)
{
    close();
    if (!QDir().mkpath(QFileInfo(databaseFile).absolutePath())) {
        if (error) *error = QStringLiteral("无法创建数据库目录");
        return false;
    }
    database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    database_.setDatabaseName(databaseFile);
    if (!database_.open()) {
        if (error) *error = database_.lastError().text();
        return false;
    }
    QSqlQuery pragma(database_);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys=ON"));
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    pragma.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));
    pragma.exec(QStringLiteral("PRAGMA busy_timeout=5000"));
    return migrate(error);
}

void AccessEventStore::close()
{
    if (database_.isValid()) database_.close();
    database_ = {};
    if (QSqlDatabase::contains(connectionName_))
        QSqlDatabase::removeDatabase(connectionName_);
}

bool AccessEventStore::isOpen() const { return database_.isOpen(); }

bool AccessEventStore::migrate(QString *error)
{
    QSqlQuery query(database_);
    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS access_events ("
        "id TEXT PRIMARY KEY, device_id TEXT NOT NULL, person_id TEXT, display_name TEXT, "
        "decision TEXT NOT NULL CHECK(decision IN ('granted','denied')), reason TEXT NOT NULL, "
        "confidence REAL NOT NULL DEFAULT 0, door_action TEXT NOT NULL, occurred_at TEXT NOT NULL) ");
    if (!query.exec(sql) ||
        !query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_access_events_time ON access_events(occurred_at DESC)"))) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool AccessEventStore::append(const AccessEvent &event, QString *error)
{
    QSqlQuery query(database_);
    query.prepare(QStringLiteral(
        "INSERT INTO access_events(id,device_id,person_id,display_name,decision,reason,confidence,door_action,occurred_at) "
        "VALUES(?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(event.id);
    query.addBindValue(event.deviceId);
    query.addBindValue(event.personId.isEmpty() ? QVariant(QVariant::String) : event.personId);
    query.addBindValue(event.displayName.isEmpty() ? QVariant(QVariant::String) : event.displayName);
    query.addBindValue(event.decision);
    query.addBindValue(event.reason);
    query.addBindValue(event.confidence);
    query.addBindValue(event.doorAction);
    query.addBindValue(event.occurredAt);
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool AccessEventStore::updateDoorAction(const QString &eventId, const QString &doorAction,
                                        QString *error)
{
    QSqlQuery query(database_);
    query.prepare(QStringLiteral("UPDATE access_events SET door_action=? WHERE id=?"));
    query.addBindValue(doorAction);
    query.addBindValue(eventId);
    if (!query.exec() || query.numRowsAffected() != 1) {
        if (error) *error = query.lastError().text().isEmpty()
            ? QStringLiteral("审计事件不存在") : query.lastError().text();
        return false;
    }
    return true;
}

QVector<AccessEvent> AccessEventStore::recent(int limit, QString *error) const
{
    QVector<AccessEvent> result;
    QSqlQuery query(database_);
    query.prepare(QStringLiteral(
        "SELECT id,device_id,person_id,display_name,decision,reason,confidence,door_action,occurred_at "
        "FROM access_events ORDER BY occurred_at DESC LIMIT ?"));
    query.addBindValue(qBound(1, limit, 1000));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next()) {
        result.append({query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString(),
                       query.value(4).toString(), query.value(5).toString(),
                       query.value(6).toDouble(), query.value(7).toString(),
                       query.value(8).toString()});
    }
    return result;
}

bool AccessEventStore::purgeOlderThan(int retentionDays, QString *error)
{
    QSqlQuery query(database_);
    query.prepare(QStringLiteral("DELETE FROM access_events WHERE occurred_at < ?"));
    query.addBindValue(QDateTime::currentDateTime().addDays(-qMax(1, retentionDays)).toString(Qt::ISODate));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonObject toJson(const AccessEvent &event)
{
    return {{"id", event.id}, {"deviceId", event.deviceId}, {"personId", event.personId},
            {"displayName", event.displayName}, {"decision", event.decision},
            {"reason", event.reason}, {"confidence", event.confidence},
            {"doorAction", event.doorAction}, {"occurredAt", event.occurredAt}};
}

} // namespace saw::persistence
