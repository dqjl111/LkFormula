#include "fmatoolbar.h"
#include <QMouseEvent>

FmaToolBar::FmaToolBar(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
}

void FmaToolBar::addWatch(FmaOpsButton *btn)
{
    watchList.append(btn);
    btn->setOpsPopCause(this);
    btn->setFont(font());
    connect(btn, SIGNAL(toOp(QString)), this, SIGNAL(toOp(QString)));
}


void FmaToolBar::mouseMoveEvent(QMouseEvent *event)
{
    FmaOpsButton* visibleOps = nullptr;
    for(FmaOpsButton* w: watchList){
        if(w->isOpsVisible()){
            visibleOps = w;
            break;
        }
    }

    if(visibleOps){
        QPoint p = event->globalPosition().toPoint();
        for(FmaOpsButton* w: watchList){
            if(w->rect().contains(w->mapFromGlobal(p))){
                if(w != visibleOps){
                    visibleOps->closePop();
                    w->setFocus(Qt::MouseFocusReason);
                    w->openPop();
                }
                return;
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}
