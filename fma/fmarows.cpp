#include "fmarows.h"
#include "fmarow.h"
#include "fmadoc.h"

FmaRows::FmaRows()
{

}

void FmaRows::fromCborMap(const QCborMap &cborMap)
{
    clear();
    int r=0;
    QCborMap::const_iterator mapIter = findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        QCborArray::const_iterator iter = cborArr.begin();
        for(; iter != cborArr.end(); ++iter){
            IFmaBase* b= FmaDoc::fromCbor(iter->toMap());
            if(b){
                if(b->isRow()){
                    children.append(b);
                    b->parent = this;
                }else{
                    children.append(new FmaRow);
                    ((FmaRow*)children[r])->append(b);
                    ((FmaRow*)children[r])->parent = this;
                }
                r++;
            }
        }
    }
}

bool FmaRows::isDynChild()
{
    return true;
}

bool FmaRows::append(FmaRow *child)
{
    return insert(child, children.size());
}

bool FmaRows::prepend(FmaRow *child)
{
    return insert(child, 0);
}

bool FmaRows::insert(IFmaBase *child, int pos)
{
    if(child->isRow()){
        IFmaContainer::insert(child, pos);
    }
    return false;
}

bool FmaRows::setToRowChild(IFmaBase *t, int index)
{
    if(children.size() > index){
        return IFmaContainer::setToRowChild(t, index);
    }else if(children.size() == index){
        if(t->isRow()){
            return insert((FmaRow*)t, index);
        }else{
            FmaRow* tRow = new FmaRow;
            tRow->append(t);
            return insert(tRow, index);
        }
    }else{
        return false;
    }
}

FmaRow *FmaRows::currentRow()
{
    return (FmaRow *)activeChild;
}

FmaRow *FmaRows::firstRow()
{
    return (FmaRow *)(firstBlock());
}

FmaRow *FmaRows::nextRow()
{
    return (FmaRow *)(nextBlock());
}

FmaRow *FmaRows::previousRow()
{
    return (FmaRow *)(previousBlock());
}

FmaRow *FmaRows::lastRow()
{
    return (FmaRow *)(lastBlock());
}

void FmaRows::calculateSize()
{
    IFmaContainer::calculateSize();

    qreal height = 0;
    qreal maxWidth = 0;
    for (IFmaBase* eb : children)
    {
        height += eb->fullHeight();
        maxWidth = qMax(maxWidth, eb->fullWidth());
    }
    rect.setWidth(maxWidth);
    rect.setHeight(height);
}

void FmaRows::calculatePos()
{
    qreal y = rect.y();
    for (IFmaBase* eb : children)
    {
        eb->rect.moveLeft(rect.x() + eb->marginLeft());
        y += eb->marginTop();
        eb->rect.moveTop(y);
        y += eb->rect.height() + eb->marginBottom();
    }
    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

bool FmaRows::mouseClick(const QPointF &p)
{
    if(bounds().contains(p)){
        mouseClickIgnoreWidth(p);
    }
    return false;
}

bool FmaRows::moveCursor(QTextCursor::MoveOperation op)
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
            activeChild->moveToEnd();
            return true;
        }
        break;
    }
    default:{
        break;
    }
    }
    IFmaContainer::moveCursor(op);

    if(isAtEnd()){
        IFmaBase *t = lastBlock();
        if(isValid(t)){
            activeChild = t;
            activeChild->moveToEnd();
        }
    }else if(isAtStart()){
        IFmaBase *t = firstBlock();
        if(isValid(t)){
            activeChild = t;
            activeChild->moveToStart();
        }
    }
    return true;
}

bool FmaRows::mouseClickIgnoreWidth(const QPointF &p)
{
    activeChild = nullptr;
    qreal s = rect.top();
    qreal e = s;
    for (IFmaBase* eb : children)
    {
        e = eb->rect.bottom() + eb->marginBottom();
        if ((p.y() >= s) &&
                (p.y() <= e))
        {
            activeChild = eb;
            break;
        }
        s = e;
    }

    if(isValid(activeChild)){
        activeChild->mouseClick(p);
        return true;
    }else{
        if(p.y() >= midY()){
            IFmaBase *t = lastBlock();
            if(isValid(t)){
                activeChild = t;
                activeChild->mouseClick(p);
                return true;
            }
        }else{
            IFmaBase *t = firstBlock();
            if(isValid(t)){
                activeChild = t;
                activeChild->mouseClick(p);
                return true;
            }
        }
    }
    return false;
}
