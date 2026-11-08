#include "fmaopwidget.h"
#include "fma/fmatextformats.h"
#include <QGridLayout>
#include <QtMath>
#include <QPushButton>
#include <QToolButton>
#include <QFileInfo>
#include <QMouseEvent>
#include <QApplication>

#define FMA_POPLAG Qt::WindowDoesNotAcceptFocus|Qt::Popup|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint
//#define FMA_POPLAG Qt::WindowDoesNotAcceptFocus|Qt::Popup|Qt::WindowStaysOnTopHint

FmaOpWidget::FmaOpWidget(const QStringList &items, int colNum, QWidget *parent) : QFrame(parent)
{
    // 弹层是独立窗口，必须自己填充主题背景，否则浅色主题下会是深色底 + 深色文字。
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);

    //setStyleSheet("border: 1px");
    mainLayout = new QGridLayout(this);
    mainLayout->setVerticalSpacing(1);
    mainLayout->setHorizontalSpacing(1);
    mainLayout->setContentsMargins(1, 1, 1, 1);

    setLayout(mainLayout);
    setOps(items, colNum);

    setWindowFlags(FMA_POPLAG);
    setAttribute(Qt::WA_MouseTracking, true);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);
}

void FmaOpWidget::onBtnClicked()
{
    QAbstractButton* btn = qobject_cast<QAbstractButton *>(sender());
    emit toOp(btn->text());
    close();
}

void FmaOpWidget::setOps(const QStringList &items, int colNum)
{
    int itemNum = items.size();
    int t = qFloor(qSqrt(itemNum));
    if(colNum <=0){
        colNum = qMax(t ,1);
    }
    int rowNum = itemNum/colNum;
    if(itemNum%colNum != 0){
        rowNum ++;
    }

    QFont tFont(FmaTextFormats::getDefaultFontFamily(), 13);

    int k= 0;
    for(int i=0; i<rowNum; ++i){
        for(int j=0; j<colNum; ++j){
            if(k>=itemNum){
                break;
            }

            const QString& t = items[k];
            if(t.length()>1){
                QToolButton* btn = new QToolButton(this);
                btn->setStyleSheet(QStringLiteral("QToolButton { padding: 0px; border: none; }"));
                btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
                btn->setIcon(QIcon(t));
                btn->setIconSize(QSize(28,28));
                btn->setText(QFileInfo(t).baseName());
                connect(btn, SIGNAL(clicked()), this, SLOT(onBtnClicked()));

                btn->setFixedWidth(28);
                btn->setFixedHeight(28);                
                mainLayout->addWidget(btn, i, j);
            }else{
                QPushButton* btn = new QPushButton(t, this);
                btn->setStyleSheet(QStringLiteral("QPushButton { padding: 0px; }"));
                btn->setFont(tFont);
                connect(btn, SIGNAL(clicked()), this, SLOT(onBtnClicked()));

                btn->setFixedWidth(28);
                btn->setFixedHeight(28);
                mainLayout->addWidget(btn, i, j);
            }
            k++;
        }
        if(k>=itemNum){
            break;
        }
    }
}


bool FmaOpWidget::mouseEventTaken(QMouseEvent *e)
{
    if (frameGeometry().contains(e->globalPosition().toPoint())){
        return false; //otherwise if the event is in our rect we want it..
    }

    if(popCause) {
        QPoint cpos = popCause->mapFromGlobal(e->globalPosition().toPoint());
        if (popCause->rect().contains(cpos)) {
            if (e->type() != QEvent::MouseButtonRelease) {
                QMouseEvent new_e(e->type(), cpos, popCause->mapTo(popCause->topLevelWidget(), cpos), e->globalPosition(),
                                  e->button(), e->buttons(), e->modifiers(), e->source());
                QApplication::sendEvent(popCause, &new_e);
                return true;
            }
        }
    }
    return false;
}

void FmaOpWidget::mousePressEvent(QMouseEvent *e)
{
    if(mouseEventTaken(e)){
        return;
    }
    QWidget::mousePressEvent(e);
}

void FmaOpWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(mouseEventTaken(e)){
        return;
    }

    QWidget::mousePressEvent(e);
}

void FmaOpWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mouseEventTaken(e)){
        return;
    }
    QWidget::mouseMoveEvent(e);
}
