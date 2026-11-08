#include "fmafrac.h"
#include "fmarow.h"
#include "fmatext.h"
#include "fmadoc.h"
#include <QPainter>
#include <QDebug>

FmaFrac::FmaFrac()
{
    children.append(new FmaRow);
    children.append(new FmaRow);
    children[0]->parent = this;
    children[1]->parent = this;
}

FmaFrac::FmaFrac(const QCborMap &cborMap):IFmaContainer(cborMap, 2)
{
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::Bevelled));
    if(iter != cborMap.end()){
        bevelled = iter.value().toBool();
    }
}

FmaType FmaFrac::type()
{
    return FmaType::mfrac;
}

void FmaFrac::calculateSize()
{
    IFmaContainer::calculateSize();

    if(bevelled){
        qreal thick = lineThickness(5);
        qreal width = children[0]->fullWidth() + children[1]->fullWidth() + thick;
        rect.setWidth(width);
        rect.setHeight(children[0]->fullHeight() + children[1]->fullHeight()-2*thick);
    }else{
        qreal thick = lineThickness(5);
        qreal width = qMax(children[0]->fullWidth(), children[1]->fullWidth());
        rect.setWidth(width + thick);
        rect.setHeight(children[0]->fullHeight() + children[1]->fullHeight() + thick);
    }
}

void FmaFrac::calculatePos()
{
    if(bevelled){

        {
            IFmaBase* up = children[0];
            up->rect.moveLeft(rect.x());
            up->rect.moveTop(rect.y() + up->marginTop());
        }
        {
            IFmaBase* down = children[1];
            down->rect.moveLeft(rect.x()+rect.width()
                                -down->rect.width()
                                -down->marginRight());
            down->rect.moveTop(rect.y()+rect.height()
                               -down->rect.height()
                               -down->marginBottom());
        }
    }else{
        qreal center = rect.x() + rect.width()/2;
        {
            IFmaBase* up = children[0];
            up->rect.moveLeft(center - up->rect.width()/2);
            up->rect.moveTop(rect.y() + up->marginTop());
        }

        {
            IFmaBase* down = children[1];
            down->rect.moveLeft(center - down->rect.width()/2);
            down->rect.moveTop(rect.y()+ fullHeight()-down->marginBottom()-down->rect.height());
        }
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

void FmaFrac::paint(QPainter *painter)
{
    for (IFmaBase* eb : children)
    {
        eb->paint(painter);
    }
    qreal thick = lineThickness();
    painter->save();
    QPen pen = painter->pen();
    pen.setWidthF( thick );
    painter->setPen( pen );

    if(bevelled){
        qreal cx = children[0]->rect.x()
                + children[0]->rect.width()
                + children[0]->marginRight()
                + thick*2.5;

        qreal overlapH = children[0]->fullHeight() + children[1]->fullHeight() - rect.height();

        qreal upH = children[0]->rect.height()
                + children[0]->marginBottom()
                -thick*2.5 - overlapH/2;
        qreal downH = children[1]->rect.height() + children[1]->marginTop()-thick*2.5 - overlapH/2;

        qreal upX = cx + upH*0.5;
        qreal upY = 0;
        if(upX > rect.x() + rect.width()){
            upX = rect.x()+rect.width();
            upY = children[0]->rect.y() + upH - (upX-cx)*2;
        }else{
            upY = children[0]->rect.y();
        }
        qreal downX = cx -downH*0.5;
        qreal downY = 0;
        if(downX < rect.x()){
            downX = rect.x();
            downY = children[1]->rect.y()+children[1]->rect.height() - downH + (cx-downX)*2;
        }else{
            downY = children[1]->rect.y()+children[1]->rect.height();
        }

        painter->drawLine( QPointF(upX, upY), QPointF(downX, downY));
    }else{
        qreal y = rect.y() + children[0]->fullHeight();
        painter->drawLine( QPointF( rect.left()+thick*1.5, y), QPointF( rect.right()-thick*1.5, y) );
    }
    painter->restore();
}

qreal FmaFrac::upHeight()
{
    return children[0]->fullHeight();
}

void FmaFrac::setUp(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setUp(new FmaText(type, text, formFix));
}

void FmaFrac::setUp(IFmaBase *up)
{
    if(setToRowChild(up, 0)){
        setLevelOffset(children[0], 0);
    }
}

FmaRow *FmaFrac::getUp()
{
    return (FmaRow *)children[0];
}

void FmaFrac::setDown(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setDown(new FmaText(type, text, formFix));
}

void FmaFrac::setDown(IFmaBase *down)
{
    if(setToRowChild(down, 1)){
        setLevelOffset(children[1], 0);
    }
}

FmaRow *FmaFrac::getDown()
{
    return (FmaRow *)children[1];
}

bool FmaFrac::moveCursor(QTextCursor::MoveOperation op)
{
    switch(op){
    case QTextCursor::Up:
    case QTextCursor::Down:{
        if(children.isEmpty()){
            return false;
        }
        if(isValid(activeChild)){
            if(activeChild->moveCursor(op)){
                return true;
            }
        }

        IFmaBase *t;
        if(op == QTextCursor::Up){
            t = previousBlock();
        }else{
            t= nextBlock();
        }

        if(isValid(t) && t != activeChild){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::End);
        }
        break;
    }
    default:{
        break;
    }
    }
    return IFmaContainer::moveCursor(op);
}

QCborMap FmaFrac::toCbor(bool filterSelect)
{
    QCborMap ret = IFmaContainer::toCbor(filterSelect);
    ret.insert(int(FmaCborId::Bevelled), bevelled);
    return ret;
}
