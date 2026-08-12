#include "main_window.h"
#include "saw/logging/json_logger.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SmartAccessWelcome"));
    QCoreApplication::setApplicationName(QStringLiteral("SmartAccessWelcome"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QString dataDirectory = qEnvironmentVariable("SAW_DATA_DIR").trimmed();
    if (dataDirectory.isEmpty())
        dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    saw::logging::JsonLogger::install(QDir(dataDirectory).filePath(QStringLiteral("logs")));
    qInfo("Smart Access Welcome starting");
    MainWindow window;
    window.show();
    const int result = app.exec();
    qInfo("Smart Access Welcome stopped");
    saw::logging::JsonLogger::shutdown();
    return result;
}
