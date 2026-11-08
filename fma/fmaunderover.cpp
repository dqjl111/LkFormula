#include "fmaunderover.h"
#include "fmarow.h"
#include "fmatext.h"
#include <QDebug>

FmaUnder::FmaUnder(IFmaBase* buddy):FmaBuddyBase(buddy)
{
    children.append(new FmaRow);
    setLevelOffset(children[1], 1);
    children[1]->parent = this;
}

FmaUnder::FmaUnder(const QCborMap &cborMap):FmaBuddyBase(cborMap, 2)
{
    setLevelOffset(children[1], 1);
}

FmaType FmaUnder::type()
{
    return FmaType::munder;
}

void FmaUnder::calculateSize()
{
    IFmaContainer::calculateSize();

    spacing = lineThickness();
    if(((FmaRow*)children[1])->isSingleText()){
        FmaText * t=((FmaRow*)children[1])->rowFirstText();
        QFontMetricsF metrics = QFontMetricsF( t->font() );
        QRectF rect1 = metrics.tightBoundingRect(t->text);
        spacing += -(metrics.ascent() + rect1.top());
    }

    rect.setHeight(children[0]->fullHeight() + children[1]->fullHeight() + spacing);
    rect.setWidth(qMax(children[0]->fullWidth(), children[1]->fullWidth()));
}

void FmaUnder::calculatePos()
{
    qreal center = rect.x() + rect.width()/2;
    qreal y = rect.y();

    for(IFmaBase* eb : children){
        eb->rect.moveLeft(center - eb->rect.width()/2);
        eb->rect.moveTop(y+eb->marginTop());
        y+=eb->fullHeight() + spacing;
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaUnder::upHeight()
{
    return children[0]->fullHeight()*0.5;
}

void FmaUnder::setUnder(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setUnder(new FmaText(type, text, formFix));
}

void FmaUnder::setUnder(IFmaBase *under)
{
    if(setToRowChild(under, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaUnder::getUnder()
{
    return (FmaRow *)children[1];
}

bool FmaUnder::canStrechChild(IFmaBase *ch)
{
    if(ch == children[0]){
        return false;
    }

    if(ch->isDisplayStyle()){
        return false;
    }
    return true;
}
////////////////////////////
FmaOver::FmaOver(IFmaBase* buddy):FmaBuddyBase(buddy)
{
    children.append(new FmaRow);
    setLevelOffset(children[1], 1);
    children[1]->parent = this;
}

FmaOver::FmaOver(const QCborMap &cborMap):FmaBuddyBase(cborMap, 2)
{
    setLevelOffset(children[1], 1);
}

FmaType FmaOver::type()
{
    return FmaType::mover;
}

void FmaOver::calculateSize()
{
    IFmaContainer::calculateSize();

    //spacing = -lineThickness();
    spacing = 0;
    if(((FmaRow*)children[0])->isSingleText()){
        FmaText * t=((FmaRow*)children[0])->rowFirstText();
        QFontMetricsF metrics = QFontMetricsF( t->font() );
        QRectF rect1 = metrics.tightBoundingRect(t->text);
        spacing += -(metrics.ascent() + rect1.top());
    }

    rect.setHeight(children[0]->fullHeight() + children[1]->fullHeight() + spacing);
    rect.setWidth(qMax(children[0]->fullWidth(), children[1]->fullWidth()));
}

void FmaOver::calculatePos()
{
    qreal center = rect.x() + rect.width()/2;
    qreal y = rect.y();

    IFmaBase* vb[2] = {children[1], children[0]};

    for(IFmaBase* eb : vb){
        eb->rect.moveLeft(center - eb->rect.width()/2);
        eb->rect.moveTop(y+eb->marginTop());
        y+=eb->fullHeight() + spacing;
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaOver::upHeight()
{
    return children[0]->fullHeight()*0.5 + children[1]->fullHeight() + spacing;
}

void FmaOver::setOver(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setOver(new FmaText(type, text, formFix));
}

void FmaOver::setOver(IFmaBase *over)
{
    if(setToRowChild(over, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaOver::getOver()
{
    return (FmaRow *)children[1];
}

bool FmaOver::canStrechChild(IFmaBase *ch)
{
    if(ch == children[0]){
        return false;
    }

    if(ch->isDisplayStyle()){
        return false;
    }
    return true;
}

/////////
FmaUnderOver::FmaUnderOver(IFmaBase* buddy):FmaBuddyBase(buddy)
{
    children.append(new FmaRow);
    setLevelOffset(children[1], 1);
    children[1]->parent = this;

    children.append(new FmaRow);
    setLevelOffset(children[2], 1);
    children[2]->parent = this;
}

FmaUnderOver::FmaUnderOver(const QCborMap &cborMap):FmaBuddyBase(cborMap, 3)
{
    setLevelOffset(children[1], 1);
    setLevelOffset(children[2], 1);
}


FmaType FmaUnderOver::type()
{
    return FmaType::munderover;
}

void FmaUnderOver::calculateSize()
{
    IFmaContainer::calculateSize();

    underSpacing = lineThickness();
    overSpacing = 0;
    if(((FmaRow*)children[0])->isSingleText()){
        FmaText * t=((FmaRow*)children[0])->rowFirstText();
        QFontMetricsF metrics = QFontMetricsF( t->font() );
        QRectF rect1 = metrics.tightBoundingRect(t->text);
        overSpacing += -(metrics.ascent() + rect1.top());
    }

    if(((FmaRow*)children[1])->isSingleText()){
        FmaText * t=((FmaRow*)children[1])->rowFirstText();
        QFontMetricsF metrics = QFontMetricsF( t->font() );
        QRectF rect1 = metrics.tightBoundingRect(t->text);
        underSpacing += -(metrics.ascent() + rect1.top());
    }

    rect.setHeight(children[0]->fullHeight()
                    + children[1]->fullHeight()
                    + children[2]->fullHeight()
                    + overSpacing + underSpacing);

    qreal width = children[0]->fullWidth();
    qreal tWidth = qMax(children[1]->fullWidth(), children[2]->fullWidth());
    if(tWidth > width){
        rect.setWidth(tWidth);
    }else{
        rect.setWidth(width);
    }

}

void FmaUnderOver::calculatePos()
{
    qreal center = rect.x() + rect.width()/2;
    qreal y = rect.y();

    IFmaBase* vb[3] = {children[2], children[0], children[1]};
    qreal spaces[3] = {overSpacing, underSpacing, 0};

    int i=0;
    for(IFmaBase* eb : vb){
        eb->rect.moveLeft(center - eb->rect.width()/2);
        eb->rect.moveTop(y+eb->marginTop());
        y+=eb->fullHeight() + spaces[i++];
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaUnderOver::upHeight()
{
    return children[0]->fullHeight()*0.5 + children[1]->fullHeight() + overSpacing;
}

void FmaUnderOver::setUnder(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setUnder(new FmaText(type, text, formFix));
}

void FmaUnderOver::setUnder(IFmaBase *under)
{
    if(setToRowChild(under, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaUnderOver::getUnder()
{
    return (FmaRow *)children[1];
}

void FmaUnderOver::setOver(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setOver(new FmaText(type, text, formFix));
}

void FmaUnderOver::setOver(IFmaBase *over)
{
    if(setToRowChild(over, 2)){
        setLevelOffset(children[2], 1);
    }
}

FmaRow *FmaUnderOver::getOver()
{
    return (FmaRow *)children[2];
}

bool FmaUnderOver::canStrechChild(IFmaBase *ch)
{
    if(ch == children[0]){
        return false;
    }

    if(ch->isDisplayStyle()){
        return false;
    }
    return true;
}
