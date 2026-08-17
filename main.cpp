#include "MainWindow.h"
#include <QApplication>
#include <QLockFile>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Prevent multiple instances
    QLockFile lockFile(QDir::tempPath() + QDir::separator() + "LlamaServerLauncher.lock");
    if (!lockFile.tryLock()) {
        QMessageBox::critical(nullptr, "Application Already Running", 
            "Llama Server Launcher is already running. Only one instance is allowed.");
        return 1;
    }
    
    // Ensure lock is released on exit
    app.setQuitOnLastWindowClosed(true);
    QObject::connect(&app, &QApplication::aboutToQuit, [&lockFile]() {
        lockFile.unlock();
    });
    
    MainWindow window;
    window.show();
    return app.exec();
}
