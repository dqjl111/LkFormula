#include "fmasubsuperbase.h"
#include "fmarow.h"
#include "fmatextformats.h"
#include "fmatext.h"
#include <QDebug>

FmaSubSuperBase::FmaSubSuperBase(IFmaBase *buddy): FmaBuddyBase(buddy)
{

}

FmaSubSuperBase::FmaSubSuperBase(const QCborMap &cborMap, int limit): FmaBuddyBase(cborMap, limit)
{
    dir = Qt::LeftToRight;
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = Qt::RightToLeft;
        }
    }
}

QCborMap FmaSubSuperBase::toCbor(bool filterSelect)
{    
    QCborMap ret = FmaBuddyBase::toCbor(filterSelect);
    ret.insert(int(FmaCborId::Dir), int(dir));
    return ret;
}



FmaSub::FmaSub(IFmaBase *buddy, IFmaBase *sub) : FmaSubSuperBase(buddy)
{
    if(sub){
        if(sub->isRow()){
            children.append(sub);
        }else{
            children.append(new FmaRow);
            ((FmaRow*)children[1])->append(sub);
        }
    }else{
        children.append(new FmaRow);
    }
    setLevelOffset(children[1], 1);
    children[1]->parent = this;
}

FmaSub::FmaSub(const QCborMap &cborMap): FmaSubSuperBase(cborMap, 2)
{
    setLevelOffset(children[1], 1);
}

FmaType FmaSub::type()
{
    return FmaType::msub;
}

void FmaSub::calculateSize()
{
    IFmaContainer::calculateSize();

    qreal width = 0;
    for (IFmaBase* eb : children)
    {
        width += eb->fullWidth();
    }
    rect.setWidth(width);
    qreal halfHeight = children[0]->fullHeight()*0.5;
    rect.setHeight(halfHeight + qMax(children[1]->fullHeight(), halfHeight));
}

void FmaSub::calculatePos()
{
    qreal x = rect.x();
    qreal y = rect.y();

    if(dir == Qt::RightToLeft){
        IFmaBase* eb = children[1];
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        eb->rect.moveTop(y + rect.height() - eb->rect.height() - eb->marginBottom());
        x += eb->rect.width() + eb->marginRight();

        eb = children[0];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + eb->marginTop());
    }else{
        IFmaBase* eb = children[0];
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        eb->rect.moveTop(y + eb->marginTop());
        x += eb->rect.width() + eb->marginRight();

        eb = children[1];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + rect.height() - eb->rect.height() - eb->marginBottom());
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaSub::upHeight()
{
    return children[0]->fullHeight()*0.5 + marginTop();
}

void FmaSub::setSub(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setSub(new FmaText(type, text, formFix));
}

void FmaSub::setSub(IFmaBase *sub)
{
    if(setToRowChild(sub, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaSub::getSub()
{
    return (FmaRow *)children[1];
}

FmaSuper::FmaSuper(IFmaBase *buddy, IFmaBase *super) : FmaSubSuperBase(buddy)
{
    if(super){
        if(super->isRow()){
            children.append(super);
        }else{
            children.append(new FmaRow);
            ((FmaRow*)children[1])->append(super);
        }
    }else{
        children.append(new FmaRow);
    }
    setLevelOffset(children[1], 1);
    children[1]->parent = this;
}

FmaSuper::FmaSuper(const QCborMap &cborMap): FmaSubSuperBase(cborMap, 2)
{
    setLevelOffset(children[1], 1);
}

FmaType FmaSuper::type()
{
    return FmaType::msup;
}

void FmaSuper::calculateSize()
{
    IFmaContainer::calculateSize();

    qreal width = 0;
    for (IFmaBase* eb : children)
    {
        width += eb->fullWidth();
    }
    rect.setWidth(width);
    qreal halfHeight = children[0]->fullHeight()*0.5;
    rect.setHeight(halfHeight + qMax(children[1]->fullHeight(), halfHeight));
}

void FmaSuper::calculatePos()
{
    qreal x = rect.x();
    qreal y = rect.y();


    if(dir == Qt::RightToLeft){        
        IFmaBase* eb = children[1];
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        eb->rect.moveTop(y + eb->marginTop());
        x += eb->rect.width() + eb->marginRight();

        eb = children[0];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + rect.height() - children[0]->marginBottom() - children[0]->rect.height());
    }else{
        IFmaBase* eb = children[0];
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        eb->rect.moveTop(y + rect.height() - children[0]->marginBottom() - children[0]->rect.height());
        x += eb->rect.width() + eb->marginRight();

        eb = children[1];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + eb->marginTop());
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaSuper::upHeight()
{
    qreal halfHeight = children[0]->fullHeight()*0.5;
    return rect.height() - halfHeight + marginTop();
}

void FmaSuper::setSuper(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setSuper(new FmaText(type, text, formFix));
}

void FmaSuper::setSuper(IFmaBase *super)
{
    if(setToRowChild(super, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaSuper::getSuper()
{
    return (FmaRow *)children[1];
}

FmaSubSuper::FmaSubSuper(IFmaBase *buddy, IFmaBase *sub, IFmaBase *super): FmaSubSuperBase(buddy)
{
    if(sub){
        if(sub->isRow()){
            children.append(super);
        }else{
            children.append(new FmaRow);
            ((FmaRow*)children[1])->append(sub);
        }
    }else{
        children.append(new FmaRow);
    }
    setLevelOffset(children[1], 1);
    children[1]->parent = this;

    if(super){
        if(super->isRow()){
            children.append(super);
        }else{
            children.append(new FmaRow);
            ((FmaRow*)children[2])->append(super);
        }
    }else{
        children.append(new FmaRow);
    }
    setLevelOffset(children[2], 1);
    children[2]->parent = this;
}

FmaSubSuper::FmaSubSuper(const QCborMap &cborMap): FmaSubSuperBase(cborMap, 3)
{
    setLevelOffset(children[1], 1);
    setLevelOffset(children[2], 1);
}

FmaType FmaSubSuper::type()
{
    return FmaType::msubsup;
}

void FmaSubSuper::calculateSize()
{
    IFmaContainer::calculateSize();

    qreal width = children[0]->fullWidth()
            + qMax(children[1]->fullWidth(), children[2]->fullWidth());

    rect.setWidth(width);
    qreal halfHeight = children[0]->fullHeight()*0.5;
    rect.setHeight(qMax(children[1]->fullHeight(), halfHeight) + qMax(children[2]->fullHeight(), halfHeight));
}

void FmaSubSuper::calculatePos()
{
    qreal x = rect.x();
    qreal y = rect.y();

    if(dir == Qt::RightToLeft){
        IFmaBase* eb = nullptr;
        qreal w1 = children[1]->fullWidth();
        qreal w2 = children[2]->fullWidth();
        if(w1 >= w2){            
            eb = children[1];
            //x += eb->marginLeft();
            eb->rect.moveLeft(x + eb->marginLeft());
            eb->rect.moveTop(y + rect.height() - eb->rect.height() - eb->marginBottom());

            x += (w1-w2);
            eb = children[2];
            x += eb->marginLeft();
            eb->rect.moveLeft(x);
            eb->rect.moveTop(y + eb->marginTop());

            x += eb->rect.width() + eb->marginRight();
        }else{
            eb = children[2];
            //x += eb->marginLeft();
            eb->rect.moveLeft(x + eb->marginLeft());
            eb->rect.moveTop(y + eb->marginTop());

            x += (w2-w1);
            eb = children[1];
            x += eb->marginLeft();
            eb->rect.moveLeft(x);
            eb->rect.moveTop(y + rect.height() - eb->rect.height() - eb->marginBottom());
            x += eb->rect.width() + eb->marginRight();
        }

        eb = children[0];
        eb->rect.moveLeft(x + eb->marginLeft());
        qreal baseHeight = eb->fullHeight();
        eb->rect.moveTop(y + qMax(children[2]->fullHeight(), baseHeight*0.5)- baseHeight*0.5 + eb->marginTop());
    }else{
        IFmaBase* eb = children[0];
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        qreal baseHeight = eb->fullHeight();
        eb->rect.moveTop(y + qMax(children[2]->fullHeight(), baseHeight*0.5)- baseHeight*0.5 + eb->marginTop());
        x += eb->rect.width() + eb->marginRight();

        eb = children[1];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + rect.height() - eb->rect.height() - eb->marginBottom());

        eb = children[2];
        eb->rect.moveLeft(x + eb->marginLeft());
        eb->rect.moveTop(y + eb->marginTop());
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaSubSuper::upHeight()
{
    return qMax(children[2]->fullHeight(), children[0]->fullHeight()*0.5) + marginTop();
}

void FmaSubSuper::setSub(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setSub(new FmaText(type, text, formFix));
}

void FmaSubSuper::setSub(IFmaBase *sub)
{
    if(setToRowChild(sub, 1)){
        setLevelOffset(children[1], 1);
    }
}

FmaRow *FmaSubSuper::getSub()
{
    return (FmaRow *)children[1];
}

void FmaSubSuper::setSuper(const QString &text, FmaType type, FmaOperDict::FormFix formFix)
{
    setSuper(new FmaText(type, text, formFix));
}

void FmaSubSuper::setSuper(IFmaBase *super)
{
    if(setToRowChild(super, 2)){
        setLevelOffset(children[2], 1);
    }
}

FmaRow *FmaSubSuper::getSuper()
{
    return (FmaRow *)children[2];
}

