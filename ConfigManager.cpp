#include "ConfigManager.h"
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QMessageBox>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_filePath("")
    , m_modified(false)
{
    m_serversArray["servers"] = QJsonArray();
    m_profilesArray["profiles"] = QJsonArray();
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::warning(nullptr, "JSON Parse Error",
            "Failed to parse JSON: Invalid document format.");
        return false;
    }

    if (!doc.isObject()) {
        QMessageBox::warning(nullptr, "JSON Structure Error",
            "Invalid JSON structure: Root must be an object with 'servers' and 'profiles' arrays.");
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("servers") || !root.contains("profiles")) {
        QMessageBox::warning(nullptr, "JSON Structure Error",
            "Missing required 'servers' or 'profiles' array in JSON.");
        return false;
    }

    m_document = doc;
    m_filePath = filePath;
    m_modified = false;
    return true;
}

bool ConfigManager::saveToFile(const QString &filePath)
{
    QJsonObject root = m_document.object();

    // Update servers array
    QJsonArray serversArray = root.value("servers").toArray();
    // This is a placeholder - actual implementation would rebuild from internal state
    root["servers"] = serversArray;

    // Update profiles array
    QJsonArray profilesArray = root.value("profiles").toArray();
    root["profiles"] = profilesArray;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ConfigManager::saveToFile(const QString &filePath, const QJsonObject &doc)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(QJsonDocument(doc).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QString ConfigManager::generateUuid()
{
    return QUuid::createUuid().toString().remove('{').remove('}');
}

QString ConfigManager::resolvePath(const QString &targetPath, const QString &currentFolder)
{
    QFileInfo fi(targetPath);
    if (QDir::isAbsolutePath(targetPath)) {
        return targetPath;
    }
    return QDir(currentFolder).absoluteFilePath(targetPath);
}

QStringList ConfigManager::parseArguments(const QString &text)
{
    QStringList result;
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            result.append(trimmed);
        }
    }
    return result;
}

QString ConfigManager::joinArguments(const QStringList &args)
{
    return args.join('\n');
}

bool ConfigManager::validatePath(const QString &path)
{
    return QFileInfo::exists(path);
}

QJsonObject ConfigManager::serverToJson(const QJsonObject &server)
{
    return server;
}

QJsonObject ConfigManager::profileToJson(const QJsonObject &profile)
{
    return profile;
}

QJsonObject ConfigManager::jsonToServer(const QJsonObject &json)
{
    return json;
}

QJsonObject ConfigManager::jsonToProfile(const QJsonObject &json)
{
    return json;
}

bool ConfigManager::addServer(const QJsonObject &server)
{
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();
    servers.append(server);
    root["servers"] = servers;
    m_document = QJsonDocument(root);
    m_modified = true;
    return true;
}

bool ConfigManager::updateServer(const QString &uuid, const QJsonObject &server)
{
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();

    for (int i = 0; i < servers.size(); ++i) {
        QJsonObject s = servers[i].toObject();
        if (s.value("uuid").toString() == uuid) {
            servers[i] = server;
            root["servers"] = servers;
            m_document = QJsonDocument(root);
            m_modified = true;
            return true;
        }
    }
    return false;
}

bool ConfigManager::deleteServer(const QString &uuid)
{
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();

    for (int i = 0; i < servers.size(); ++i) {
        QJsonObject s = servers[i].toObject();
        if (s.value("uuid").toString() == uuid) {
            servers.removeAt(i);
            root["servers"] = servers;
            m_document = QJsonDocument(root);
            m_modified = true;
            return true;
        }
    }
    return false;
}

bool ConfigManager::serverExists(const QString &uuid) const
{
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();

    for (const QJsonValue &val : servers) {
        QJsonObject s = val.toObject();
        if (s.value("uuid").toString() == uuid) {
            return true;
        }
    }
    return false;
}

QJsonObject ConfigManager::getServer(const QString &uuid) const
{
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();

    for (const QJsonValue &val : servers) {
        QJsonObject s = val.toObject();
        if (s.value("uuid").toString() == uuid) {
            return s;
        }
    }
    return QJsonObject();
}

QStringList ConfigManager::getServerUuids() const
{
    QStringList result;
    QJsonObject root = m_document.object();
    QJsonArray servers = root.value("servers").toArray();

    for (const QJsonValue &val : servers) {
        QJsonObject s = val.toObject();
        result.append(s.value("uuid").toString());
    }
    return result;
}

bool ConfigManager::addProfile(const QJsonObject &profile)
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();
    profiles.append(profile);
    root["profiles"] = profiles;
    m_document = QJsonDocument(root);
    m_modified = true;
    return true;
}

bool ConfigManager::updateProfile(const QString &uuid, const QJsonObject &profile)
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (int i = 0; i < profiles.size(); ++i) {
        QJsonObject p = profiles[i].toObject();
        if (p.value("uuid").toString() == uuid) {
            profiles[i] = profile;
            root["profiles"] = profiles;
            m_document = QJsonDocument(root);
            m_modified = true;
            return true;
        }
    }
    return false;
}

bool ConfigManager::deleteProfile(const QString &uuid)
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (int i = 0; i < profiles.size(); ++i) {
        QJsonObject p = profiles[i].toObject();
        if (p.value("uuid").toString() == uuid) {
            profiles.removeAt(i);
            root["profiles"] = profiles;
            m_document = QJsonDocument(root);
            m_modified = true;
            return true;
        }
    }
    return false;
}

bool ConfigManager::profileExists(const QString &uuid) const
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (const QJsonValue &val : profiles) {
        QJsonObject p = val.toObject();
        if (p.value("uuid").toString() == uuid) {
            return true;
        }
    }
    return false;
}

QJsonObject ConfigManager::getProfile(const QString &uuid) const
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (const QJsonValue &val : profiles) {
        QJsonObject p = val.toObject();
        if (p.value("uuid").toString() == uuid) {
            return p;
        }
    }
    return QJsonObject();
}

QStringList ConfigManager::getProfileUuids() const
{
    QStringList result;
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (const QJsonValue &val : profiles) {
        QJsonObject p = val.toObject();
        result.append(p.value("uuid").toString());
    }
    return result;
}

bool ConfigManager::hasReferencesToServer(const QString &serverUuid) const
{
    QJsonObject root = m_document.object();
    QJsonArray profiles = root.value("profiles").toArray();

    for (const QJsonValue &val : profiles) {
        QJsonObject p = val.toObject();
        if (p.value("server_uuid").toString() == serverUuid) {
            return true;
        }
    }
    return false;
}

void ConfigManager::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

QString ConfigManager::filePath() const
{
    return m_filePath;
}

void ConfigManager::setModified(bool modified)
{
    m_modified = modified;
}

bool ConfigManager::isModified() const
{
    return m_modified;
}

QJsonDocument ConfigManager::document() const
{
    return m_document;
}

void ConfigManager::setDocument(const QJsonDocument &doc)
{
    m_document = doc;
}

QString ConfigManager::currentFolderForServer(const QString &serverUuid) const
{
    QJsonObject server = getServer(serverUuid);
    return server.value("current_folder").toString();
}
