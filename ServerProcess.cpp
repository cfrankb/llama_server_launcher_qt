#include "ServerProcess.h"
#include <QByteArray>
#include <QString>

ServerProcess::ServerProcess(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &ServerProcess::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &ServerProcess::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ServerProcess::onFinished);
    connect(m_process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &ServerProcess::onError);
}

ServerProcess::~ServerProcess()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->deleteLater();
    }
}

bool ServerProcess::startServer(const QString &serverBin, const QStringList &arguments)
{
    if (m_process && m_process->state() == QProcess::Running) {
        return false; // Already running
    }

    m_process->start(serverBin, arguments);

    if (!m_process->waitForStarted(1000)) {
        emit serverError("Failed to start server: " + m_process->errorString());
        return false;
    }

    emit serverStarted();
    return true;
}

void ServerProcess::stopServer()
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
    }
}

bool ServerProcess::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

qint64 ServerProcess::pid() const
{
    return m_process ? m_process->processId() : -1;
}

void ServerProcess::onReadyReadStandardOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
    if (!output.isEmpty()) {
        emit serverOutput(QString::fromUtf8(output));
    }
}

void ServerProcess::onReadyReadStandardError()
{
    QByteArray errorOutput = m_process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        emit serverOutput("[ERROR] " + QString::fromUtf8(errorOutput));
    }
}

void ServerProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit serverStopped(exitCode);
}

void ServerProcess::onError(QProcess::ProcessError error)
{
    emit serverError(m_process->errorString());
}

void ServerProcess::onStateChanged(QProcess::ProcessState state)
{
    emit stateChanged(state);
}
