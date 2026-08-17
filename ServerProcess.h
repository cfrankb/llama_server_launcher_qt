#ifndef SERVERPROCESS_H
#define SERVERPROCESS_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QByteArray>

class ServerProcess : public QObject
{
    Q_OBJECT

public:
    explicit ServerProcess(QObject *parent = nullptr);
    ~ServerProcess();

    bool startServer(const QString &binaryPath, const QStringList &arguments);
    void stopServer();
    bool isRunning() const;
    qint64 pid() const;
    void setWorkingDirectory(const QString &dir);

signals:
    void serverStarted();
    void serverStopped(int exitCode);
    void serverError(const QString &error);
    void serverOutput(const QString &output);
    void stateChanged(QProcess::ProcessState state);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onError(QProcess::ProcessError error);
    void onStateChanged(QProcess::ProcessState state);

private:
    QProcess *m_process;
};

#endif // SERVERPROCESS_H
