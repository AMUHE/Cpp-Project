#pragma once

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>
#include <QWebSocketServer>

class QTcpSocket;
class QWebSocket;

namespace saw::server {

class AccessServer final : public QObject {
    Q_OBJECT
public:
    explicit AccessServer(QObject *parent = nullptr);
    ~AccessServer() override;

    bool start(const QHostAddress &address, quint16 port);
    void stop();
    bool isRunning() const;
    quint16 port() const;
    QString errorString() const;

    void configure(QString deviceId, QString deviceName);
    void setReadiness(QString camera, QString recognizer, QString database,
                      QString speech);
    void setTerminalStatus(QString state, QString message);
    void publishEvent(const QString &type, const QJsonObject &payload);

signals:
    void clientCountChanged(int count);

private:
    void acceptConnections();
    void inspectRequest(QTcpSocket *socket);
    void handleHttp(QTcpSocket *socket, const QByteArray &request);
    void acceptWebSocket();
    QByteArray response(int status, const QByteArray &contentType,
                        const QByteArray &body) const;
    QByteArray jsonResponse(int status, const QJsonObject &body) const;
    QJsonObject snapshot() const;

    QTcpServer tcpServer_;
    QWebSocketServer webSocketServer_;
    QHash<QTcpSocket *, QByteArray> requests_;
    QList<QWebSocket *> clients_;
    QString state_{QStringLiteral("idle")};
    QString message_{QStringLiteral("请面向摄像头")};
    QString updatedAt_;
    QString deviceId_{QStringLiteral("terminal-demo-01")};
    QString deviceName_{QStringLiteral("智能门禁终端")};
    QJsonObject readiness_{{"camera", "offline"}, {"recognizer", "unavailable"},
                           {"database", "unavailable"}, {"speech", "unavailable"}};
};

} // namespace saw::server
