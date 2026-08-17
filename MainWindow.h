#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QSettings>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFont>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextStream>
#include <QApplication>
#include <QStatusBar>
#include <QToolBar>
#include <QGroupBox>
#include <QListView>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QSettings>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFont>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextStream>
#include <QApplication>
#include <QStatusBar>
#include <QToolBar>
#include <QProcessEnvironment>

#include "ConfigManager.h"
#include "ServerProcess.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewFile();
    void onOpenFile();
    void onOpenRecent();
    void onSaveFile();
    void onSaveAsFile();
    void onExit();
    void onLaunchServer();
    void onStopServer();
    void onAddServer();
    void onEditServer();
    void onDuplicateServer();
    void onDeleteServer();
    void onAddProfile();
    void onEditProfile();
    void onDuplicateProfile();
    void onDeleteProfile();
    void updateRecentFilesMenu();
    void onServerSelectionChanged();
    void onProfileSelectionChanged();
    void onServerBinaryChanged();
    void onModelStateChanged();

private:
    void setupMenuBar();
    void setupCentralWidget();
    void loadFromFile();
    void saveToFile();
    void loadRecentFiles();
    void saveRecentFiles();
    void refreshServerList();
    void refreshProfileList();
    void updateMetadataDisplay();
    void updateServerStateButtons();
    void updateOutput(const QString &text);
    void showEditServerDialog(const QString &uuid = QString());
    void showEditProfileDialog(const QString &uuid = QString());
    void validateAndHighlight();
    void enableButtons(bool stateStart, bool stateStop);

    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_recentMenu;
    QTabWidget *m_tabWidget;
    QListWidget *m_serverList;
    QListWidget *m_profileList;
    QPushButton *m_addServerButton;
    QPushButton *m_editServerButton;
    QPushButton *m_duplicateServerButton;
    QPushButton *m_deleteServerButton;
    QPushButton *m_addProfileButton;
    QPushButton *m_editProfileButton;
    QPushButton *m_duplicateProfileButton;
    QPushButton *m_deleteProfileButton;
    QLabel *m_activeProfileLabel;
    QLabel *m_modelLabel;
    QLabel *m_statusLabel;
    QPushButton *m_launchButton;
    QPushButton *m_stopButton;
    QPlainTextEdit *m_outputWidget;
    QSplitter *m_mainSplitter;

    ConfigManager *m_configManager;
    ServerProcess *m_serverProcess;
    QStringList m_recentFiles;
    QString m_currentFilePath;
    bool m_fileModified;

    QProcess *m_activeProcess;
};

#endif // MAINWINDOW_H
