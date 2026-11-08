#include "fmatrayicon.h"
#include "fmabinder.h"

FmaTrayIcon::FmaTrayIcon(QObject *parent)
    : QSystemTrayIcon{parent}
{

}

void FmaTrayIcon::setBindVisible(bool on)
{
    if(isVisible() != on){
        if(on){
            InterlockedIncrement(&(FmaBinder::fmaWinRef));
            setVisible(true);
        }else{
            InterlockedDecrement(&(FmaBinder::fmaWinRef));
            setVisible(false);
        }
    }
}

void FmaTrayIcon::bindQuit()
{
    setBindVisible(false);
    if(FmaBinder::fmaWinRef == 0){
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }
}
