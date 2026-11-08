#ifndef FMATRAYICON_H
#define FMATRAYICON_H

#include <QSystemTrayIcon>

class FmaTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit FmaTrayIcon(QObject *parent = nullptr);

    void setBindVisible(bool isOn);

signals:

public slots:
    void bindQuit();
};

#endif // FMATRAYICON_H
