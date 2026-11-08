#include "pathconfig.h"
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>

QString PathConfig::config = QStringLiteral("config.ini");
QString PathConfig::baseDir;
QString PathConfig::logDir;

PathConfig::PathConfig()
{

}

const QString PathConfig::getAbsoluteBase()
{
    return baseDir;
}

const QString PathConfig::getConfigFile()
{
    return baseDir + config;
}

const QString PathConfig::getAppDir()
{
    return QCoreApplication::applicationDirPath();  //Return bin directory
}

const QString PathConfig::getModuleDir()
{
    return getLibDir() + QDir::separator() + "module";
}

const QString PathConfig::getLibDir()
{
#ifdef __WIN32
    return QCoreApplication::applicationDirPath();
#else
    return QFileInfo(QCoreApplication::applicationDirPath()).dir().path() + QDir::separator() + "lib";
#endif
}

const QString PathConfig::getInitDBDir()
{
#ifdef __WIN32
    return  QCoreApplication::applicationDirPath()  + QDir::separator() + "initdb" + QDir::separator();
#else
    return QFileInfo(QCoreApplication::applicationDirPath()).dir().path() + QDir::separator() + "initdb" + QDir::separator();
#endif
}

void PathConfig::initPath(const QString _baseDir, const QString _logDir)
{
//    if(config.isEmpty()){
//        config = "config.ini";
//    }

    QString tempDirStr;
    if(_baseDir.isEmpty())
    {
        tempDirStr = QCoreApplication::applicationDirPath() ;
    }
    else
    {
        tempDirStr = _baseDir;
    }
    tempDirStr.replace("\\", QDir::separator());
    tempDirStr.replace("/", QDir::separator());
    if(!tempDirStr.endsWith(QDir::separator()))
    {
        tempDirStr += QDir::separator();
    }

    QDir dir;
    dir.setPath(tempDirStr);
    if( !dir.exists() )
    {
        dir.mkpath(tempDirStr);
    }
    baseDir = tempDirStr;

    if(_logDir.isEmpty()){
        tempDirStr = baseDir + "log" + QDir::separator();
    }else{
        tempDirStr= _logDir;
        tempDirStr.replace("\\", QDir::separator());
        tempDirStr.replace("/", QDir::separator());
        if(!tempDirStr.endsWith(QDir::separator()))
        {
            tempDirStr += QDir::separator();
        }
    }

    dir.setPath(tempDirStr);
    if( !dir.exists() )
    {
        dir.mkpath(tempDirStr);
    }
    logDir = tempDirStr;

    std::cout << "baseDir: "<<baseDir.toStdString() << std::endl;
    std::cout <<  "logDir: " << logDir.toStdString() << std::endl;
}

