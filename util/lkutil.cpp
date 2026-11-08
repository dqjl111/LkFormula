/*************************************************
 Copyright (C), 1988-2016, Xiamen Kehua Hengsheng Co.,Ltd.
 Version:     1.0
 Date:        20161006
 Description:
 Author:      linzhipeng
 Modifer:
*************************************************/
#include "lkutil.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include <iostream>
#include <QStringList>
#include <QProcess>
#include <stdio.h>
#include <QCryptographicHash>
#include "pathconfig.h"
#include <QtMath>
#include <QTextCodec>
#include <codecvt>
#include <locale>
#include <windows.h>

LkUtil::LkUtil(void)
{
}


LkUtil::~LkUtil(void)
{
}

void LkUtil::setStyleSheet(QWidget *widget, const QString& cssPath)
{
    QFile styleFile(cssPath);
    if(!styleFile.open(QFile::ReadOnly))
    {
        qWarning()<<"style file not open.";
    }
    else
    {
        QString style(styleFile.readAll());
        widget->setStyleSheet(style);
        styleFile.close();
    }
}

QString LkUtil::readRCFile(const QString &cssPath)
{
    QFile styleFile(cssPath);
    if(!styleFile.open(QFile::ReadOnly))
    {
        qWarning()<<"style file not open.";
    }
    else
    {
        QString style(styleFile.readAll());
        styleFile.close();
        return style;
    }
    return "";
}

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
void MyUtil::outputMessage(QtMsgType type, const char*  msg)
#else
void LkUtil::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#endif
{
#ifndef QT_DEBUG
    if(type == QtDebugMsg){
        return;
    }
#endif
    QString text;
    switch(type)
    {
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
    case QtInfoMsg:
        text = QString("Info:");
        break;
#endif

    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    default:
        break;
    }
    QDateTime dateTime = QDateTime::currentDateTime();
    QString currentDate = dateTime.toString("yyyy-MM-dd");

    QString currentTime = dateTime.toString("hh:mm:ss");
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QString message = QString("%0 %1 %2 %3").arg(currentDate).arg(currentTime).arg(text).arg(msg);
#else
#ifdef QT_DEBUG
    QString contextInfo = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
    QString message = QString("%0 %1 %2 %3 %4").arg(currentDate).arg(currentTime).arg(text).arg(contextInfo).arg(msg);
#else
    QString message = QString("%0 %1 %2 %3").arg(currentDate).arg(currentTime).arg(text).arg(msg);
#endif
#endif

    QFile file(PathConfig::logDir+currentDate+".txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream textStream(&file);
        textStream << message << '\n';
        file.flush();
        file.close();
    }else{
        std::cout << "Open log file failed!";
    }
#ifdef QT_DEBUG
#ifdef WIN32
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec* gbk = QTextCodec::codecForName("gbk");
    QString strUnicode= utf8->toUnicode(message.toLocal8Bit().data());
    std::cout << gbk->fromUnicode(strUnicode).data() << std::endl;
#else
    std::cout << message.toLocal8Bit().toStdString() << std::endl;
#endif
#endif
}


void LkUtil::clearPastLog(int keepDayNum)
{
    QDate date = QDate::currentDate();

    QDir logDir(QCoreApplication::applicationDirPath() + "/log/");
    QFileInfoList fileInfoList = logDir.entryInfoList();
    foreach(QFileInfo fileInfo , fileInfoList){
        QString fileName = fileInfo.baseName();
        QDate logDate = QDate::fromString(fileName, "yyyy-MM-dd");
        if(logDate.isValid()){
            if((date.daysTo(logDate) + keepDayNum) <= 0){
                QFile::remove(fileInfo.absoluteFilePath());
            }
        }else{
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

