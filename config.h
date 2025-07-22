#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>

class Config {
public:
    static Config& instance();
    
    // 获取配置路径
    QString getPythonPath() const;
    QString getFloorplanWorkingDirectory() const;
    QString getTextureWorkingDirectory() const;
    QString getVcpkgPath() const;
    
    // 设置配置路径
    void setPythonPath(const QString& path);
    void setFloorplanWorkingDirectory(const QString& path);
    void setTextureWorkingDirectory(const QString& path);
    void setVcpkgPath(const QString& path);
    
    // 配置文件操作
    bool loadConfig();
    bool saveConfig();
    QString getConfigFilePath() const;
    
    // 智能路径查找
    QString findPythonExecutable() const;
    QString findWorkingDirectory(const QString& relativePath) const;

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    QJsonObject m_config;
    bool m_loaded = false;
    
    void loadDefaults();
    QString findExecutableInPath(const QString& executable) const;
};

#endif // CONFIG_H