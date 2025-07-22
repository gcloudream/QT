#include "config.h"
#include <QApplication>
#include <QJsonParseError>
#include <QTextStream>

Config& Config::instance() {
    static Config instance;
    if (!instance.m_loaded) {
        instance.loadConfig();
    }
    return instance;
}

QString Config::getPythonPath() const {
    QString path = m_config.value("pythonPath").toString();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        // 尝试自动查找Python
        path = const_cast<Config*>(this)->findPythonExecutable();
        if (!path.isEmpty()) {
            const_cast<Config*>(this)->setPythonPath(path);
        }
    }
    return path;
}

QString Config::getFloorplanWorkingDirectory() const {
    QString path = m_config.value("floorplanWorkingDirectory").toString();
    if (path.isEmpty()) {
        path = findWorkingDirectory("floorplan_code_v1/py_script");
    }
    return path;
}

QString Config::getTextureWorkingDirectory() const {
    QString path = m_config.value("textureWorkingDirectory").toString();
    if (path.isEmpty()) {
        path = findWorkingDirectory("floorplan_code_v1/texture");
    }
    return path;
}

QString Config::getVcpkgPath() const {
    QString path = m_config.value("vcpkgPath").toString();
    if (path.isEmpty()) {
        // 尝试常见的vcpkg路径
        QStringList candidates = {
            "C:/vcpkg/installed/x64-windows",
            "C:/Users/" + qgetenv("USERNAME") + "/vcpkg/installed/x64-windows",
            "D:/vcpkg/installed/x64-windows"
        };
        
        for (const QString& candidate : candidates) {
            if (QDir(candidate + "/include").exists()) {
                const_cast<Config*>(this)->setVcpkgPath(candidate);
                return candidate;
            }
        }
    }
    return path;
}

void Config::setPythonPath(const QString& path) {
    m_config["pythonPath"] = path;
    saveConfig();
}

void Config::setFloorplanWorkingDirectory(const QString& path) {
    m_config["floorplanWorkingDirectory"] = path;
    saveConfig();
}

void Config::setTextureWorkingDirectory(const QString& path) {
    m_config["textureWorkingDirectory"] = path;
    saveConfig();
}

void Config::setVcpkgPath(const QString& path) {
    m_config["vcpkgPath"] = path;
    saveConfig();
}

bool Config::loadConfig() {
    QString configPath = getConfigFilePath();
    
    if (!QFile::exists(configPath)) {
        loadDefaults();
        saveConfig();
        return true;
    }
    
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << configPath;
        loadDefaults();
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "配置文件JSON解析错误:" << error.errorString();
        loadDefaults();
        return false;
    }
    
    m_config = doc.object();
    m_loaded = true;
    
    qDebug() << "配置文件加载成功:" << configPath;
    return true;
}

bool Config::saveConfig() {
    QString configPath = getConfigFilePath();
    
    // 确保目录存在
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法保存配置文件:" << configPath;
        return false;
    }
    
    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    
    qDebug() << "配置文件保存成功:" << configPath;
    return true;
}

QString Config::getConfigFilePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(dataPath).filePath("config.json");
}

QString Config::findPythonExecutable() const {
    QStringList candidates = {
        "python",
        "python3",
        "python.exe",
        "python3.exe"
    };
    
    // 首先在PATH中查找
    for (const QString& candidate : candidates) {
        QString path = findExecutableInPath(candidate);
        if (!path.isEmpty()) {
            return path;
        }
    }
    
    // Windows特定路径查找
#ifdef Q_OS_WIN
    QStringList windowsPaths = {
        "C:/Python*/python.exe",
        "C:/Users/*/AppData/Local/Programs/Python/Python*/python.exe",
        "C:/Program Files/Python*/python.exe"
    };
    
    for (const QString& pattern : windowsPaths) {
        // 简化的通配符搜索
        QDir dir("C:/");
        if (pattern.contains("Users")) {
            QString userPath = QString(pattern).replace("*", qgetenv("USERNAME"));
            if (QFile::exists(userPath)) {
                return userPath;
            }
        }
    }
#endif
    
    return QString();
}

QString Config::findWorkingDirectory(const QString& relativePath) const {
    // 搜索可能的工作目录位置
    QStringList basePaths = {
        QDir::currentPath(),
        QApplication::applicationDirPath(),
        "C:/SLAM",
        "D:/SLAM",
        QDir::homePath() + "/Documents"
    };
    
    for (const QString& basePath : basePaths) {
        QString fullPath = QDir(basePath).filePath(relativePath);
        if (QDir(fullPath).exists()) {
            return fullPath;
        }
    }
    
    return QString();
}

void Config::loadDefaults() {
    m_config = QJsonObject();
    
    // 设置默认值
    m_config["pythonPath"] = "";
    m_config["floorplanWorkingDirectory"] = "";
    m_config["textureWorkingDirectory"] = "";
    m_config["vcpkgPath"] = "";
    
    // 添加配置说明
    m_config["_description"] = "Qt点云处理应用程序配置文件";
    m_config["_version"] = "1.0";
    
    qDebug() << "已加载默认配置";
}

QString Config::findExecutableInPath(const QString& executable) const {
    QProcess process;
    
#ifdef Q_OS_WIN
    process.start("where", QStringList() << executable);
#else
    process.start("which", QStringList() << executable);
#endif
    
    if (process.waitForFinished(3000) && process.exitCode() == 0) {
        QString output = process.readAllStandardOutput().trimmed();
        QStringList lines = output.split('\n');
        if (!lines.isEmpty()) {
            return lines.first().trimmed();
        }
    }
    
    return QString();
}