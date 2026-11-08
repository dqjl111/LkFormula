#include "fmaroot.h"
#include "fmarow.h"
#include "fmatextformats.h"
#include "fmatext.h"
#include <QPainter>
#include <QDebug>

FmaSquareRoot::FmaSquareRoot(IFmaBase* buddy): FmaBuddyBase(buddy)
{
}

FmaSquareRoot::FmaSquareRoot(const QCborMap &cborMap): FmaBuddyBase(cborMap)
{

}

FmaSquareRoot::FmaSquareRoot(const QCborMap &cborMap, int limit): FmaBuddyBase(cborMap, limit)
{

}

FmaType FmaSquareRoot::type()
{
    return FmaType::msqrt;
}

void FmaSquareRoot::calculateSize()
{
    IFmaContainer::calculateSize();

    rect.setHeight(children[0]->fullHeight() + lineThickness());
    rect.setWidth(children[0]->fullWidth() + children[0]->fullHeight()*0.5);
}

void FmaSquareRoot::calculatePos()
{
    IFmaBase* buddy = children[0];

    buddy->rect.moveLeft(rect.x() + rect.width()-buddy->rect.width()-buddy->marginRight());
    buddy->rect.moveTop(rect.y() + rect.height()-buddy->rect.height()-buddy->marginBottom());

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

void FmaSquareRoot::paint(QPainter *painter)
{
    for (IFmaBase* eb : children)
    {
        eb->paint(painter);
    }

    qreal thick = lineThickness();
    qreal xr = children[0]->rect.x() - children[0]->marginLeft() - children[0]->fullHeight()*0.5;
    qreal yr = children[0]->rect.y() - children[0]->marginTop() - thick;
    qreal h = children[0]->fullHeight();
    qreal d = children[0]->fullHeight()*0.5;

    QPointF p0(xr, yr+h-d*0.24);
    QPointF p1(xr+d*0.1, yr+h-d*0.3);
    QPointF p2(xr+d*0.3, yr+h);
    QPointF p3(xr+d, yr);
    QPointF p4(p3.x()+children[0]->fullWidth(), yr);

    painter->save();
    QPen pen = painter->pen();
    pen.setWidthF( thick );
    painter->setPen( pen );
    painter->drawLines({p0,p1,p1,p2,p2,p3,p3,p4});
    painter->restore();
}

qreal FmaSquareRoot::upHeight()
{
    return fullHeight() - children[0]->fullHeight()*0.5;
}

qreal FmaSquareRoot::marginTop() const
{
    return FmaTextFormats::spacing(FmaTextFormats::Thinmathspace, scriptLevel())*0.5;
}


//////////////////
FmaRoot::FmaRoot(IFmaBase* buddy): FmaSquareRoot(buddy)
{
    children.append(new FmaRow);
    setLevelOffset(children[1], 1);
    children[1]->parent = this;
}

FmaRoot::FmaRoot(const QCborMap &cborMap): FmaSquareRoot(cborMap, 2)
{
    setLevelOffset(children[1], 1);
}

FmaType FmaRoot::type()
{
    return FmaType::mroot;
}

void FmaRoot::calculateSize()
{
    IFmaContainer::calculateSize();
    qreal halfHeight = (children[0]->fullHeight() + lineThickness())*0.5;
    rect.setHeight(halfHeight + qMax(children[1]->fullHeight(), halfHeight));

    qreal d = children[0]->fullHeight()*0.5;
    rect.setWidth(children[0]->fullWidth() +
            qMax(d, d*0.4 + children[1]->fullWidth()));
}

void FmaRoot::calculatePos()
{
    {
        IFmaBase* buddy = children[0];

        buddy->rect.moveLeft(rect.x() + rect.width()-buddy->rect.width()-buddy->marginRight());
        buddy->rect.moveTop(rect.y() + rect.height()-buddy->rect.height()-buddy->marginBottom());
    }
    {
        IFmaBase* power = children[1];
        power->rect.moveLeft(rect.x()+power->marginLeft());
        power->rect.moveTop(rect.y()+power->marginTop());
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

void FmaRoot::setPower(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setPower(new FmaText(type, text, formFix));
}

void FmaRoot::setPower(IFmaBase *power)
{
    if(setToRowChild(power, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaRoot::getPower()
{
    return (FmaRow *)children[1];
}

qreal FmaRoot::marginTop() const
{
    return FmaTextFormats::spacing(FmaTextFormats::Thinmathspace, scriptLevel())*0.5;
}
