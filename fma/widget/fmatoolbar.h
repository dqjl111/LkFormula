#ifndef FMATOOLBAR_H
#define FMATOOLBAR_H

#include <QWidget>
#include "fmaopsbutton.h"

class FmaToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit FmaToolBar(QWidget *parent = nullptr);
    void addWatch(FmaOpsButton* btn);
signals:
    void toOp(const QString& op);
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;

protected:
    QList<FmaOpsButton*> watchList;
};

#endif // FMATOOLBAR_H
