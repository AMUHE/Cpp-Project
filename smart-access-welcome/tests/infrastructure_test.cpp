#include "saw/config/app_config.h"
#include "saw/device/door_controller.h"
#include "saw/persistence/access_event_store.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class InfrastructureTest final : public QObject {
    Q_OBJECT

private slots:
    void configurationIsLoadedAndBounded()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QFile file(directory.filePath(QStringLiteral("config.json")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"({"schemaVersion":1,"device":{"id":"gate-a"},
          "recognition":{"requiredConsecutiveMatches":4},
          "server":{"bindAddress":"127.0.0.1","httpPort":9090},
          "speech":{"enabled":false}})");
        file.close();

        saw::config::AppConfig config;
        QString error;
        QVERIFY2(saw::config::ConfigLoader::load(file.fileName(), directory.path(), config, &error),
                 qPrintable(error));
        QCOMPARE(config.deviceId, QStringLiteral("gate-a"));
        QCOMPARE(config.requiredConsecutiveMatches, 4);
        QCOMPARE(config.httpPort, quint16(9090));
        QVERIFY(!config.speechEnabled);
        QCOMPARE(config.cameraMode, QStringLiteral("single_device"));
        QCOMPARE(config.frameHeight, 720);
        QVERIFY(config.databaseFile.startsWith(directory.path()));
    }

    void eventsArePersisted()
    {
        QTemporaryDir directory;
        saw::persistence::AccessEventStore store;
        QString error;
        QVERIFY2(store.open(directory.filePath(QStringLiteral("events.db")), &error), qPrintable(error));
        saw::persistence::AccessEvent event{QStringLiteral("event-1"), QStringLiteral("gate-a"),
            QStringLiteral("person-1"), QStringLiteral("Alice"), QStringLiteral("granted"),
            QStringLiteral("recognized"), 42.0, QStringLiteral("simulated_unlocked"),
            QStringLiteral("2026-08-12T10:00:00+08:00")};
        QVERIFY2(store.append(event, &error), qPrintable(error));
        QVERIFY2(store.updateDoorAction(event.id, QStringLiteral("locked_again"), &error),
                 qPrintable(error));
        const auto events = store.recent(10, &error);
        QCOMPARE(events.size(), 1);
        QCOMPARE(events.first().id, event.id);
        QCOMPARE(events.first().displayName, event.displayName);
        QCOMPARE(events.first().doorAction, QStringLiteral("locked_again"));
    }

    void simulatedDoorRelocks()
    {
        saw::device::SimulatedDoorController door;
        QSignalSpy changes(&door, &saw::device::SimulatedDoorController::stateChanged);
        QString error;
        QVERIFY2(door.unlock(100, &error), qPrintable(error));
        QVERIFY(door.isUnlocked());
        QTRY_VERIFY_WITH_TIMEOUT(!door.isUnlocked(), 1000);
        QCOMPARE(changes.count(), 2);
    }
};

QTEST_GUILESS_MAIN(InfrastructureTest)
#include "infrastructure_test.moc"
