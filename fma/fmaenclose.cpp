#include "fmaenclose.h"
#include "fmarow.h"
#include <QPainter>
#include <QPaintEngine>
#include "wmf/fmawmfpaintengine.h"

FmaEnclose::FmaEnclose(int _notation, IFmaBase* child, bool withQuotient)
    :FmaBuddyBase(child)
{
    notation = _notation;

    if(notation == LongDiv && withQuotient){
        // 商位行：空行由 FmaRow::paint 画虚线框，可点击进入输入商。
        FmaRow* quotient = new FmaRow;
        quotient->parent = this;
        children.append(quotient);
        setLevelOffset(quotient, 0);
    }
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
        // 商位在顶部（children[1]），横线画在商位下方、被除数上方。
        const qreal quotientH = (children.size() > 1) ? children[1]->fullHeight() : 0;
        const qreal lineY = rect.y() + thick + quotientH;
        painter->drawLine(QPointF(rect.x()+thick, lineY),
                          QPointF(rect.x()+rect.width()-thick, lineY));

        // 左侧弧线：只包住被除数部分（横线以下），
        // 避免弧线在商位左上方多出一段。
        const qreal xtrude = thick * 1.5;
        QRectF arcRect(rect.x()-xtrude, lineY, xtrude*2,
                       qMax(1.0, rect.bottom() - thick - lineY));

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
        const qreal quotientH = (children.size() > 1) ? children[1]->fullHeight() : 0;
        if(children.size() > 1){
            // 商位：顶部，宽度取商内容与被除数较宽者，避免内容溢出、横线过短。
            const qreal quotientW = qMax(children[1]->fullWidth(), children[0]->fullWidth());
            children[1]->rect.setWidth(quotientW);
            children[1]->rect.setHeight(quotientH);
            children[1]->rect.moveLeft(x + thick);
            children[1]->rect.moveTop(y + thick);
        }
        children[0]->rect.moveLeft(x+rect.width()-children[0]->fullWidth()-thick);
        children[0]->rect.moveTop(y + thick + quotientH);
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
        if(children.size() > 1){
            upH += children[1]->fullHeight();  // 商位
        }
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
        const qreal thick = lineThickness(3);
        const qreal contentH = children[0]->fullHeight();
        // 无商位时高度不再额外加一行，避免框线过长。
        const qreal quotientH = (children.size() > 1) ? children[1]->fullHeight() : 0.0;
        // 宽度取商位与被除数中较宽者，横线才能跟随内容变长。
        const qreal contentW = (children.size() > 1)
                ? qMax(children[0]->fullWidth(), children[1]->fullWidth())
                : children[0]->fullWidth();
        rect.setWidth(contentW + thick * 3);
        rect.setHeight(quotientH + contentH + thick * 2);
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
        if(notation & Right){
            // 右边线单独加大间距，避免框的右边线贴到内容上。
            width += thick * 2.5;
        }else if((notation & Horizontalstrike)
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


