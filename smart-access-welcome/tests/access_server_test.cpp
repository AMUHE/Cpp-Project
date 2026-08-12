#include "saw/server/access_server.h"

#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTcpSocket>
#include <QWebSocket>
#include <QtTest>

class AccessServerTest final : public QObject {
    Q_OBJECT

    QByteArray get(quint16 port, const QByteArray &path)
    {
        QTcpSocket socket;
        socket.connectToHost(QHostAddress::LocalHost, port);
        if (!socket.waitForConnected(2000)) return {};
        socket.write("GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
        socket.flush();
        QByteArray response;
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < 2000 && socket.state() != QAbstractSocket::UnconnectedState) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            response += socket.readAll();
            QTest::qWait(5);
        }
        response += socket.readAll();
        return response;
    }

private slots:
    void healthAndStatusAreServed()
    {
        saw::server::AccessServer server;
        QVERIFY2(server.start(QHostAddress::LocalHost, 0), qPrintable(server.errorString()));

        const QByteArray health = get(server.port(), "/health/live");
        QVERIFY(health.startsWith("HTTP/1.1 200 OK"));
        QVERIFY(health.contains("\"status\":\"ok\""));

        server.setTerminalStatus(QStringLiteral("detecting"), QStringLiteral("ready"));
        const QByteArray status = get(server.port(), "/api/v1/terminal/status");
        QVERIFY(status.startsWith("HTTP/1.1 200 OK"));
        QVERIFY(status.contains("\"state\":\"detecting\""));
        QVERIFY(status.contains("\"message\":\"ready\""));
    }

    void welcomePageAndReadinessAreServed()
    {
        saw::server::AccessServer server;
        server.configure(QStringLiteral("gate-a"), QStringLiteral("总部大门"));
        server.setReadiness(QStringLiteral("ready"), QStringLiteral("ready"),
                            QStringLiteral("ready"), QStringLiteral("disabled"));
        QVERIFY(server.start(QHostAddress::LocalHost, 0));
        const QByteArray page = get(server.port(), "/");
        QVERIFY(page.startsWith("HTTP/1.1 200 OK"));
        QVERIFY(page.contains("Content-Security-Policy"));
        QVERIFY(page.contains("<!doctype html>"));
        const QByteArray ready = get(server.port(), "/health/ready");
        QVERIFY(ready.contains("\"database\":\"ready\""));
    }

    void websocketReceivesInitialSnapshot()
    {
        saw::server::AccessServer server;
        QVERIFY(server.start(QHostAddress::LocalHost, 0));
        QWebSocket client;
        QSignalSpy connected(&client, &QWebSocket::connected);
        QSignalSpy messages(&client, &QWebSocket::textMessageReceived);
        client.open(QUrl(QStringLiteral("ws://127.0.0.1:%1/ws").arg(server.port())));
        QTRY_COMPARE_WITH_TIMEOUT(connected.count(), 1, 2000);
        QTRY_COMPARE_WITH_TIMEOUT(messages.count(), 1, 2000);

        const QJsonDocument document = QJsonDocument::fromJson(
            messages.first().first().toString().toUtf8());
        QVERIFY(document.isObject());
        QCOMPARE(document.object().value("schemaVersion").toInt(), 1);
        QCOMPARE(document.object().value("type").toString(), QStringLiteral("terminal.snapshot"));
    }
};

QTEST_GUILESS_MAIN(AccessServerTest)
#include "access_server_test.moc"
