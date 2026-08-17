#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QUuid>
#include <QFileInfo>
#include <QJsonParseError>
#include <QMessageBox>
#include <QValidator>
#include <QRegularExpressionValidator>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath);
    bool saveToFile(const QString &filePath, const QJsonObject &doc);

    static QString generateUuid();
    static QString resolvePath(const QString &targetPath, const QString &currentFolder);
    static QStringList parseArguments(const QString &text);
    static QString joinArguments(const QStringList &args);
    static bool validatePath(const QString &path);

    QJsonObject serverToJson(const QJsonObject &server);
    QJsonObject profileToJson(const QJsonObject &profile);

    static QJsonObject jsonToServer(const QJsonObject &json);
    static QJsonObject jsonToProfile(const QJsonObject &json);

    bool addServer(const QJsonObject &server);
    bool updateServer(const QString &uuid, const QJsonObject &server);
    bool deleteServer(const QString &uuid);
    bool serverExists(const QString &uuid) const;
    QJsonObject getServer(const QString &uuid) const;
    QStringList getServerUuids() const;

    bool addProfile(const QJsonObject &profile);
    bool updateProfile(const QString &uuid, const QJsonObject &profile);
    bool deleteProfile(const QString &uuid);
    bool profileExists(const QString &uuid) const;
    QJsonObject getProfile(const QString &uuid) const;
    QStringList getProfileUuids() const;

    bool hasReferencesToServer(const QString &serverUuid) const;

    void setFilePath(const QString &filePath);
    QString filePath() const;
    void setModified(bool modified);
    bool isModified() const;

    QJsonDocument document() const;
    void setDocument(const QJsonDocument &doc);

    QString currentFolderForServer(const QString &serverUuid) const;

private:
    QString m_filePath;
    bool m_modified;
    QJsonDocument m_document;
    QJsonObject m_serversArray;
    QJsonObject m_profilesArray;
};

#endif // CONFIGMANAGER_H
