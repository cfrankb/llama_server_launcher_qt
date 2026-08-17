#include "MainWindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QProcess>
#include <QTextStream>
#include <QApplication>
#include <QFont>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_menuBar(new QMenuBar(this))
    , m_fileMenu(new QMenu("File", m_menuBar))
    , m_recentMenu(new QMenu("Open Recent", m_menuBar))
    , m_tabWidget(new QTabWidget(this))
    , m_serverList(new QListWidget(this))
    , m_profileList(new QListWidget(this))
    , m_addServerButton(new QPushButton("New", this))
    , m_editServerButton(new QPushButton("Edit", this))
    , m_duplicateServerButton(new QPushButton("Duplicate", this))
    , m_deleteServerButton(new QPushButton("Delete", this))
    , m_addProfileButton(new QPushButton("New", this))
    , m_editProfileButton(new QPushButton("Edit", this))
    , m_duplicateProfileButton(new QPushButton("Duplicate", this))
    , m_deleteProfileButton(new QPushButton("Delete", this))
    , m_activeProfileLabel(new QLabel("Active Profile: None", this))
    , m_pathLabel(new QLabel("Path: ", this))
    , m_modelLabel(new QLabel("Model: ", this))
    , m_statusLabel(new QLabel("STOPPED", this))
    , m_launchButton(new QPushButton("Launch Server", this))
    , m_stopButton(new QPushButton("Stop Server", this))
    , m_outputWidget(new QPlainTextEdit(this))
    , m_mainSplitter(new QSplitter(Qt::Horizontal, this))
    , m_configManager(new ConfigManager(this))
    , m_serverProcess(new ServerProcess(this))
    , m_activeProcess(nullptr)
    , m_currentFilePath("")
    , m_fileModified(false)
{
    setWindowTitle("Llama Server Manager");
    resize(1200, 800);

    setupMenuBar();
    setupCentralWidget();
    loadRecentFiles();

    m_outputWidget->setReadOnly(true);
    m_outputWidget->setFont(QFont("Consolas", 10));
    m_outputWidget->setPlainText("Server output will appear here...\n");

    //m_launchButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; font-size: 14px; }");
   // m_stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; font-size: 14px; }");
   // m_stopButton->setEnabled(false);
    enableButtons(true, false);

    connect(m_launchButton, &QPushButton::clicked, this, &MainWindow::onLaunchServer);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopServer);
    connect(m_serverList, &QListWidget::itemSelectionChanged, this, &MainWindow::onServerSelectionChanged);
    connect(m_profileList, &QListWidget::itemSelectionChanged, this, &MainWindow::onProfileSelectionChanged);

    connect(m_addServerButton, &QPushButton::clicked, this, &MainWindow::onAddServer);
    connect(m_editServerButton, &QPushButton::clicked, this, &MainWindow::onEditServer);
    connect(m_duplicateServerButton, &QPushButton::clicked, this, &MainWindow::onDuplicateServer);
    connect(m_deleteServerButton, &QPushButton::clicked, this, &MainWindow::onDeleteServer);
    connect(m_addProfileButton, &QPushButton::clicked, this, &MainWindow::onAddProfile);
    connect(m_editProfileButton, &QPushButton::clicked, this, &MainWindow::onEditProfile);
    connect(m_duplicateProfileButton, &QPushButton::clicked, this, &MainWindow::onDuplicateProfile);
    connect(m_deleteProfileButton, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);

    connect(m_serverProcess, &ServerProcess::serverStarted, this, [this]() {
        m_statusLabel->setText("RUNNING");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        //m_launchButton->setEnabled(false);
        //m_stopButton->setEnabled(true);
        enableButtons(false, true);
    });

    connect(m_serverProcess, &ServerProcess::serverStopped, this, [this](int exitCode) {
        m_statusLabel->setText("STOPPED");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        //m_launchButton->setEnabled(true);
        //m_stopButton->setEnabled(false);
        enableButtons(true, false);
        updateOutput(QString("\n[INFO] Server stopped with exit code: %1\n----------------------------------------\n").arg(exitCode));
    });

    connect(m_serverProcess, &ServerProcess::serverError, this, [this](const QString &error) {
        updateOutput("[ERROR] " + error + "\n");
    });

    connect(m_serverProcess, &ServerProcess::serverOutput, this, [this](const QString &output) {
        updateOutput(output);
    });

    connect(m_serverProcess, &ServerProcess::stateChanged, this, [this](QProcess::ProcessState state) {
        if (state == QProcess::Starting) {
            m_statusLabel->setText("STARTING");
            //m_launchButton->setEnabled(false);
            //m_stopButton->setEnabled(true);
            enableButtons(false, true);
        } else if (state == QProcess::Running) {
            m_statusLabel->setText("RUNNING");
            //m_launchButton->setEnabled(false);
            //m_stopButton->setEnabled(true);
            enableButtons(false, true);
        } else if (state == QProcess::NotRunning) {
            m_statusLabel->setText("STOPPED");
            //m_launchButton->setEnabled(true);
            //m_stopButton->setEnabled(false);
            enableButtons(true, false);
        }
    });

    updateRecentFilesMenu();
    refreshServerList();
    refreshProfileList();
}

void MainWindow::enableButtons(bool stateStart, bool stateStop)
{
    m_launchButton->setEnabled(stateStart);
    m_stopButton->setEnabled(stateStop);

    if (stateStart)
        m_launchButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; font-size: 14px; }");
    else
        m_launchButton->setStyleSheet("QPushButton { background-color: gray; color: white; padding: 10px; font-size: 14px; }");

    if (stateStop)
        m_stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; font-size: 14px; }");
    else
        m_stopButton->setStyleSheet("QPushButton { background-color: gray; color: white; padding: 10px; font-size: 14px; }");
}


MainWindow::~MainWindow()
{
    if (m_activeProcess && m_activeProcess->state() == QProcess::Running) {
        m_activeProcess->kill();
    }
    if (m_activeProcess) {
        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
    }
    saveRecentFiles();
}

void MainWindow::setupMenuBar()
{
    QAction *newAction = new QAction("New...", m_fileMenu);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);
    m_fileMenu->addAction(newAction);

    QAction *openAction = new QAction("Open File...", m_fileMenu);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    m_fileMenu->addAction(openAction);
    updateRecentFilesMenu();
    m_fileMenu->addMenu(m_recentMenu);

    m_fileMenu->addSeparator();

    QAction *saveAction = new QAction("Save", m_fileMenu);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    m_fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction("Save As...", m_fileMenu);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    m_fileMenu->addAction(saveAsAction);

    m_fileMenu->addSeparator();

    QAction *exitAction = new QAction("Exit", m_fileMenu);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    m_fileMenu->addAction(exitAction);


    m_menuBar->addMenu(m_fileMenu);
    setMenuBar(m_menuBar);
}

void MainWindow::setupCentralWidget()
{
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

    QGroupBox *metadataGroup(new QGroupBox("Metadata", rightWidget));
    QVBoxLayout *metadataLayout = new QVBoxLayout(metadataGroup);
    metadataLayout->addWidget(m_activeProfileLabel);
    metadataLayout->addWidget(m_pathLabel);
    metadataLayout->addWidget(m_modelLabel);
    metadataLayout->addWidget(m_statusLabel);
    rightLayout->addWidget(metadataGroup);

    QGroupBox *controlGroup(new QGroupBox("Controls", rightWidget));
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);
    controlLayout->addWidget(m_launchButton);
    controlLayout->addWidget(m_stopButton);
    rightLayout->addWidget(controlGroup);

    rightLayout->addWidget(new QLabel("Output:", rightWidget));
    rightLayout->addWidget(m_outputWidget, 1);

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(rightWidget);

    QWidget *serverTabWidget = new QWidget(this);
    QVBoxLayout *serverTabLayout = new QVBoxLayout(serverTabWidget);
    serverTabLayout->addWidget(new QLabel("Server List:", serverTabWidget));
    serverTabLayout->addWidget(m_serverList);
    QHBoxLayout *serverButtons = new QHBoxLayout();
    serverButtons->addWidget(m_addServerButton);
    serverButtons->addWidget(m_editServerButton);
    serverButtons->addWidget(m_duplicateServerButton);
    serverButtons->addWidget(m_deleteServerButton);
    serverTabLayout->addLayout(serverButtons);
    m_tabWidget->addTab(serverTabWidget, "Servers");

    QWidget *profileTabWidget = new QWidget(this);
    QVBoxLayout *profileTabLayout = new QVBoxLayout(profileTabWidget);
    profileTabLayout->addWidget(new QLabel("Profile List:", profileTabWidget));
    profileTabLayout->addWidget(m_profileList);
    QHBoxLayout *profileButtons = new QHBoxLayout();
    profileButtons->addWidget(m_addProfileButton);
    profileButtons->addWidget(m_editProfileButton);
    profileButtons->addWidget(m_duplicateProfileButton);
    profileButtons->addWidget(m_deleteProfileButton);
    profileTabLayout->addLayout(profileButtons);
    m_tabWidget->addTab(profileTabWidget, "Profiles");

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->addWidget(rightSplitter);
    m_mainSplitter->setStretchFactor(0, 4);
    m_mainSplitter->setStretchFactor(1, 6);

    setCentralWidget(m_mainSplitter);
}

void MainWindow::onNewFile()
{
    if (m_fileModified) {
        int reply = QMessageBox::question(this, "Confirm New File",
            "Current file has unsaved changes. Continue?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) return;
        if (reply == QMessageBox::Yes) {
            saveToFile();
        }
    }

    m_currentFilePath = "";
    m_fileModified = false;
    m_configManager->setModified(false);
    m_serverList->clear();
    m_profileList->clear();
    m_activeProfileLabel->setText("Active Profile: None");
    m_pathLabel->setText("Path: ");
    m_modelLabel->setText("Model: ");
    m_statusLabel->setText("STOPPED");
    updateOutput("New empty configuration created.\n");
}

void MainWindow::onOpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        "Open Configuration File",
        QDir::currentPath(),
        "JSON Files (*.json);;All Files (*)"
    );

    if (!filePath.isEmpty()) {
        if (m_configManager->loadFromFile(filePath)) {
            m_currentFilePath = filePath;
            m_fileModified = false;
            m_configManager->setModified(false);
            
            // Add to recent files
            m_recentFiles.removeAll(filePath);
            m_recentFiles.prepend(filePath);
            if (m_recentFiles.size() > 5) {
                m_recentFiles = m_recentFiles.mid(0, 5);
            }
            saveRecentFiles();
            
            loadRecentFiles();
            refreshServerList();
            refreshProfileList();
            updateOutput(QString("Opened: %1\n").arg(QFileInfo(filePath).fileName()));
        } else {
            QMessageBox::warning(this, "Invalid File", "Failed to parse JSON configuration file.");
        }
    }
}

void MainWindow::onOpenRecent()
{
    updateRecentFilesMenu();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }

    saveToFile();
    m_fileModified = false;
    m_configManager->setModified(false);
    updateOutput("Configuration saved.\n");
}

void MainWindow::onSaveAsFile()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        "Save Configuration As",
        QDir::currentPath(),
        "JSON Files (*.json);;All Files (*)"
    );

    if (!filePath.isEmpty()) {
        m_currentFilePath = filePath;
        saveToFile();
        m_fileModified = false;
        m_configManager->setModified(false);
        
        // Add to recent files
        m_recentFiles.removeAll(filePath);
        m_recentFiles.prepend(filePath);
        if (m_recentFiles.size() > 5) {
            m_recentFiles = m_recentFiles.mid(0, 5);
        }
        saveRecentFiles();
        
        loadRecentFiles();
        updateOutput(QString("Configuration saved as: %1\n").arg(QFileInfo(filePath).fileName()));
    }
}

void MainWindow::onExit()
{
    if (m_activeProcess && m_activeProcess->state() == QProcess::Running) {
        int reply = QMessageBox::question(this, "Confirm Exit",
            "A server is currently running. Stop it and exit?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            m_serverProcess->stopServer();
            QApplication::quit();
        }
        return;
    }

    QApplication::quit();
}

void MainWindow::onLaunchServer()
{
    QListWidgetItem *profileItem = m_profileList->currentItem();
    if (!profileItem) {
        QMessageBox::warning(this, "Warning", "No profile selected. Please select a profile first.");
        return;
    }

    QString profileUuid = profileItem->data(Qt::UserRole).toString();
    QJsonObject profile = m_configManager->getProfile(profileUuid);
    if (profile.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Invalid profile selected.");
        return;
    }

    QString serverUuid = profile.value("server_uuid").toString();
    QJsonObject server = m_configManager->getServer(serverUuid);
    if (server.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Server referenced by profile not found.");
        return;
    }

    QString currentFolder = server.value("current_folder").toString();
    QString binaryPath = ConfigManager::resolvePath(server.value("binary_path").toString(), currentFolder);
    QString modelPath = ConfigManager::resolvePath(profile.value("model_path").toString(), currentFolder);

    QStringList parameters;
    QJsonArray paramArray = profile.value("parameters").toArray();
    for (const QJsonValue &val : paramArray) {
        parameters.append(val.toString());
    }

    if (!QFileInfo::exists(binaryPath)) {
        QMessageBox::critical(this, "Error", QString("Binary path does not exist: %1").arg(binaryPath));
        updateOutput(QString("[ERROR] Binary not found: %1\n").arg(binaryPath));
        return;
    }

    if (!QFileInfo::exists(modelPath)) {
        QMessageBox::critical(this, "Error", QString("Model path does not exist: %1").arg(modelPath));
        updateOutput(QString("[ERROR] Model not found: %1\n").arg(modelPath));
        return;
    }

    m_outputWidget->clear();
    updateOutput(QString("Starting server...\n"));
    updateOutput(QString("Binary: %1\n").arg(binaryPath));
    updateOutput(QString("Model: %1\n").arg(modelPath));
    updateOutput(QString("Arguments: %1\n").arg(parameters.join(" ")));
    updateOutput(QString("----------------------------------------\n"));

    m_activeProcess = new QProcess();
    m_activeProcess->setWorkingDirectory(currentFolder);
    m_activeProcess->setProcessChannelMode(QProcess::MergedChannels);

    // Prepend model path as first argument: -m modelpath
    QStringList finalArgs;
    finalArgs << "-m" << modelPath;
    for (const QString &arg : parameters) {
        // Split arguments with spaces into separate elements
        QStringList splitArgs = arg.split(' ', Qt::SkipEmptyParts);
        for (const QString &sArg : splitArgs) {
            finalArgs << sArg;
        }
    }

    // Set LD_LIBRARY_PATH to include the binary's parent directory
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString binaryDir = QFileInfo(binaryPath).absolutePath();
    QString existingLibPath = env.value("LD_LIBRARY_PATH");
    if (!existingLibPath.isEmpty()) {
        env.insert("LD_LIBRARY_PATH", existingLibPath + ":" + binaryDir);
    } else {
        env.insert("LD_LIBRARY_PATH", binaryDir);
    }
    m_activeProcess->setProcessEnvironment(env);

    connect(m_activeProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray output = m_activeProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            updateOutput(QString::fromUtf8(output));
        }
    });

    connect(m_activeProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray errorOutput = m_activeProcess->readAllStandardError();
        if (!errorOutput.isEmpty()) {
            updateOutput("[ERROR] " + QString::fromUtf8(errorOutput) + "\n");
        }
    });

    connect(m_activeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        updateOutput(QString("\n[INFO] Server stopped with exit code: %1\n----------------------------------------\n").arg(exitCode));
        //m_stopButton->setEnabled(false);
        //m_launchButton->setEnabled(true);
        enableButtons(true, false);

        m_statusLabel->setText("STOPPED");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    });
    connect(m_activeProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        updateOutput("[ERROR] " + m_activeProcess->errorString() + "\n");
    });

    updateOutput(QString("[CMD] %1 %2\n").arg(binaryPath).arg(finalArgs.join(" ")));

    m_activeProcess->start(binaryPath, finalArgs);

    if (!m_activeProcess->waitForStarted(5000)) {
        QMessageBox::critical(this, "Error", "Failed to start server process.");
        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
        return;
    }

    //m_launchButton->setEnabled(false);
    //m_stopButton->setEnabled(true);
    enableButtons(false, true);
    m_statusLabel->setText(QString("RUNNING: %1 @ %2")
                               .arg(server.value("name").toString())
                               .arg(profile.value("name").toString()));
    m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    updateOutput(QString("[INFO] Server running (PID: %1)\n").arg(m_activeProcess->processId()));
}

void MainWindow::onStopServer()
{
    if (m_activeProcess && m_activeProcess->state() == QProcess::Running) {
        updateOutput("\n[INFO] Stopping server...\n");
        m_activeProcess->kill();
        m_activeProcess->waitForFinished(1000);

        // Disconnect all signals before deleting
        disconnect(m_activeProcess, nullptr, this, nullptr);

        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
        //m_launchButton->setEnabled(true);
        //m_stopButton->setEnabled(false);
        enableButtons(true, false);
        m_statusLabel->setText("STOPPED");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        updateOutput("[INFO] Server stopped.\n----------------------------------------\n");
    } else {
        QMessageBox::warning(this, "Warning", "No server is running!");
    }
}

void MainWindow::onAddServer()
{
    showEditServerDialog(QString());
}

void MainWindow::onEditServer()
{
    QListWidgetItem *item = m_serverList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a server to edit.");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    showEditServerDialog(uuid);
}

void MainWindow::onDuplicateServer()
{
    QListWidgetItem *item = m_serverList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a server to duplicate.");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    QJsonObject server = m_configManager->getServer(uuid);
    if (server.isEmpty()) return;

    QString originalName = server.value("name").toString();
    QString newName = originalName + " - Copy";

    QJsonObject newServer = server;
    newServer["uuid"] = ConfigManager::generateUuid();
    newServer["name"] = newName;

    m_configManager->addServer(newServer);
    m_fileModified = true;
    m_configManager->setModified(true);
    refreshServerList();
    updateOutput(QString("Server duplicated: %1\n").arg(newName));
}

void MainWindow::onDeleteServer()
{
    QListWidgetItem *item = m_serverList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a server to delete.");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    QString name = item->text();

    if (m_configManager->hasReferencesToServer(uuid)) {
        int reply = QMessageBox::question(this, "Confirm Delete",
            QString("Server '%1' is referenced by one or more profiles. Delete anyway?")
                .arg(name),
            QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes) return;
    }

    QMessageBox::StandardButton delReply = QMessageBox::question(
        this,
        "Confirm Delete",
        QString("Are you sure you want to delete server '%1'?").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (delReply == QMessageBox::Yes) {
        m_configManager->deleteServer(uuid);
        m_fileModified = true;
        m_configManager->setModified(true);
        refreshServerList();
        refreshProfileList();
        updateOutput(QString("Server deleted: %1\n").arg(name));
    }
}

void MainWindow::onAddProfile()
{
    showEditProfileDialog(QString());
}

void MainWindow::onEditProfile()
{
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a profile to edit.");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    showEditProfileDialog(uuid);
}

void MainWindow::onDuplicateProfile()
{
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a profile to duplicate.");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    QJsonObject profile = m_configManager->getProfile(uuid);
    if (profile.isEmpty()) return;

    QString originalName = profile.value("name").toString();
    QString newName = originalName + " - Copy";

    QJsonObject newProfile = profile;
    newProfile["uuid"] = ConfigManager::generateUuid();
    newProfile["name"] = newName;

    m_configManager->addProfile(newProfile);
    m_fileModified = true;
    m_configManager->setModified(true);
    refreshProfileList();
    updateOutput(QString("Profile duplicated: %1\n").arg(newName));
}

void MainWindow::onDeleteProfile()
{
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a profile to delete.");
        return;
    }

    QString name = item->text();

    QMessageBox::StandardButton delReply = QMessageBox::question(
        this,
        "Confirm Delete",
        QString("Are you sure you want to delete profile '%1'?").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (delReply == QMessageBox::Yes) {
        QString uuid = item->data(Qt::UserRole).toString();
        m_configManager->deleteProfile(uuid);
        m_fileModified = true;
        m_configManager->setModified(true);
        refreshProfileList();
        updateOutput(QString("Profile deleted: %1\n").arg(name));
    }
}

void MainWindow::updateRecentFilesMenu()
{
    m_recentMenu->clear();

    for (const QString &file : m_recentFiles) {
        const QString filename = QFileInfo(file).fileName();
        QAction *action = new QAction(filename, m_recentMenu);
        connect(action, &QAction::triggered, this, [this, file]() {
            if (m_configManager->loadFromFile(file)) {
                m_currentFilePath = file;
                m_fileModified = false;
                m_configManager->setModified(false);
                loadRecentFiles();
                refreshServerList();
                refreshProfileList();
                updateOutput(QString("Opened: %1\n").arg(QFileInfo(file).fileName()));
            }
        });
        m_recentMenu->addAction(action);
    }

    if (m_recentFiles.isEmpty()) {
        m_recentMenu->addSeparator();
        m_recentMenu->addAction("No recent files");
    }
}

void MainWindow::onServerSelectionChanged()
{
    QListWidgetItem *item = m_serverList->currentItem();
    if (!item) return;

    QString uuid = item->data(Qt::UserRole).toString();
    QJsonObject server = m_configManager->getServer(uuid);
    if (server.isEmpty()) return;

    QString currentFolder = server.value("current_folder").toString();
    QString binaryPath = ConfigManager::resolvePath(server.value("binary_path").toString(), currentFolder);
    QString name = server.value("name").toString();

    updateOutput(QString("Server selected: %1 (UUID: %2)\n").arg(name).arg(uuid));
    updateOutput(QString("Binary: %1\n").arg(binaryPath));
    updateOutput(QString("Current Folder: %1\n").arg(currentFolder));
}

void MainWindow::onProfileSelectionChanged()
{
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        m_activeProfileLabel->setText("Active Profile: None");
        m_pathLabel->setText("Path: ");
        m_modelLabel->setText("Model: ");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    QJsonObject profile = m_configManager->getProfile(uuid);
    if (profile.isEmpty()) return;

    QString name = profile.value("name").toString();
    QString modelPath = ConfigManager::resolvePath(profile.value("model_path").toString(), "");
    m_activeProfileLabel->setText("Active Profile: " + name);
    m_pathLabel->setText("Path: ");
    m_modelLabel->setText("Model: " + profile.value("model_path").toString());
    updateOutput(QString("Profile selected: %1\n").arg(name));
}

void MainWindow::onServerBinaryChanged()
{
    m_fileModified = true;
    m_configManager->setModified(true);
}

void MainWindow::onModelStateChanged()
{
    m_fileModified = true;
    m_configManager->setModified(true);
}

void MainWindow::loadFromFile()
{
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No file loaded. Use File > Open File to load a configuration.");
        return;
    }

    if (m_configManager->loadFromFile(m_currentFilePath)) {
        refreshServerList();
        refreshProfileList();
        updateOutput(QString("Configuration loaded from: %1\n").arg(m_currentFilePath));
    } else {
        QMessageBox::warning(this, "Invalid File", "Failed to parse configuration file.");
    }
}

void MainWindow::saveToFile()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }

    if (m_configManager->saveToFile(m_currentFilePath)) {
        updateOutput(QString("Configuration saved to: %1\n").arg(m_currentFilePath));
    } else {
        QMessageBox::critical(this, "Error", "Failed to save configuration file.");
    }
}

void MainWindow::loadRecentFiles()
{
    QSettings settings("LlamaServerLauncher", "RecentFiles");
    QStringList recent = settings.value("recentFiles").toStringList();
    m_recentFiles = recent;
    updateRecentFilesMenu();
}

void MainWindow::saveRecentFiles()
{
    QSettings settings("LlamaServerLauncher", "RecentFiles");
    settings.setValue("recentFiles", m_recentFiles);
}

void MainWindow::refreshServerList()
{
    m_serverList->clear();

    QStringList uuids = m_configManager->getServerUuids();
    for (const QString &uuid : uuids) {
        QJsonObject server = m_configManager->getServer(uuid);
        if (server.isEmpty()) continue;

        QString name = server.value("name").toString();
        QString currentFolder = server.value("current_folder").toString();
        QString binaryPath = ConfigManager::resolvePath(server.value("binary_path").toString(), currentFolder);

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, uuid);
        item->setData(Qt::ToolTipRole, QString("UUID: %1\nBinary: %2\nFolder: %3")
            .arg(uuid).arg(binaryPath).arg(currentFolder));

        if (!QFileInfo::exists(binaryPath)) {
            item->setForeground(QBrush(Qt::red));
        }

        m_serverList->addItem(item);
    }
}

void MainWindow::refreshProfileList()
{
    m_profileList->clear();

    QStringList uuids = m_configManager->getProfileUuids();
    for (const QString &uuid : uuids) {
        QJsonObject profile = m_configManager->getProfile(uuid);
        if (profile.isEmpty()) continue;

        QString name = profile.value("name").toString();
        QString modelPath = ConfigManager::resolvePath(profile.value("model_path").toString(), "");

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, uuid);
        item->setData(Qt::ToolTipRole, QString("UUID: %1\nModel: %2\nServer: %3")
            .arg(uuid).arg(modelPath).arg(profile.value("server_uuid").toString()));

        m_profileList->addItem(item);
    }
}

void MainWindow::updateMetadataDisplay()
{
    QListWidgetItem *item = m_profileList->currentItem();
    if (!item) {
        m_activeProfileLabel->setText("Active Profile: None");
        m_pathLabel->setText("Path: ");
        m_modelLabel->setText("Model: ");
        return;
    }

    QString uuid = item->data(Qt::UserRole).toString();
    QJsonObject profile = m_configManager->getProfile(uuid);
    if (profile.isEmpty()) return;

    QString name = profile.value("name").toString();
    QString modelPath = ConfigManager::resolvePath(profile.value("model_path").toString(), "");
    m_activeProfileLabel->setText("Active Profile: " + name);
    m_pathLabel->setText("Path: ");
    m_modelLabel->setText("Model: " + profile.value("model_path").toString());
}

void MainWindow::updateServerStateButtons()
{
}

void MainWindow::updateOutput(const QString &text)
{
    m_outputWidget->appendPlainText(text);
}

void MainWindow::showEditServerDialog(const QString &uuid)
{
    QDialog dialog(this);
    dialog.setWindowTitle(uuid.isEmpty() ? "New Server" : "Edit Server");
    dialog.resize(500, 300);

    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *binaryPathEdit = new QLineEdit(&dialog);
    QLineEdit *searchPathEdit = new QLineEdit(&dialog);
    QLineEdit *currentFolderEdit = new QLineEdit(&dialog);
    QLineEdit *descriptionEdit = new QLineEdit(&dialog);

    form->addRow("Name:", nameEdit);
    form->addRow("Binary Path:", binaryPathEdit);
    form->addRow("Search Path:", searchPathEdit);
    form->addRow("Current Folder:", currentFolderEdit);
    form->addRow("Description:", descriptionEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (!uuid.isEmpty()) {
        QJsonObject server = m_configManager->getServer(uuid);
        if (!server.isEmpty()) {
            nameEdit->setText(server.value("name").toString());
            binaryPathEdit->setText(server.value("binary_path").toString());
            searchPathEdit->setText(server.value("search_path").toString());
            currentFolderEdit->setText(server.value("current_folder").toString());
            descriptionEdit->setText(server.value("description").toString());
        }
    }

    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject server;
        if (uuid.isEmpty()) {
            server["uuid"] = ConfigManager::generateUuid();
        } else {
            server["uuid"] = uuid;
        }
        server["name"] = nameEdit->text();
        server["binary_path"] = binaryPathEdit->text();
        server["search_path"] = searchPathEdit->text();
        server["current_folder"] = currentFolderEdit->text();
        server["description"] = descriptionEdit->text();

        if (uuid.isEmpty()) {
            m_configManager->addServer(server);
        } else {
            m_configManager->updateServer(uuid, server);
        }

        m_fileModified = true;
        m_configManager->setModified(true);
        refreshServerList();
        updateOutput(QString("Server %1: %2\n").arg(uuid.isEmpty() ? "added" : "updated").arg(nameEdit->text()));
    }
}

void MainWindow::showEditProfileDialog(const QString &uuid)
{
    QDialog dialog(this);
    dialog.setWindowTitle(uuid.isEmpty() ? "New Profile" : "Edit Profile");
    dialog.resize(600, 400);

    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *descriptionEdit = new QLineEdit(&dialog);
    QLineEdit *modelPathEdit = new QLineEdit(&dialog);
    QComboBox *serverSelector = new QComboBox(&dialog);
    QTextEdit *paramsEdit = new QTextEdit(&dialog);
    paramsEdit->setMinimumHeight(150);

    form->addRow("Name:", nameEdit);
    form->addRow("Description:", descriptionEdit);
    form->addRow("Model Path:", modelPathEdit);

    QStringList serverUuids = m_configManager->getServerUuids();
    for (const QString &sUuid : serverUuids) {
        QJsonObject server = m_configManager->getServer(sUuid);
        if (!server.isEmpty()) {
            serverSelector->addItem(server.value("name").toString(), sUuid);
        }
    }
    form->addRow("Server:", serverSelector);
    form->addRow("Parameters (one per line):", paramsEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (!uuid.isEmpty()) {
        QJsonObject profile = m_configManager->getProfile(uuid);
        if (!profile.isEmpty()) {
            nameEdit->setText(profile.value("name").toString());
            descriptionEdit->setText(profile.value("description").toString());
            modelPathEdit->setText(profile.value("model_path").toString());
            QString serverUuid = profile.value("server_uuid").toString();
            for (int i = 0; i < serverSelector->count(); ++i) {
                if (serverSelector->itemData(i).toString() == serverUuid) {
                    serverSelector->setCurrentIndex(i);
                    break;
                }
            }
            QStringList params;
            QJsonArray paramArray = profile.value("parameters").toArray();
            for (const QJsonValue &val : paramArray) {
                params.append(val.toString());
            }
            paramsEdit->setPlainText(ConfigManager::joinArguments(params));
        }
    }

    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject profile;
        if (uuid.isEmpty()) {
            profile["uuid"] = ConfigManager::generateUuid();
        } else {
            profile["uuid"] = uuid;
        }
        profile["name"] = nameEdit->text();
        profile["description"] = descriptionEdit->text();
        profile["model_path"] = modelPathEdit->text();
        profile["server_uuid"] = serverSelector->currentData().toString();
        profile["parameters"] = QJsonArray::fromStringList(ConfigManager::parseArguments(paramsEdit->toPlainText()));

        if (uuid.isEmpty()) {
            m_configManager->addProfile(profile);
        } else {
            m_configManager->updateProfile(uuid, profile);
        }

        m_fileModified = true;
        m_configManager->setModified(true);
        refreshProfileList();
        updateOutput(QString("Profile %1: %2\n").arg(uuid.isEmpty() ? "added" : "updated").arg(nameEdit->text()));
    }
}
