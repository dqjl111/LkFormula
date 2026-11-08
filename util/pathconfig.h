#ifndef PATHCONFIG_H
#define PATHCONFIG_H
#include <QString>
#include <QFileInfo>
#include <memory>

class PathConfig
{
public:
    PathConfig();
    static const QString getAbsoluteBase();
    static const QString getConfigFile();

    static const QString getAppDir();
    static const QString getModuleDir();
    static const QString getLibDir();
    static const QString getInitDBDir();

    static void initPath(const QString _baseDir = "", const QString _logDir = "");

    static QString config;
    static QString baseDir;
    static QString logDir;
};

#endif // PATHCONFIG_H
