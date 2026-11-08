#ifndef FMABINDER_H
#define FMABINDER_H

#ifdef _WIN32

#include <QObject>
#include <QAxBindable>
#include "fma/fmadoc.h"
#include "fmamain.h"


struct IOleClientSite;
class FmaAggregated;
class FmaTrayIcon;
class FmaBinder : public QObject
        , public QAxBindable
{
    Q_OBJECT
    Q_CLASSINFO("ClassID",     _FMA_CLASSID)
    Q_CLASSINFO("InterfaceID", "{3CF618CE-AB14-4188-906E-0AA773C0FC47}")
    Q_CLASSINFO("RegisterObject", "yes")
    Q_CLASSINFO("Insertable", "yes")

    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
public:
    explicit FmaBinder(QObject *parent = nullptr);
    virtual ~FmaBinder();
    virtual QAxAggregated *createAggregate();
    bool event(QEvent *e) override;

    FmaMain *window();
    std::shared_ptr<FmaDoc> doc();

    QString id() const { return objectName(); }

    void setVisible(bool on, bool forceFront = true);
    bool isVisible();

    IOleClientSite* oleClientSite() const;

signals:
//    void toBindShow();
public slots:
    void bindQuit();
    void slotTrayActivated(QSystemTrayIcon::ActivationReason);
private:
    QScopedPointer <FmaMain> m_ui;
    FmaAggregated* aggergated = nullptr;
    std::shared_ptr<FmaDoc> docPtr = nullptr;
    static LONG fmaWinRef;


    friend class FmaAggregated;
    friend class FmaTrayIcon;

    QString buildClassName();

#ifndef _NO_FMA_TRAY
    void initTray();
#endif
    void initG();

//    void setTrayVisible(bool on);
};
#endif

#endif // FMAOLEDOCH_H
