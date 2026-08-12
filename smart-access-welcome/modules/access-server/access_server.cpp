#include "saw/server/access_server.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QUuid>
#include <QWebSocket>

namespace {
constexpr qint64 MaxRequestBytes = 16 * 1024;
constexpr int MaxConnections = 32;

const QByteArray WelcomePage = QByteArrayLiteral(R"HTML(<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>智能门禁欢迎屏</title><style>
:root{color-scheme:dark;font-family:"Microsoft YaHei UI",system-ui,sans-serif}*{box-sizing:border-box}
body{margin:0;min-height:100vh;display:grid;place-items:center;background:radial-gradient(circle at 50% 10%,#193b56,#07131f 62%);color:#eef8ff}
main{width:min(900px,92vw);padding:56px;text-align:center;border:1px solid #2c6686;border-radius:28px;background:#091d2dcc;box-shadow:0 24px 80px #0008}
.brand{letter-spacing:.24em;color:#73d5ff;font-size:14px}.icon{font-size:72px;margin:28px}h1{font-size:clamp(36px,7vw,72px);margin:12px 0}
#message{font-size:clamp(20px,3vw,34px);color:#b9d8e9}.meta{margin-top:46px;color:#7393a6}.online{color:#5ee6a8}.offline{color:#ff8b8b}
main.granted{border-color:#36db91;box-shadow:0 24px 90px #1db97444}main.denied{border-color:#ef6363;box-shadow:0 24px 90px #e5484844}
</style></head><body><main id="panel"><div class="brand" id="device">SMART ACCESS</div><div class="icon" id="icon">◉</div><h1 id="state">请面向摄像头</h1><div id="message">系统正在连接终端</div><div class="meta"><span id="connection" class="offline">● 正在连接</span> · <span id="time"></span></div></main>
<script>
const $=id=>document.getElementById(id),panel=$('panel');let retry=800,timer;
function render(p){panel.className=p.state||'';$('state').textContent=p.message||'请面向摄像头';$('message').textContent={granted:'身份验证通过',denied:'访问被拒绝',enrolling:'正在采集人脸信息',verifying:'正在验证身份',detecting:'本地识别服务已就绪',idle:'终端当前空闲'}[p.state]||'智能门禁终端';}
function connect(){const ws=new WebSocket(`${location.protocol==='https:'?'wss':'ws'}://${location.host}/ws`);ws.onopen=()=>{$('connection').textContent='● 实时连接';$('connection').className='online';retry=800};ws.onmessage=e=>{try{const m=JSON.parse(e.data);if(m.type==='terminal.snapshot')render(m.payload);else if(m.type==='access.granted')render({state:'granted',message:m.payload.welcomeText});else if(m.type==='access.denied')render({state:'denied',message:m.payload.message});clearTimeout(timer);timer=setTimeout(()=>fetch('/api/v1/terminal/status',{cache:'no-store'}).then(r=>r.json()).then(render),5000)}catch(_){}};ws.onclose=()=>{$('connection').textContent='● 连接中断';$('connection').className='offline';setTimeout(connect,retry);retry=Math.min(retry*2,15000)}}
setInterval(()=>{$('time').textContent=new Date().toLocaleString('zh-CN')},1000);connect();
</script></body></html>)HTML");

QByteArray statusText(int status)
{
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 413: return "Payload Too Large";
    case 503: return "Service Unavailable";
    default: return "Internal Server Error";
    }
}
}

namespace saw::server {

AccessServer::AccessServer(QObject *parent)
    : QObject(parent),
      webSocketServer_(QStringLiteral("Smart Access Welcome"),
                       QWebSocketServer::NonSecureMode, this),
      updatedAt_(QDateTime::currentDateTime().toString(Qt::ISODate))
{
    connect(&tcpServer_, &QTcpServer::newConnection, this, &AccessServer::acceptConnections);
    connect(&webSocketServer_, &QWebSocketServer::newConnection,
            this, &AccessServer::acceptWebSocket);
}

AccessServer::~AccessServer() { stop(); }

bool AccessServer::start(const QHostAddress &address, quint16 port)
{
    if (isRunning()) return true;
    return tcpServer_.listen(address, port);
}

void AccessServer::stop()
{
    tcpServer_.close();
    for (QTcpSocket *socket : requests_.keys()) {
        socket->disconnect(this);
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    requests_.clear();
    for (QWebSocket *client : clients_) {
        client->close();
        client->deleteLater();
    }
    clients_.clear();
    emit clientCountChanged(0);
}

bool AccessServer::isRunning() const { return tcpServer_.isListening(); }
quint16 AccessServer::port() const { return tcpServer_.serverPort(); }
QString AccessServer::errorString() const { return tcpServer_.errorString(); }

void AccessServer::acceptConnections()
{
    while (tcpServer_.hasPendingConnections()) {
        QTcpSocket *socket = tcpServer_.nextPendingConnection();
        if (requests_.size() + clients_.size() >= MaxConnections) {
            socket->write(jsonResponse(503, {{"code", "connection_limit"},
                                             {"message", "too many connections"}}));
            socket->disconnectFromHost();
            continue;
        }
        requests_.insert(socket, {});
        connect(socket, &QTcpSocket::readyRead, this, [this, socket] { inspectRequest(socket); });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
            requests_.remove(socket);
            socket->deleteLater();
        });
    }
}

void AccessServer::inspectRequest(QTcpSocket *socket)
{
    if (!requests_.contains(socket)) return;
    const QByteArray available = socket->peek(MaxRequestBytes + 1);
    if (available.size() > MaxRequestBytes) {
        socket->readAll();
        socket->write(jsonResponse(413, {{"code", "request_too_large"},
                                         {"message", "request headers exceed limit"}}));
        socket->disconnectFromHost();
        return;
    }
    const int headerEnd = available.indexOf("\r\n\r\n");
    if (headerEnd < 0) return;
    const QByteArray firstLine = available.left(available.indexOf("\r\n"));
    const bool webSocketUpgrade = available.toLower().contains("upgrade: websocket") &&
                                  firstLine.startsWith("GET /ws ");
    if (webSocketUpgrade) {
        requests_.remove(socket);
        socket->disconnect(this);
        webSocketServer_.handleConnection(socket);
        return;
    }
    const QByteArray request = socket->read(headerEnd + 4);
    handleHttp(socket, request);
}

void AccessServer::handleHttp(QTcpSocket *socket, const QByteArray &request)
{
    const QByteArray line = request.left(request.indexOf("\r\n"));
    const QList<QByteArray> parts = line.split(' ');
    if (parts.size() != 3) {
        socket->write(jsonResponse(400, {{"code", "bad_request"}, {"message", "invalid request line"}}));
    } else if (parts[0] != "GET") {
        socket->write(jsonResponse(405, {{"code", "method_not_allowed"}, {"message", "GET required"}}));
    } else if (parts[1] == "/" || parts[1] == "/index.html") {
        socket->write(response(200, "text/html; charset=utf-8", WelcomePage));
    } else if (parts[1] == "/health/live") {
        socket->write(jsonResponse(200, {{"status", "ok"}}));
    } else if (parts[1] == "/health/ready") {
        bool ready = true;
        for (const QJsonValue &value : readiness_)
            if (value.toString() == QStringLiteral("unavailable")) ready = false;
        socket->write(jsonResponse(200, {{"status", ready ? "ok" : "degraded"},
            {"components", readiness_}}));
    } else if (parts[1] == "/api/v1/terminal/status") {
        socket->write(jsonResponse(200, snapshot().value("payload").toObject()));
    } else {
        socket->write(jsonResponse(404, {{"code", "not_found"}, {"message", "route not found"}}));
    }
    socket->disconnectFromHost();
}

QByteArray AccessServer::jsonResponse(int status, const QJsonObject &body) const
{
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    return response(status, "application/json; charset=utf-8", payload);
}

QByteArray AccessServer::response(int status, const QByteArray &contentType,
                                  const QByteArray &body) const
{
    QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + ' ' + statusText(status) + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n";
    response += "Content-Security-Policy: default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self' ws: wss:\r\n";
    response += "Referrer-Policy: no-referrer\r\nConnection: close\r\n";
    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n";
    return response + body;
}

QJsonObject AccessServer::snapshot() const
{
    return {{"schemaVersion", 1}, {"type", "terminal.snapshot"},
            {"eventId", QUuid::createUuid().toString(QUuid::WithoutBraces)},
            {"occurredAt", QDateTime::currentDateTime().toString(Qt::ISODate)},
            {"payload", QJsonObject{{"deviceId", deviceId_}, {"deviceName", deviceName_},
                                    {"state", state_}, {"message", message_},
                                    {"updatedAt", updatedAt_}}}};
}

void AccessServer::configure(QString deviceId, QString deviceName)
{
    deviceId_ = std::move(deviceId);
    deviceName_ = std::move(deviceName);
}

void AccessServer::setReadiness(QString camera, QString recognizer, QString database,
                                QString speech)
{
    readiness_ = {{"camera", std::move(camera)}, {"recognizer", std::move(recognizer)},
                  {"database", std::move(database)}, {"speech", std::move(speech)},
                  {"server", "ready"}};
}

void AccessServer::acceptWebSocket()
{
    while (webSocketServer_.hasPendingConnections()) {
        QWebSocket *client = webSocketServer_.nextPendingConnection();
        clients_.append(client);
        client->sendTextMessage(QString::fromUtf8(QJsonDocument(snapshot()).toJson(QJsonDocument::Compact)));
        connect(client, &QWebSocket::disconnected, this, [this, client] {
            clients_.removeOne(client);
            client->deleteLater();
            emit clientCountChanged(clients_.size());
        });
    }
    emit clientCountChanged(clients_.size());
}

void AccessServer::setTerminalStatus(QString state, QString message)
{
    state_ = std::move(state);
    message_ = std::move(message);
    updatedAt_ = QDateTime::currentDateTime().toString(Qt::ISODate);
    const QByteArray event = QJsonDocument(snapshot()).toJson(QJsonDocument::Compact);
    for (QWebSocket *client : clients_) client->sendTextMessage(QString::fromUtf8(event));
}

void AccessServer::publishEvent(const QString &type, const QJsonObject &payload)
{
    const QJsonObject event{{"schemaVersion", 1}, {"type", type},
                            {"eventId", QUuid::createUuid().toString(QUuid::WithoutBraces)},
                            {"occurredAt", QDateTime::currentDateTime().toString(Qt::ISODate)},
                            {"payload", payload}};
    const QString message = QString::fromUtf8(QJsonDocument(event).toJson(QJsonDocument::Compact));
    for (QWebSocket *client : clients_) client->sendTextMessage(message);
}

} // namespace saw::server
