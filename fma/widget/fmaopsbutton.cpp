#include "fmaopsbutton.h"
#include "fmaopwidget.h"
#include <QMenu>
#include <QWidgetAction>
#include <QDebug>

FmaOpsButton::FmaOpsButton(QWidget *parent) : QPushButton(parent)
{
}

FmaOpsButton::FmaOpsButton(const QString &text, QWidget *parent): QPushButton(text, parent)
{
}

FmaOpsButton::FmaOpsButton(const QIcon &icon, const QString &text, QWidget *parent)
: QPushButton(icon, text, parent)
{
}

FmaOpsButton::FmaOpsButton(const QString& text, const QStringList& ops, QWidget *parent)
    : QPushButton(parent)
{
    setText(text);
    setOps(ops);
}

FmaOpsButton::FmaOpsButton(const QString &text, const QStringList &ops, int colNum, QWidget *parent)
    : QPushButton(parent)
{
    setText(text);
    setOps(ops, colNum);
}

void FmaOpsButton::setOps(const QStringList &ops, int colNum)
{
    if(opTables){
        delete opTables;
        opTables = nullptr;
    }

    opTables = new FmaOpWidget(ops, colNum, this);
    opTables->popCause = this;
    connect(opTables, SIGNAL(toOp(QString)), this, SIGNAL(toOp(QString)));
    connect(this, SIGNAL(clicked()), this, SLOT(onTrigger()));
}

bool FmaOpsButton::isOpsVisible()
{
    if(opTables){
        return opTables->isVisible();
    }
    return false;
}

void FmaOpsButton::setOpsPopCause(QWidget *w)
{
    if(opTables){
        opTables->popCause = w;
    }
}

void FmaOpsButton::closePop()
{
    if(opTables){
        opTables->close();
    }
}

void FmaOpsButton::openPop()
{
    if(opTables){
        QPoint p = mapToGlobal(QPoint(0, height()));
        opTables->move(p);
        opTables->show();
    }
}

void FmaOpsButton::onTrigger()
{
    if(opTables){
        if(opTables->isVisible()){
            opTables->close();
        }else{
            QPoint p = mapToGlobal(QPoint(0, height()));
            opTables->move(p);
            opTables->show();
        }
    }
}

