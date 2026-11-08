#include "fmarow.h"
#include "fmatext.h"
#include "fmadoc.h"
#include <QPainter>
#include <QDebug>

FmaRow::FmaRow(IFmaBase* child)
{
    if(isValid(child)){
        if(child->isRow()){
            insert((FmaRow*)child);
        }else{
            children.append(child);
            child->parent = this;
        }
    }
}

FmaRow::FmaRow(const QCborMap &cborMap):IFmaContainer(cborMap)
{
    QCborMap::const_iterator mapIter = findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        QCborArray::const_iterator iter = cborArr.begin();
        for(; iter != cborArr.end(); ++iter){
            IFmaBase* b= FmaDoc::fromCbor(iter->toMap());
            if(b){
                children.append(b);
                b->parent = this;
            }
        }
    }
}

bool FmaRow::isRow()
{
    return true;
}

bool FmaRow::isStretchX()
{
    return false;
}

bool FmaRow::isStretchY()
{
    return false;
}

FmaType FmaRow::type()
{
    return FmaType::mrow;
}

bool FmaRow::isDynChild()
{
    return true;
}

void FmaRow::calculateSize()
{
    IFmaContainer::calculateSize();

    if(children.isEmpty()){
        QFont font = FmaTextFormats::getScriptFont(FmaTextFormats::NormalMV, scriptLevel());
        QFontMetricsF metrics = QFontMetricsF( font);
        qreal space = FmaTextFormats::spacing(FmaTextFormats::Thinmathspace, scriptLevel())*0.5;
        margin = QMargins(space, space, space, space);
        rect = metrics.boundingRect('H');
        baseUpHeight = rect.height()*0.5;
        baseHeight = rect.height();
    }else{
        margin = QMarginsF(0, 0, 0, 0);
        qreal width = 0;
        qreal maxUpHeight = 0;
        qreal maxDownHeight = 0;
        for (IFmaBase* eb : children)
        {
            width += eb->fullWidth();
            maxUpHeight = qMax(maxUpHeight, eb->upHeight());
            maxDownHeight = qMax(maxDownHeight, eb->downHeight());
        }

        baseUpHeight = maxUpHeight;
        baseHeight = maxUpHeight + maxDownHeight;
        rect.setWidth(width);
        rect.setHeight(baseHeight);
    }
}

void FmaRow::calculatePos()
{
    qreal x = rect.x();
    qreal centerV = rect.y() + upHeight();

    for (IFmaBase* eb : children)
    {
        x += eb->marginLeft();
        eb->rect.moveLeft(x);
        x += eb->rect.width() + eb->marginRight();
        eb->rect.moveTop(centerV - eb->upHeight() + eb->marginTop());
    }

    for (IFmaBase* eb : children)
    {
        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

qreal FmaRow::upHeight()
{
    if(baseHeight>0){
        return baseUpHeight/baseHeight * rect.height();
    }else{
        return IFmaContainer::upHeight();
    }
}

void FmaRow::paint(QPainter *painter)
{
    if(children.isEmpty()){
        painter->save();
        QPen pen(Qt::DotLine);
        pen.setWidthF(1.0);
        pen.setCosmetic(true); // 固定 1 设备像素，放大/高 DPI 下不变粗
        painter->setPen(pen);
        painter->drawRect(rect);
        painter->restore();
    }else{
        IFmaContainer::paint(painter);
    }
}

QList<std::shared_ptr<FmaEditOp>> FmaRow::doInput(const QChar ch)
{
    FmaText* textRow = rowCurrentText();
    if(textRow == nullptr){
        textRow = new FmaText(FmaType::munknown, ch, FmaOperDict::mmlNofix);
        return putToActive(textRow, true, NoMerge);
    }else{
        if(textRow->text.startsWith("&")){
            if(textRow->text.endsWith(";")){
                textRow = new FmaText(FmaType::munknown, ch, FmaOperDict::mmlNofix);
                return putToActive(textRow, true, NoMerge);
            }else{
                QList<std::shared_ptr<FmaEditOp>> ret;
                std::shared_ptr<FmaTextInputDelete> op
                        = std::make_shared<FmaTextInputDelete>(true);
                textRow->buildTextIndexes(op->editItemIndex);
                op->text = ch;
                ret.append(op);

                textRow->input(ch);
                return ret;
            }
        }else{
            FmaText* t = nullptr;
            if(ch == '&'){
                t = new FmaText(FmaType::munknown, ch, FmaOperDict::mmlNofix);
            }else{
                FmaOperDict::Form form;
                FmaType type = parseTextType(ch, form);
                switch(type){
                case FmaType::mo:{
                    t = new FmaText(type, ch, FmaOperDict::mmlNofix);
                    break;
                }
                default:{
                    if(textRow->type() == type){
                        QList<std::shared_ptr<FmaEditOp>> ret;
                        std::shared_ptr<FmaTextInputDelete> op
                                = std::make_shared<FmaTextInputDelete>(true);
                        textRow->buildTextIndexes(op->editItemIndex);
                        op->text = ch;
                        ret.append(op);

                        textRow->input(ch);
                        return ret;
                    }else{
                        t = new FmaText(type, ch, FmaOperDict::mmlNofix);
                    }
                    break;
                }
                }
            }

            if(t){
                return putToActive(t, true, NoMerge);
            }
        }
    }
    return QList<std::shared_ptr<FmaEditOp>>();
}


QList<std::shared_ptr<FmaEditOp>> FmaRow::putToActive(IFmaBase *child, bool isForward, MergeType mergeType)
{
    QList<std::shared_ptr<FmaEditOp>> ret;

    int pos = -1;
    if(!isValid(activeChild)){
        if(!children.isEmpty()){
            if(isStart(activeChild)){
                if(activeChild->isText()){
                    activeChild = children[0];
                    activeChild->moveCursor(QTextCursor::StartOfBlock);
                }else{
                    pos = 0;
                }
            }else{
                activeChild = children[children.size()-1];
                activeChild->moveCursor(QTextCursor::EndOfBlock);
            }
        }
    }

    if(isValid(activeChild)){
        bool isSplitPos = false;

        if(activeChild->isText()){
            FmaText* textRow = (FmaText *)activeChild;
            if(textRow != nullptr){
                if(!textRow->text.startsWith("&")){
                    if(mergeType != NoMerge){
                        if(child->type() == textRow->type()){
                            if(child->type() != FmaType::mo){

                                std::shared_ptr<FmaTextInputDelete> op
                                        = std::make_shared<FmaTextInputDelete>(true);
                                textRow->buildTextIndexes(op->editItemIndex);
                                op->text = ((FmaText*)child)->text;

                                textRow->input(op->text);
                                if(mergeType==MergeDelete){
                                    delete child;
                                }
                                ret.append(op);
                                return ret;
                            }
                        }
                    }

                    pos = children.indexOf(activeChild);
                    isSplitPos = true;
                    if(pos >=0){
                        QString prev = textRow->text.left(textRow->activePos());
                        QString next = textRow->text.mid(textRow->activePos());

                        if(prev.isEmpty()){
                        }else{
                            if(!next.isEmpty()){
                                std::shared_ptr<FmaTextMergeSplit> op
                                        = std::make_shared<FmaTextMergeSplit>(false);
                                textRow->buildTextIndexes(op->mergeItemIndex);
                                op->text = next;
                                op->hasSplit = true;
                                ret.append(op);

                                textRow->text = prev;
                                textRow->reparse(FmaType::munknown, FmaOperDict::mmlNofix);
                                textRow->setActivePos(0);
                                textRow = new FmaText(FmaType::munknown, next, FmaOperDict::mmlNofix);
                                insert(textRow, pos+1);
                            }

                            if(isForward){
                                pos +=1;
                            }
                        }
                    }
                }
            }
        }

        if(!isSplitPos){
            pos = children.indexOf(activeChild);
            if(!activeChild->isAtStart()){
                if(isForward){
                    pos +=1;
                }
            }
        }
    }

    if(pos <0)
    {
        if(isForward){
            pos = children.size();
        }else{
            pos = 0;
        }
    }

    IFmaBase *mergeItem = nullptr;
    if((child->type() == type()) && (mergeType != NoMerge)){

        std::shared_ptr<FmaItemInputDelete> op
                = std::make_shared<FmaItemInputDelete>(true);
        buildIndexes(op->editContainerIndex);
        op->pos = pos;
        op->inputCbor = ((FmaRow*)child)->childrenCbor();

        ret.append(op);
        mergeItem = merge((FmaRow*)child, pos, mergeType==MergeDelete);
        if(mergeItem){
            op->validNum = indexOfChild(mergeItem)-pos +1;
            ret.append(op);
        }
    }

    if(mergeItem == nullptr){

        if(insert(child, pos)){
            activeChild = child;
            activeChild->moveCursor(QTextCursor::EndOfBlock);

            std::shared_ptr<FmaItemInputDelete> op
                    = std::make_shared<FmaItemInputDelete>(true);
            buildIndexes(op->editContainerIndex);
            op->pos = pos;
            op->inputCbor.append(child->toCbor());
            op->validNum = 1;
            ret.append(op);
        }
    }else{
        activeChild = mergeItem;
        activeChild->moveCursor(QTextCursor::EndOfBlock);
    }
    return ret;
}

bool FmaRow::isSingleText()
{
    if(children.size() == 1){
        return children[0]->isText();
    }
    return false;
}

IFmaBase* FmaRow::merge(FmaRow *row, int pos, bool deleteMerge)
{
    if(isAncestor(row)){
        return nullptr;
    }

    if(row && (row != this)){
        if((pos >children.size()) || (pos <0)){
            if(deleteMerge){
                if(row->parent != nullptr){
                    row->parent->deleteChild(row);
                }else{
                    delete row;
                }
            }
            return nullptr;
        }

        IFmaBase *lastItem = nullptr;
        QList<IFmaBase*> tmp = row->children;  //需要复制一下，否则insert中删除操作take，循环会产生问题
        for(IFmaBase* eb : tmp){
            if(insert(eb, pos++)){
                lastItem = eb;
            }
        }

        if(deleteMerge){
            if(row->parent != nullptr){
                row->parent->deleteChild(row);
            }else{
                delete row;
            }
        }else{
            if(row->parent != nullptr){
                row->parent->take(row);
            }
        }

        return lastItem;
    }
    return nullptr;
}

bool FmaRow::append(IFmaBase *child)
{
    return insert(child, children.size());
}

bool FmaRow::prepend(IFmaBase *child)
{
    return insert(child, 0);
}

bool FmaRow::setToRowChild(IFmaBase *t, int index)
{
    if(children.size() > index){
        return IFmaContainer::setToRowChild(t, index);
    }else if(children.size() == index){
        return insert(t, index);
    }else{
        return false;
    }
}

FmaText *FmaRow::blockCurrentText()
{
    IFmaBase* ch = activeChild;
    while(isValid(ch)){
        if(ch->isContainer()){
            ch = ((IFmaContainer*)ch)->getActiveChild();
        }else{
            if(ch->isText()){
                return (FmaText *)ch;
            }
        }
    }
    return nullptr;
}

FmaText *FmaRow::rowCurrentText()
{
    if(isValid(activeChild)){
        if(activeChild->isText()){
            return (FmaText *)activeChild;
        }
    }

    return nullptr;
}

FmaText *FmaRow::rowFirstText()
{
    for (IFmaBase* eb : children)
    {
        if(eb->isText()){
            return (FmaText *)eb;
        }
    }
    return nullptr;
}

FmaText *FmaRow::rowNextText()
{
    if(isValid(activeChild)){
        if(activeChild->isText()){
            int i= children.indexOf(activeChild);
            if(i>=0 && (i+1) < children.size()){
                IFmaBase* eb = children[i+1];
                if(eb->isText()){
                    return (FmaText *)eb;
                }
            }
        }
    }
    return nullptr;
}

FmaText *FmaRow::rowPreviousText()
{
    if(isValid(activeChild)){
        if(activeChild->isText()){
            int i= children.indexOf(activeChild);
            if((i-1)>=0){
                IFmaBase* eb = children[i-1];
                if(eb->isText()){
                    return (FmaText *)eb;
                }
            }
        }
    }
    return nullptr;
}

FmaText *FmaRow::rowLastText()
{
    QList<IFmaBase*>::reverse_iterator iter = children.rbegin();
    for (; iter != children.rend(); ++iter)
    {
        if((*iter)->isText()){
            return (FmaText *)(*iter);
        }
    }
    return nullptr;
}


bool FmaRow::mouseClick(const QPointF &p)
{
    activeChild = nullptr;
    qreal s = rect.left();
    qreal e = s;
    for (IFmaBase* eb : children)
    {
        e = eb->rect.right() + eb->marginRight();
        if ((p.x() >= s) &&
                (p.x() <= e))
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
        if(p.x() > midX()){
            moveToEnd();
        }else{
            moveToStart();
        }
        return true;
    }
}

bool FmaRow::moveCursor(QTextCursor::MoveOperation op)
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
    case QTextCursor::StartOfBlock:
    case QTextCursor::StartOfWord:{
        moveToStart();
        return true;
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
        if(isValid(t) && t != activeChild){
            bool needMoveMore = false;
            if(isAtEnd()){
                needMoveMore = true;
            }else if(isValid(activeChild)){
                if(activeChild->isRow()){
                    needMoveMore = true;
                }else if(activeChild->isText()){
                    needMoveMore = true;
                }else if(activeChild->isAtStart()){
                    needMoveMore = true;
                }
            }

            activeChild = t;
            if(t->isContainer()){
                if(!t->isRow()){
                    IFmaContainer* c = (IFmaContainer*)t;
                    c->setActiveChild(nullptr);
                    if(needMoveMore){
                        return activeChild->moveCursor(QTextCursor::EndOfBlock);
                    }else{
                        return true;
                    }
                }
            }
            if(needMoveMore){
                return activeChild->moveCursor(op);
            }else{
                return activeChild->moveCursor(QTextCursor::EndOfBlock);
            }
        }
        break;
    }
    case QTextCursor::End:
    case QTextCursor::EndOfLine:
    case QTextCursor::EndOfBlock:
    case QTextCursor::EndOfWord:{
        moveToEnd();
        return true;
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
        if(isValid(t) && t != activeChild){
            bool needMoveMore = false;
            if(isAtStart()){
                needMoveMore = true;
            }else if(isValid(activeChild)){
                if(activeChild->isRow()){
                    needMoveMore = true;
                }else if(activeChild->isText()){
                    needMoveMore = true;
                }else if(activeChild->isAtEnd()){
                    needMoveMore = true;
                }
            }

            activeChild = t;
            if(t->isContainer()){
                if(!t->isRow()){
                    IFmaContainer* c = (IFmaContainer*)t;
                    c->setActiveChild(nullptr);
                    if(needMoveMore){
                        return activeChild->moveCursor(QTextCursor::StartOfBlock);
                    }else{
                        return true;
                    }
                }
            }
            if(needMoveMore){
                return activeChild->moveCursor(op);
            }else{
                return activeChild->moveCursor(QTextCursor::StartOfBlock);
            }
        }
        break;
    }
    default:{
        break;
    }
    }
    return false;
}



