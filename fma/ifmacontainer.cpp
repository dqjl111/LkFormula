#include "ifmacontainer.h"
#include "fmarow.h"
#include "fmatext.h"
#include "fmadoc.h"
#include <QPainter>
#include <QJsonArray>

IFmaContainer::IFmaContainer()
{

}

IFmaContainer::IFmaContainer(const QCborMap &cborMap, int initChildNum):IFmaBase(cborMap)
{
    if(initChildNum>0){
        initFixNumChildren(cborMap, initChildNum);
    }
}

IFmaContainer::~IFmaContainer()
{
    clear();
}

bool IFmaContainer::isContainer()
{
    return true;
}

void IFmaContainer::calculateRect()
{
    calculateSize();
    stretchSize();
    calculatePos();
}

void IFmaContainer::stretchSize()
{
    IFmaBase::stretchSize();
    for (IFmaBase* eb : children)
    {
        eb->stretchSize();
    }
}

void IFmaContainer::calculatePos()
{

}

void IFmaContainer::calculateSize()
{
    for (IFmaBase* eb : children)
    {
        eb->calculateSize();
    }
}

FmaType IFmaContainer::type()
{
    return FmaType::mno;
}

bool IFmaContainer::isDynChild()
{
    return false;
}

QCborMap IFmaContainer::toCbor(bool filterSelect)
{
    QCborMap ret = IFmaBase::toCbor(filterSelect);
    QCborArray arr;

    if(filterSelect){
        if(isAllSelect()){
            for(IFmaBase* b: children){
                arr.append(b->toCbor(filterSelect));
            }
        }else{
            if(isDynChild()){
                for(IFmaBase* b: children){
                    if(b->cursorState != NoSelect){
                        arr.append(b->toCbor(filterSelect));
                    }
                }
            }else{
                for(IFmaBase* b: children){
                    arr.append(b->toCbor(filterSelect));
                }
            }
        }
    }else{
        for(IFmaBase* b: children){
            arr.append(b->toCbor(filterSelect));
        }
    }

    ret.insert(int(FmaCborId::Children), arr);
    return ret;
}


QCborArray IFmaContainer::childrenCbor(int pos, int len)
{
    QCborArray arr;
    int endPos = 0;
    if(len<0){
        endPos = children.size();
    }else{
        endPos = qMin(pos +len, children.size());
    }
    for(int i=pos; i<endPos; ++i){
        arr.append(children[i]->toCbor());
    }
    return arr;
}

//void IFmaContainer::parseChildren(const QCborArray &cborArr, int childNumLimit)
//{
//    QCborArray::const_iterator iter = cborArr.begin();
//    for(; iter != cborArr.end(); ++iter){
//        IFmaBase *fma = FmaDoc::createFma(iter->toMap());
//        if(fma){
//            children.append(fma);
//        }else{
//            return;
//        }
//    }
//}

//QJsonObject IFmaContainer::toJObjet()
//{
//    QJsonObject ret = IFmaBase::toJObjet();
//    QJsonArray arr;
//    for(IFmaBase* b: children){
//        arr.append(b->toJObjet());
//    }
//    ret[toFmaTypeName(type())] = arr;
//    return ret;
//}

const QList<IFmaBase *> &IFmaContainer::getChildren()
{
    return children;
}

IFmaBase *IFmaContainer::getActiveChild()
{
    return activeChild;
}

void IFmaContainer::setActiveChild(IFmaBase *_activeChild)
{
    if(_activeChild == nullptr){
        activeChild = nullptr;
        return;
    }

    if(_activeChild == &startBase){
        activeChild = _activeChild;
        return;
    }

    if(children.contains(_activeChild)){
        activeChild = _activeChild;
    }
}

bool IFmaContainer::deleteChild(IFmaBase *child)
{
    if(isDynChild()){
        if(isValid(child)){
            if(child->parent == this){
                if(child == activeChild){
                    activeChild = nullptr;
                }
                children.removeAll(child);
                delete child;
                return true;
            }
        }
    }
    return false;
}

bool IFmaContainer::take(IFmaBase *child)
{
    if(isDynChild()){
        if(isValid(child)){
            if(child->parent == this){
                children.removeAll(child);
                child->parent = nullptr;
                if(child == activeChild){
                    activeChild = nullptr;
                }
                return true;
            }
        }

    }
    return false;
}

int IFmaContainer::indexOfChild(IFmaBase *child)
{
    return children.indexOf(child);
}

int IFmaContainer::del(int pos, int length)
{
    if(pos <0){
        return 0;
    }

    if(pos >children.size()){
        return 0;
    }


    if(isDynChild()){
        int delNum =0;
        for(int i=0; i<length; ++i){
            if(pos<children.size()){
                IFmaBase* child = children.takeAt(pos);
                if(child == activeChild){
                    activeChild = nullptr;
                }
                delete child;
                delNum ++;
            }else{
                break;
            }
        }

        if(pos >=0 && pos< children.size()){
            activeChild = children[pos];
            activeChild->moveToStart();
        }
        return delNum;
    }else{
        return 0;
    }
}

bool IFmaContainer::insert(IFmaBase *child, int pos)
{
    if(isDynChild()){
        if(isAcceptableChild(child)){
            if(child->parent != nullptr){
                child->parent->take(child);
            }
            children.insert(pos, child);
            child->parent = this;
            return true;
        }
        return false;
    }
    return false;
}

void IFmaContainer::clear()
{
    for(IFmaBase* b: children){
        delete b;
    }
    children.clear();
    activeChild = nullptr;
}

bool IFmaContainer::hasChilds()
{
    return !children.isEmpty();
}

bool IFmaContainer::isAncestor(IFmaBase *t)
{
    if(t== nullptr){
        return false;
    }

    IFmaContainer* p = parent;
    while(p){
        if(p == t){
            return true;
        }
        p = p->parent;
    }
    return false;
}

bool IFmaContainer::isAcceptableChild(IFmaBase *t)
{
    if(t == this){
        return false;
    }

    if(!isValid(t)){
        return false;
    }

    if(children.contains(t)){
        return false;
    }

    IFmaContainer* p = parent;
    while(p){
        if(p == t){
            return false;
        }
        p = p->parent;
    }

    return true;
}

int IFmaContainer::childCount()
{
    return children.size();
}

IFmaBase *IFmaContainer::firstBlock()
{
    if(children.isEmpty()){
        return nullptr;
    }
    return children.first();
}

IFmaBase *IFmaContainer::nextBlock()
{
    if(isAtStart()){
        return firstBlock();
    }else if(isAtEnd()){
        return nullptr;
    }else{
        int i= children.indexOf(activeChild);
        if(i>=0 && (i+1) < children.size()){
            return children[i+1];
        }else{
            return nullptr;
        }
    }
}

IFmaBase *IFmaContainer::previousBlock()
{
    if(isAtStart()){
        return nullptr;
    }else if(isAtEnd()){
        return lastBlock();
    }else{
        int i= children.indexOf(activeChild);
        if((i-1)>=0){
            return children[i-1];
        }else{
            return nullptr;
        }
    }
}

IFmaBase *IFmaContainer::lastBlock()
{
    if(children.isEmpty()){
        return nullptr;
    }
    return children.last();
}

bool IFmaContainer::hasSelect()
{
    if(cursorState == NoSelect){
        return false;
    }

    for (IFmaBase* eb : children)
    {
        if(eb->hasSelect()){
            return true;
        }
    }
    return false;
}

bool IFmaContainer::isAllSelect()
{
    if(cursorState == NoSelect){
        return false;
    }

    for (IFmaBase* eb : children)
    {
        if(!eb->isAllSelect()){
            return false;
        }
    }
    return true;
}


void IFmaContainer::deSelect()
{
    cursorState = NoSelect;
    for (IFmaBase* eb : children)
    {
        eb->deSelect();
    }
}

void IFmaContainer::selectAll(bool isLeftToRight)
{
    cursorState = PartialSelect;
    if(isLeftToRight){
        partSelectStartId = 0;
        partSelectEndId = children.size();
    }else{
        partSelectStartId = children.size();
        partSelectEndId = 0;
    }
    for (IFmaBase* eb : children)
    {
        eb->selectAll(isLeftToRight);
    }
}


void IFmaContainer::selectFrom(const QVector<int> &indexes, int level, quint8 type)
{
    if(level>= indexes.size()){
        switch (type) {
        case SelectToLeft:{
            selectAll(true);
            break;
        }
        case SelectToRight:{
            deSelect();
            break;
        }
        default:{
            deSelect();
            break;
        }
        }
        return;
    }


    cursorState = PartialSelect;
    partSelectStartId = indexes[level];

    switch (type) {
    case SelectToLeft:{
        partSelectEndId = -1;
        int i=0;
        for (IFmaBase* eb : children)
        {
            if(i< partSelectStartId){
                eb->selectAll(false);
            }else if(i == partSelectStartId){
                eb->selectFrom(indexes, level+1, type);
            }else{
                eb->deSelect();
            }
            ++i;
        }
        break;
    }
    case SelectToRight:{
        partSelectEndId = children.size();
        int i=0;
        for (IFmaBase* eb : children)
        {
            if(i> partSelectStartId){
                eb->selectAll(true);
            }else if(i == partSelectStartId){
                eb->selectFrom(indexes, level+1, type);
            }else{
                eb->deSelect();
            }
            ++i;
        }
        break;
    }
    default:{
        partSelectEndId = partSelectStartId;
        if(partSelectStartId>=0 && partSelectStartId< children.size()){
            IFmaBase* eb = children[partSelectStartId];
            eb->selectFrom(indexes, level+1, type);
        }
        break;
    }
    }
}

void IFmaContainer::selectTo(const QVector<int> &indexes, int level, quint8 type)
{
    if(level>= indexes.size()){
        switch (type) {
        case SelectToRight:{
            selectAll(false);
            break;
        }
        case SelectToLeft:{
            deSelect();
            break;
        }
        default:{
            deSelect();
            break;
        }
        }
        return;
    }


    cursorState = PartialSelect;
    partSelectEndId = indexes[level];

    switch (type) {
    case SelectToRight:{
        partSelectStartId = -1;
        int i=0;
        for (IFmaBase* eb : children)
        {
            if(i< partSelectEndId){
                eb->selectAll(true);
            }else if(i == partSelectEndId){
                eb->selectTo(indexes, level+1, type);
            }else{
                eb->deSelect();
            }
            ++i;
        }
        break;
    }
    case SelectToLeft:{
        partSelectStartId = children.size();
        int i=0;
        for (IFmaBase* eb : children)
        {
            if(i> partSelectEndId){
                eb->selectAll(false);
            }else if(i == partSelectEndId){
                eb->selectTo(indexes, level+1, type);
            }else{
                eb->deSelect();
            }
            ++i;
        }
        break;
    }
    default:{
        partSelectStartId = partSelectEndId;
        if(partSelectStartId>=0 && partSelectStartId< children.size()){
            IFmaBase* eb = children[partSelectStartId];
            eb->selectTo(indexes, level+1, type);
        }
        break;
    }
    }
}

void IFmaContainer::selectRange(const QVector<int> &start, const QVector<int> &end, int level, quint8 type)
{
    if(start.size() >level ){
        if(end.size()>level){
            int startId = start[level];
            int endId = end[level];

            if(startId == endId){
                cursorState = PartialSelect;
                partSelectStartId = startId;
                partSelectEndId = partSelectStartId;
                if(partSelectStartId >= 0 && partSelectStartId <children.size()){
                    IFmaBase* eb = children[partSelectStartId];
                    eb->selectRange(start, end, level+1, type);
                }
                return;
            }else{
                cursorState = PartialSelect;
                partSelectStartId = startId;
                partSelectEndId = endId;

                switch (type) {
                case SelectToLeft:{
                    int i=0;
                    for(IFmaBase* ch: children){
                        if(i >endId && i < startId){
                            ch->selectAll(false);
                        }

                        if(i == startId){
                            ch->selectFrom(start, level+1, type);
                        }else if(i == endId){
                            ch->selectTo(end, level+1, type);
                        }
                        i++;
                    }
                    break;
                }
                case SelectToRight:{
                    int i=0;
                    for(IFmaBase* ch: children){
                        if(i >startId && i < endId){
                            ch->selectAll(true);
                        }

                        if(i == startId){
                            ch->selectFrom(start, level+1, type);
                        }else if(i == endId){
                            ch->selectTo(end, level+1, type);
                        }
                        i++;
                    }
                    break;
                }
                default:{
                    break;
                }
                }
            }
        }else{
            selectFrom(start, level, type);
        }
    }else{
        if(end.size()>level){
            selectTo(end, level, type);
        }
    }
}

void IFmaContainer::initFixNumChildren(const QCborMap &cborMap, int childLimit)
{
    int r =children.size();

    QCborMap::const_iterator mapIter = findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        QCborArray::const_iterator iter = cborArr.begin();
        for(; iter != cborArr.end(); ++iter){
            if(r <childLimit){
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
            }else{
                break;
            }
        }
    }

    for(; r<childLimit; ++r){
        children.append(new FmaRow);
        ((FmaRow*)children[r])->parent = this;
    }
}

QList<std::shared_ptr<FmaEditOp>> IFmaContainer::removeSelection()
{
    QList<std::shared_ptr<FmaEditOp>> ret;
    if(cursorState == NoSelect){
        return ret;
    }

    QList<IFmaBase*> partialSelectList;
    QList<IFmaBase*> allSelectList;

    for (IFmaBase* eb : children)
    {
        if(eb->cursorState != NoSelect){
            if(eb->isAllSelect()){
                allSelectList.append(eb);
            }else{
                partialSelectList.append(eb);
            }
        }
    }

    if(isDynChild()){
        for(IFmaBase* b: allSelectList){
            if(b == activeChild){
                activeChild = nullptr;
            }

            std::shared_ptr<FmaItemInputDelete> op
                    = std::make_shared<FmaItemInputDelete>(false);
            buildIndexes(op->editContainerIndex);
            op->pos = children.indexOf(b);
            op->validNum = 1;
            op->inputCbor.append(b->toCbor());
            ret.append(op);

            children.removeAll(b);
            delete b;
        }
    }else{
        for(IFmaBase* b: allSelectList){
            ret.append(b->removeSelection());
        }
    }

    for(IFmaBase* b: partialSelectList){
        ret.append(b->removeSelection());
    }

    partSelectEndId = 0;
    partSelectStartId = 0;
    cursorState = NoSelect;
    return ret;
}

//bool IFmaContainer::deleteActive(bool isForward)
//{
//    if(isValid(activeChild)){
//        IFmaBase* t = nullptr;
//        if(isForward){
//            t = nextBlock();
//        }else{
//            t = previousBlock();
//        }
//        if(deleteChild(activeChild)){
//            activeChild = t;
//            return true;
//        }
//    }
//    return false;
//}

//void IFmaContainer::selectToJson(QJsonArray& parentArray)
//{
//    if(cursorState == NoSelect){
//        return;
//    }

//    QJsonObject ret = IFmaBase::toJObjet();
//    QJsonArray arr;
//    for(IFmaBase* eb: children){
//        if(eb->cursorState != NoSelect){
//            if(eb->isAllSelect()){
//                arr.append(eb->toJObjet());
//            }else{
//                eb->selectToJson(arr);
//            }
//        }
//    }
//    ret[toFmaTypeName(type())] = arr;
//    parentArray.append(ret);
//}

bool IFmaContainer::moveCursor(QTextCursor::MoveOperation op)
{
    if(isValid(activeChild)){
        if(activeChild->moveCursor(op)){
            return true;
        }
    }

    switch(op){
    case QTextCursor::NoMove:{
        return true;
    }
    case QTextCursor::Start:
    case QTextCursor::StartOfLine:
    case QTextCursor::StartOfWord:{
        moveToStart();
        return true;
    }
    case QTextCursor::StartOfBlock:{
        IFmaBase *t = firstBlock();
        if(isValid(t)){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::StartOfBlock);
        }else{
            moveToStart();
            return true;
        }
    }
    case QTextCursor::PreviousBlock:
    case QTextCursor::PreviousCharacter:
    case QTextCursor::PreviousWord:
    case QTextCursor::Left:
    case QTextCursor::WordLeft:
    case QTextCursor::PreviousCell:{
        if(isAtStart()){
            return false;
        }
        IFmaBase *t  = previousBlock();
        if(isValid(t)){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::EndOfBlock);
        }else{
            moveToStart();
            return true;
        }
    }
    case QTextCursor::End:
    case QTextCursor::EndOfLine:
    case QTextCursor::EndOfWord:{
        moveToEnd();
        return true;
    }
    case QTextCursor::EndOfBlock:{
        IFmaBase *t = lastBlock();
        if(isValid(t)){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::EndOfBlock);
        }else{
            moveToEnd();
            return true;
        }
    }
    case QTextCursor::NextBlock:
    case QTextCursor::NextCharacter:
    case QTextCursor::NextWord:
    case QTextCursor::Right:
    case QTextCursor::WordRight:
    case QTextCursor::NextCell:{
        if(isAtEnd()){
            return false;
        }
        IFmaBase *t = nextBlock();
        if(isValid(t)){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::StartOfBlock);
        }else{
            moveToEnd();
            return true;
        }
    }
    default:{
        break;
    }
    }
    return false;
}

bool IFmaContainer::mouseClick(const QPointF &p)
{
    activeChild = nullptr;
    for (IFmaBase* eb : children)
    {
        if(eb->bounds().contains(p)){
            activeChild = eb;
            break;
        }
    }

    if(isValid(activeChild)){
        activeChild->mouseClick(p);
        return true;
    }else{
        if(p.x() > midX()){
            moveToEnd();
        }else{
            moveToStart();
        }
        return true;
    }
}

int IFmaContainer::maxIndex()
{
    return children.count();
}

int IFmaContainer::minIndex()
{
    return -1;
}

int IFmaContainer::activePos()
{
    if(activeChild){
        if(isAtStart()){
            return -1;
        }

        return children.indexOf(activeChild);
    }else{
        return children.size();
    }
}

void IFmaContainer::setActivePos(int pos)
{
    if(pos <0){
        moveToStart();
        return;
    }

    if(pos <= children.size()){
        activeChild = children[pos];
    }else{
        moveToEnd();
    }
}

bool IFmaContainer::setToRowChild(IFmaBase *t, int index)
{
    if(index>=0 && index<=children.size()){
        if(children[index]->isRow()){
            if(!isAcceptableChild(t)){
                return false;
            }

            if(t->isRow()){
                if(children[index] != t){
                    delete children[index];
                    t->parent = this;
                    children[index] = t;
                }
            }else{
                children[index]->clear();
                ((FmaRow*)children[index])->append(t);
            }
            return true;
        }
    }
    return false;
}

bool IFmaContainer::canStrechChild(IFmaBase* ch)
{
    if(ch->isDisplayStyle()){
        return false;
    }
    if(isRow()){
        if(ch->isRow() || ch->isText()){
            return ch->levelOffset == 0;
        }
    }
    return false;
}

bool IFmaContainer::isStretchY()
{
    for (IFmaBase* eb : children)
    {
        if(!eb->isDisplayStyle()){
            if(eb->levelOffset == 0){
                if(eb->isStretchY()){
                    return true;
                }
            }
        }
    }
    return false;
}

bool IFmaContainer::isAtStart()
{
    return activeChild == &startBase;
}

bool IFmaContainer::isAtEnd()
{
    return activeChild == nullptr;
}

void IFmaContainer::moveToStart()
{
    activeChild = &startBase;
}

void IFmaContainer::moveToEnd()
{
    activeChild = nullptr;
}

void IFmaContainer::markSelectStart()
{
    if((cursorState & PartialSelect) == NoSelect){
        cursorState = PartialSelect;
        if(isAtStart()){
            partSelectStartId = -1;
        }else if(isAtEnd()){
            partSelectStartId = children.size();
        }else{
            int index = children.indexOf(activeChild);
            if(index >=0){
                partSelectStartId = index;
                activeChild->markSelectStart();
            }
        }
    }
}

bool IFmaContainer::isStretchX()
{
    for (IFmaBase* eb : children)
    {
        if(!eb->isDisplayStyle()){
            if(eb->levelOffset == 0){
                if(eb->isStretchX()){
                    return true;
                }
            }
        }
    }
    return false;
}

void IFmaContainer::paint(QPainter *painter)
{
    for (IFmaBase* eb : children)
    {
        eb->paint(painter);
    }
}

void IFmaContainer::paintSelect(QPainter *p)
{
    if(isAllSelect()){
        QRectF f = bounds();
        p->save();
        p->setBrushOrigin(f.topLeft());
        p->fillRect(f, p->background());
        p->restore();
    }else{
        for (IFmaBase* eb : children)
        {
            eb->paintSelect(p);
        }
    }
}

///////////////////
FmaBuddyBase::FmaBuddyBase(IFmaBase* buddy)
{
    if(buddy){
        if(buddy->isRow()){
            children.append(buddy);
        }else{
            children.append(new FmaRow);
            ((FmaRow*)children[0])->append(buddy);
        }
    }else{
        children.append(new FmaRow);
    }
    setLevelOffset(children[0], 0);
    children[0]->parent = this;
}

FmaBuddyBase::FmaBuddyBase(const QCborMap &cborMap, int childLimit):IFmaContainer(cborMap, childLimit)
{
}

void FmaBuddyBase::setBuddy(const QString &text,  FmaType type, FmaOperDict::FormFix formFix )
{
    setBuddy(new FmaText(type, text, formFix));
}

void FmaBuddyBase::setBuddy(IFmaBase *buddy)
{
    if(setToRowChild(buddy, 0)){
        setLevelOffset(children[0], 0);
    }
}

bool FmaBuddyBase::canStrechChild(IFmaBase *ch)
{
    if(ch == children[0]){
        return false;
    }
    return IFmaContainer::canStrechChild(ch);
}

FmaRow *FmaBuddyBase::getBuddy()
{
    return (FmaRow *)children[0];
}


////////////////////////////

FmaFenced::FmaFenced(const QString openStr, const QString closeStr, IFmaBase *child):FmaBuddyBase(child)
{
    setOpenClose(openStr, closeStr);
}

FmaFenced::FmaFenced(const QCborMap &cborMap):FmaBuddyBase(cborMap)
{
    QString openStr = "(";
    QCborMap::const_iterator iter = cborMap.find(int(FmaCborId::Open));
    if(iter != cborMap.end()){
        openStr = iter.value().toString();
    }

    QString closeStr = ")";
    iter = cborMap.find(int(FmaCborId::Close));
    if(iter != cborMap.end()){
        closeStr = iter.value().toString();
    }
    setOpenClose(openStr, closeStr);
}

FmaFenced::~FmaFenced()
{
    if(open){
        delete open;
    }
    if(close){
        delete close;
    }
}

FmaType FmaFenced::type()
{
    return FmaType::mfenced;
}

void FmaFenced::paint(QPainter *painter)
{
    if(open){
        open->paint(painter);
    }
    if(close){
        close->paint(painter);
    }
    FmaBuddyBase::paint(painter);
}

void FmaFenced::calculatePos()
{
    qreal x = rect.x();
    qreal centerV = rect.y()+ rect.height()*0.5;

    IFmaBase* items[3];
    items[0] = open;
    items[1] = children[0];
    items[2] = close;


    for (IFmaBase* eb : items)
    {
        if(eb){
            x += eb->marginLeft();
            eb->rect.moveLeft(x);
            x += eb->rect.width() + eb->marginRight();
        }
    }

    if(open){
        if(open->isStretchY()){
            open->rect.moveTop(rect.y()+ open->marginTop());
        }else{
            open->rect.moveTop(centerV - open->fullHeight()*0.5);
        }
    }

    if(close){
        if(close->isStretchY()){
            close->rect.moveTop(rect.y()+ close->marginTop());
        }else{
            close->rect.moveTop(centerV - close->fullHeight()*0.5);
        }
    }

    children[0]->rect.moveTop(centerV - children[0]->fullHeight()*0.5);

    if(children[0]->isContainer()){
        ((IFmaContainer*)children[0])->calculatePos();
    }
}

QCborMap FmaFenced::toCbor(bool filterSelect)
{
    QCborMap ret = FmaBuddyBase::toCbor(filterSelect);
    if(open){
        ret.insert(int(FmaCborId::Open), open->text);
    }else{
        ret.insert(int(FmaCborId::Open), "");
    }

    if(close){
        ret.insert(int(FmaCborId::Close), close->text);
    }else{
        ret.insert(int(FmaCborId::Close), "");
    }
    return ret;
}

void FmaFenced::calculateSize()
{
    if(open){
        open->calculateSize();
    }
    if(close){
        close->calculateSize();
    }
    FmaBuddyBase::calculateSize();

    IFmaBase* items[3];
    items[0] = open;
    items[1] = children[0];
    items[2] = close;

    qreal width = 0;
    qreal height = 0;
    for (IFmaBase* eb : items)
    {
        if(eb){
            width += eb->fullWidth();
            height = qMax(height, eb->fullHeight());
        }
    }
    rect.setWidth(width);
    rect.setHeight(height);
}

void FmaFenced::stretchSize()
{
    FmaBuddyBase::stretchSize();
    if(open){
        if(open->isStretchY()){
            open->rect.setHeight(rect.height() - marginTop() - marginBottom());
        }
    }

    if(close){
        if(close->isStretchY()){
            close->rect.setHeight(rect.height() - marginTop() - marginBottom());
        }
    }
}

void FmaFenced::setOpenClose(const QString& openStr, const QString& closeStr)
{
    if(!openStr.isEmpty()){
        if(FmaOperDict::hasForm(openStr, FmaOperDict::mmlPrefix)){
            open = new FmaText(FmaType::mo, openStr, FmaOperDict::mmlPrefix);
        }else{
            open = new FmaText(FmaType::mo, openStr, FmaOperDict::mmlNofix);
        }
        open->parent = this;
    }

    if(!closeStr.isEmpty()){
        if(FmaOperDict::hasForm(closeStr, FmaOperDict::mmlPostfix)){
            close = new FmaText(FmaType::mo, closeStr, FmaOperDict::mmlPostfix);
        }else{
            close = new FmaText(FmaType::mo, closeStr, FmaOperDict::mmlNofix);
        }
        close->parent = this;
    }
}

