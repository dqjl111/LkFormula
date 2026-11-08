#include "fmaenclose.h"
#include <QPainter>
#include <QPaintEngine>
#include "wmf/fmawmfpaintengine.h"

FmaEnclose::FmaEnclose(int _notation, IFmaBase* child):FmaBuddyBase(child)
{
    notation = _notation;
}

FmaEnclose::FmaEnclose(const QCborMap &cborMap):FmaBuddyBase(cborMap)
{
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::Notation));
    if(iter != cborMap.end()){
        notation = iter.value().toInteger(int(LongDiv));
    }
}

FmaType FmaEnclose::type()
{
    return FmaType::menclose;
}

void FmaEnclose::paint(QPainter *painter)
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

    if(notation == LongDiv){
        painter->drawLine(p0, p1);
        qreal xtrude = rect.width() -children[0]->fullWidth()-thick*4;
        QRectF arcRect(rect.x()-xtrude, rect.y()+thick, xtrude*2, rect.height());

        if(painter->paintEngine()->type() >= QPaintEngine::User){
            FmaWmfPaintEngine* wmfPaintEngine = (FmaWmfPaintEngine*)(painter->paintEngine());
            wmfPaintEngine->drawArc(arcRect, 90*16, -180*16);
        }else{
            painter->drawArc(arcRect, 90*16, -180*16);
        }
    }else{
        QPointF p2(rect.x()+rect.width()-thick, rect.y()+rect.height()-thick);
        QPointF p3(rect.x()+thick, rect.y()+rect.height()-thick);

        if(notation & Top){
            painter->drawLine(p0, p1);
        }

        if(notation & Right){
            painter->drawLine(p1, p2);
        }

        if(notation & Bottom){
            painter->drawLine(p2, p3);
        }

        if(notation & Left){
            painter->drawLine(p3, p0);
        }

        if(notation & Updiagonalstrike){
            painter->drawLine(p3, p1);
        }

        if(notation & Downdiagonalstrike){
            painter->drawLine(p0, p2);
        }

        if(notation & Horizontalstrike){
            qreal y = rect.y()+0.5*rect.height();
            painter->drawLine(p0.x(), y, p1.x(), y);
        }

        if(notation & Verticalstrike){
            qreal x = rect.x()+0.5*rect.width();
            painter->drawLine(x, p0.y(), x, p3.y());
        }
    }
    painter->restore();
}

void FmaEnclose::calculatePos()
{
    qreal x = rect.x();
    qreal y = rect.y();

    qreal thick = lineThickness(3);

    if(notation == LongDiv){
        children[0]->rect.moveLeft(x+rect.width()-children[0]->fullWidth()-thick);
        children[0]->rect.moveTop(y+thick);
    }else{
        if(notation & Left
                || (notation & Horizontalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            x+=thick;
        }

        if(notation & Top
                || (notation & Verticalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            y+=thick;
        }
        children[0]->rect.moveLeft(x);
        children[0]->rect.moveTop(y);
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaEnclose::upHeight()
{
    qreal upH = children[0]->upHeight();
    if(notation == LongDiv){
        upH+=lineThickness(3);
    }else{
        if(notation & Top
                || (notation & Verticalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            upH+=lineThickness(3);
        }
    }
    return upH;
}

QCborMap FmaEnclose::toCbor(bool filterSelect)
{
    QCborMap ret = FmaBuddyBase::toCbor(filterSelect);
    ret.insert(int(FmaCborId::Notation), notation);
    return ret;
}

QString FmaEnclose::notationString(int notation)
{
    if(notation == LongDiv){
        return "longdiv";
    }else{
        QStringList notations;
        if(notation & Top){
            notations.append("top");
        }
        if(notation & Bottom){
            notations.append("bottom");
        }
        if(notation & Left){
            notations.append("left");
        }
        if(notation & Right){
            notations.append("right");
        }
        if(notation & Updiagonalstrike){
            notations.append("updiagonalstrike");
        }
        if(notation & Downdiagonalstrike){
            notations.append("downdiagonalstrike");
        }
        if(notation & Horizontalstrike){
            notations.append("horizontalstrike");
        }
        if(notation & Verticalstrike){
            notations.append("verticalstrike");
        }
        return notations.join(" ");
    }
}

int FmaEnclose::stringToNotation(const QString &str)
{
    QStringList strList = str.split(" ");
    int ret = 0;
    for(QString nStr : strList ){
        nStr = nStr.trimmed().toLower();
        if(nStr == "longdiv"){
            return LongDiv;
        }else{
            QStringList notations;
            if(nStr == "top"){
                ret |= Top;
            }else if(nStr == "bottom"){
                ret |= Bottom;
            }else if(nStr == "left"){
                ret |= Left;
            }else if(nStr == "right"){
                ret |= Right;
            }else if(nStr == "updiagonalstrike"){
                ret |= Updiagonalstrike;
            }else if(nStr == "downdiagonalstrike"){
                ret |= Downdiagonalstrike;
            }else if(nStr == "horizontalstrike"){
                ret |= Horizontalstrike;
            }else if(nStr == "verticalstrike"){
                ret |= Verticalstrike;
            }
        }
    }
    return ret;
}

void FmaEnclose::calculateSize()
{
    IFmaContainer::calculateSize();

    if(notation == LongDiv){
        qreal baseHeight = children[0]->fullHeight() + lineThickness(3);
        rect.setHeight(baseHeight + baseHeight*0.1);
        rect.setWidth(children[0]->fullWidth() + baseHeight*0.5);
    }else{
        qreal thick = lineThickness(3);
        qreal width = children[0]->fullWidth();
        qreal height = children[0]->fullHeight();

        if((notation & Left)
                || (notation & Horizontalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            width+=thick;
        }
        if(notation & Right
                || (notation & Horizontalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            width+=thick;
        }
        if(notation & Top
                || (notation & Verticalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            height+=thick;
        }
        if(notation & Bottom
                || (notation & Verticalstrike)
                || (notation & Updiagonalstrike)
                || (notation & Downdiagonalstrike)){
            height+=thick;
        }

        rect.setWidth(width);
        rect.setHeight(height);
    }
}


