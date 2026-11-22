// ============================================================
// main.cpp – Campus Navigation System
// Dual-mode entry point:
// 1. Standard C++ Menu-Driven Console & JSON API (DSA Core)
// 2. Optional Qt6 GUI Desktop Application (when built with -DUSE_QT_GUI)
// ============================================================

#ifdef USE_QT_GUI
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("VIT Campus Navigator");
    app.setOrganizationName("VIT Pune");
    app.setApplicationVersion("1.0.0");

    // High-DPI support
    app.setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Determine data directory (look alongside executable)
    QString dataDir = QDir::currentPath() + "/data";
    if (!QDir(dataDir).exists()) {
        // Fallback: look in executable directory
        dataDir = QCoreApplication::applicationDirPath() + "/data";
    }

    MainWindow window(dataDir);
    window.show();

    return app.exec();
}
#else

#include "CampusNavigation.h"

int main(int argc, char* argv[]) {
    VITNav::CampusNavigation app;

    // If CLI arguments provided (e.g. --json, --test, --help), handle and exit
    int cliResult = app.handleCli(argc, argv);
    if (cliResult != -1) {
        return cliResult;
    }

    // Otherwise, launch the full interactive console application
    app.run();
    return 0;
}

#endif
