#include "simulation_window.h"

#include <QApplication>
#include <QCoreApplication>
#include <QPushButton>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SmartAccessWelcome"));
    QCoreApplication::setApplicationName(QStringLiteral("CameraSimulationDemo"));

    SimulationWindow window;
    window.show();

    if (QCoreApplication::arguments().contains(QStringLiteral("--smoke-test"))) {
        QTimer::singleShot(0, &window, [&window] {
            if (auto *button = window.findChild<QPushButton *>(QStringLiteral("primaryButton")))
                button->click();
        });
        QTimer::singleShot(150, &window, [&window] {
            if (auto *button = window.findChild<QPushButton *>(QStringLiteral("grantButton")))
                button->click();
        });
        QTimer::singleShot(1500, &application, &QCoreApplication::quit);
    }
    return application.exec();
}
