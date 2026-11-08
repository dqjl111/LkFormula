#ifdef _WIN32

#include "fmabinder.h"
#include "fmaaggregated.h"
#include "util/nodeleter.h"
#include "fmamain.h"
#include "util/lkutil.h"
#include "util/pathconfig.h"
#include <QAxFactory>
#include <QAction>
#include <QMenu>
#include <QDataStream>
#include <QBuffer>
#include <QMessageBox>
#include <QTextCodec>
#include <QEvent>
#include "util/lktranslator.h"
#include "fmatrayicon.h"

LONG FmaBinder::fmaWinRef=0;
static std::atomic<bool> hasInitG(false);

#ifndef _NO_FMA_TRAY
static std::shared_ptr<FmaTrayIcon> trayIcon = nullptr;
static std::shared_ptr<QMenu> trayMenu = nullptr;
static QAction* exitAct = nullptr;
static std::atomic<bool> hasInitTray(false);
#endif

FmaBinder::FmaBinder(QObject *parent): QObject(parent)
{
    setObjectName(QStringLiteral("From QAxFactory"));
    LkTranslator::addLanguageObserver(this);
    docPtr = std::make_shared<FmaDoc>();
    initG();
}

FmaBinder::~FmaBinder()
{
    //qWarning() << "Release binder";
}

bool FmaBinder::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
#ifndef _NO_FMA_TRAY
        if (exitAct)
            exitAct->setText(tr("Exit"));
        if (trayIcon)
            trayIcon->setToolTip(tr("Fma Equation"));
#endif
    }
    return QObject::event(e);
}

void FmaBinder::slotTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger :
    case QSystemTrayIcon::DoubleClick :{
        setVisible(true, false);
        break;
    }
    default:
        break;
    }
}


QString FmaBinder::buildClassName()
{
    return QCoreApplication::applicationName() +  "FmaBinder.1";
}

void FmaBinder::initG()
{
    if(!hasInitG.exchange(true)){
        QString basedir_str = QCoreApplication::applicationDirPath();
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(utf8);

        PathConfig::initPath(basedir_str + QDir::separator());
        qInstallMessageHandler(LkUtil::outputMessage);
    }
}


QAxAggregated *FmaBinder::createAggregate()
{
    if(aggergated){
        return nullptr;
    }else{
        aggergated = new FmaAggregated;
        return aggergated;
    }
}

FmaMain *FmaBinder::window()
{
    if(m_ui.isNull()){
        m_ui.reset(new FmaMain(docPtr));
        connect(m_ui.data(), SIGNAL(bindQuit()), this, SLOT(bindQuit()));
    }
    return m_ui.get();
}

std::shared_ptr<FmaDoc> FmaBinder::doc()
{
    return docPtr;
}

void FmaBinder::setVisible(bool on, bool forceFront)
{
    if(isVisible() != on){
        if(on){
#ifndef _NO_FMA_TRAY
            initTray();
#endif
            InterlockedIncrement(&fmaWinRef);
            //qWarning() << "Increment: " << binderRunRef;
            FmaMain *w = window();
            if(forceFront){
                w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
            }
            w->bindShow();
        }else{
            InterlockedDecrement(&fmaWinRef);
            //qWarning() << "Decrement: " << binderRunRef;
            window()->setVisible(false);
        }
    }
}

bool FmaBinder::isVisible()
{
    if(m_ui.isNull()){
        return false;
    }

    return window()->isVisible();
}

IOleClientSite *FmaBinder::oleClientSite() const
{
    return dynamic_cast<IOleClientSite *>(clientSite());
}

void FmaBinder::bindQuit()
{
    //qWarning() << "bind quit";
    if(aggergated){
        if(!m_ui.isNull()){
            setVisible(false);
        }

        aggergated->Close(OLECLOSE_SAVEIFDIRTY);

        if(fmaWinRef == 0){
            QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        }
    }
}

#ifndef _NO_FMA_TRAY
void FmaBinder::initTray()
{
    if(!hasInitTray.exchange(true)){
        trayIcon = std::make_shared<FmaTrayIcon>();
        trayMenu = std::make_shared<QMenu>();
        exitAct = new QAction(tr("Exit"), trayMenu.get());
        trayIcon->setIcon(QIcon(":/icon.ico"));
        trayIcon->setToolTip(tr("Fma Equation"));
        trayMenu->addAction(exitAct);
        trayIcon->setContextMenu(trayMenu.get());

        connect(exitAct, SIGNAL(triggered()), trayIcon.get(), SLOT(bindQuit()));
    }

    if(trayIcon){
        trayIcon->setBindVisible(true);
        connect(trayIcon.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(slotTrayActivated(QSystemTrayIcon::ActivationReason)));
    }
    if(exitAct){
        connect(exitAct, SIGNAL(triggered()), this, SLOT(bindQuit()));
    }

}
#endif
#endif
