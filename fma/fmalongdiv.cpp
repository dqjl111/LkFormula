#include "fmalongdiv.h"
#include <QPainter>
#include "wmf/fmawmfpaintengine.h"

FmaLongdiv::FmaLongdiv(IFmaBase *child):FmaBuddyBase(child)
{

}

FmaLongdiv::FmaLongdiv(const QCborMap &cborMap):FmaBuddyBase(cborMap)
{

}

FmaType FmaLongdiv::type()
{
    return FmaType::mlongdiv;
}

void FmaLongdiv::paint(QPainter *painter)
{
    for (IFmaBase* eb : children)
    {
        eb->paint(painter);
    }

    qreal thick = lineThickness(1.5);

    painter->save();
    QPen pen = painter->pen();
    pen.setWidthF( thick );
    painter->setPen( pen );

    QPointF p0(rect.x()+thick, rect.y()+thick);
    QPointF p1(rect.x()+rect.width()-thick, rect.y()+thick);

    painter->drawLine(p0, p1);
    qreal xtrude = rect.width() -children[0]->fullWidth()-thick*4;
    QRectF arcRect(rect.x()-xtrude, rect.y()+thick, xtrude*2, rect.height());
    if(painter->paintEngine()->type() >= QPaintEngine::User){
        FmaWmfPaintEngine* wmfPaintEngine = (FmaWmfPaintEngine*)(painter->paintEngine());
        wmfPaintEngine->drawArc(arcRect, 90*16, -180*16);
    }else{
        painter->drawArc(arcRect, 90*16, -180*16);
    }
    painter->restore();
}

void FmaLongdiv::calculatePos()
{
    qreal x = rect.x();
    qreal y = rect.y();

    qreal thick = lineThickness(3);

    children[0]->rect.moveLeft(x+rect.width()-children[0]->fullWidth()-thick);
    children[0]->rect.moveTop(y+thick);

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaLongdiv::upHeight()
{
    qreal upH = children[0]->upHeight();
    upH+=lineThickness(3);
    return upH;
}

void FmaLongdiv::calculateSize()
{
    IFmaContainer::calculateSize();

    qreal baseHeight = children[0]->fullHeight() + lineThickness(3);
    rect.setHeight(baseHeight + baseHeight*0.2);
    rect.setWidth(children[0]->fullWidth() + baseHeight*0.5);
}
