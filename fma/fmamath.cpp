#include "fmamath.h"
#include "fmarow.h"
#include "fmatext.h"
#include "fmadoc.h"
#include <QPainter>
#include <QPaintEngine>
#include <QDebug>

FmaMath::FmaMath()
{
    hCaret.isHorizontal = true;
}

FmaType FmaMath::type()
{
    return FmaType::math;
}

void FmaMath::calculateRect()
{
    rect.setLeft(marginLeft());
    rect.setTop(marginBottom());
    FmaRows::calculateRect();
    updateCaret();
}


void FmaMath::paint(QPainter *painter)
{
    paintSelect(painter);
    paintChilds(painter);
    if(isValid(activeChild)){
        paintCursor(painter);
    }
}

void FmaMath::paintCursor(QPainter *p)
{
    if(cursorOn){
        p->save();
        bool toggleAntialiasing = !(p->renderHints() & QPainter::Antialiasing)
                                  && (p->transform().type() > QTransform::TxTranslate);
        if (toggleAntialiasing){
            p->setRenderHint(QPainter::Antialiasing);
        }
        QPainter::CompositionMode origCompositionMode = p->compositionMode();
        if (p->paintEngine()->hasFeature(QPaintEngine::RasterOpModes)){
            p->setCompositionMode(QPainter::RasterOp_NotDestination);
        }

        p->fillRect(vCaret.caretRect(), p->pen().brush());
        p->fillRect(hCaret.caretRect(), p->pen().brush());
        p->setCompositionMode(origCompositionMode);
        if (toggleAntialiasing){
            p->setRenderHint(QPainter::Antialiasing, false);
        }
        p->restore();
    }
}

void FmaMath::paintChilds(QPainter *painter)
{
    for (IFmaBase* eb : children)
    {
        eb->paint(painter);
    }
}

QList<std::shared_ptr<FmaEditOp>> FmaMath::doInput(const QString &text)
{
    QList<std::shared_ptr<FmaEditOp>> ret;
    if(!text.isEmpty()){
        bool prevEnter = false;
        for(QChar ch :text){
            if(ch == "\r"){
                prevEnter = true;
                ret.append(enterNewRow());
            }else if(ch == "\n"){
                if(prevEnter){
                    prevEnter = false;
                }else{
                    ret.append(enterNewRow());
                }
            }else{
                prevEnter = false;
                FmaRow *curRow = activeRow(true);
                ret.append(curRow->doInput(ch));
            }
        }
    }
    return ret;
}

QList<std::shared_ptr<FmaEditOp>> FmaMath::doInput(IFmaBase *item)
{
    if(item != nullptr){
        FmaRow *curRow = activeRow(true);
        return curRow->putToActive(item, true, FmaRow::MergeDelete);
    }
    return QList<std::shared_ptr<FmaEditOp>>();
}

QList<std::shared_ptr<FmaEditOp>> FmaMath::doDelete(bool isForward)
{
    if(cursorState == NoSelect){
        markSelectStart();
        if(isForward){
            moveCursor(QTextCursor::NextWord);
        }else{
            moveCursor(QTextCursor::PreviousWord);
        }
        QVector<int> start = selectStartIndexes();
        QVector<int> end = activeIndexes();

        doSelectRange(start, end);
    }
    return removeSelection();
}

QList<std::shared_ptr<FmaEditOp>> FmaMath::doPaste(const QCborMap& cbor)
{
    QList<std::shared_ptr<FmaEditOp>> ret;

    QCborMap::const_iterator iter = IFmaBase::findCbor(cbor, int(FmaCborId::Type));
    if(iter == cbor.end()){
        return ret;
    }

    FmaType t = (FmaType)(iter.value().toInteger());
    if(t == type()){
        QCborMap::const_iterator mapIter = findCbor(cbor, int(FmaCborId::Children));
        if(mapIter != cbor.end()){
            bool hasInput = false;

            const QCborArray cborArr = mapIter.value().toArray();
            QCborArray::const_iterator iter = cborArr.begin();
            for(; iter != cborArr.end(); ++iter){
                IFmaBase* b= FmaDoc::fromCbor(iter->toMap());
                if(b){
                    if(hasInput){
                        ret.append(enterNewRow());
                    }
                    ret.append(doInput(b));
                    hasInput = true;
                }
            }
        }
    }else{
        IFmaBase *b = FmaDoc::fromCbor(cbor, nullptr);
        ret.append(doInput(b));
    }
    return ret;
}

bool FmaMath::doDelete(const QVector<int> &indexes)
{
    FmaIndexItem item = indexItem(indexes);
    if(item.baseItem){
        if((item.validLevel+1) == indexes.size()){
            if(item.textPos>=0){
                return ((FmaText*)(item.baseItem))->deletePos(item.textPos);
            }else if(item.baseItem->parent){
                return item.baseItem->parent->deleteChild(item.baseItem);
            }
        }
    }
    return false;
}


void FmaMath::doSelectRange(const QVector<int> &start, const QVector<int> &end)
{
    bool isLeftToRight = compareIndexes(start, end) <= 0;
    if(isLeftToRight){
        selectRange(start, end, 0, quint8(SelectToRight));
    }else{
        selectRange(start, end, 0, quint8(SelectToLeft));
    }
}


void FmaMath::updateCaret()
{
    FmaRow *curRow = activeRow();
    if(curRow){
        FmaText * textRow = curRow->blockCurrentText();
        if(textRow!= nullptr){
            IFmaContainer* p = textRow->parent;

            hCaret.pos.setX(p->rect.x());
            hCaret.pos.setY(p->rect.y() + p->rect.height());

            QRectF activeRect = textRow->textRect(textRow->activePos());
            vCaret.pos.setX(activeRect.x() + activeRect.width());
            vCaret.pos.setY(activeRect.y());

            vCaret.length = hCaret.pos.y() - vCaret.pos.y();
            hCaret.length = p->rect.width();
        }else{
            if(curRow->children.empty()){
                hCaret.pos.setX(curRow->rect.x());
                hCaret.pos.setY(curRow->rect.y() + curRow->rect.height()-1);

                vCaret.pos.setX(curRow->rect.x() + 1);
                vCaret.pos.setY(curRow->rect.y());

                vCaret.length = hCaret.pos.y() - vCaret.pos.y();
                hCaret.length = curRow->rect.width() -1;
            }else{
                hCaret.pos.setX(curRow->rect.x());
                hCaret.pos.setY(curRow->rect.y() + curRow->rect.height());

                if(curRow->isAtStart()){
                    vCaret.pos.setX(curRow->rect.x());
                }
                else if(curRow->isAtEnd()){
                    vCaret.pos.setX(curRow->rect.x() + curRow->rect.width());
                }else{
                    if(curRow->activeChild->isAtEnd()){
                        vCaret.pos.setX(curRow->activeChild->rect.x()
                                        + curRow->activeChild->rect.width()
                                        + curRow->activeChild->marginRight());
                    }else{
                        vCaret.pos.setX(curRow->activeChild->rect.x());
                    }
                }
                vCaret.pos.setY(curRow->rect.y());

                vCaret.length = hCaret.pos.y() - vCaret.pos.y();
                hCaret.length = curRow->rect.width();
            }
        }
    }
}


FmaRow *FmaMath::newRow(int pos)
{
    FmaRow* row = new FmaRow;
    insert(row, pos);
    return row;
}

std::shared_ptr<FmaRowInputDelete> FmaMath::enterNewRow()
{
    if(!isValid(activeChild)){
        if(!children.isEmpty()){
            if(activeChild == nullptr){
                activeChild = children[children.size()-1];
                activeChild->moveToEnd();
            }else{
                activeChild = children[0];
                activeChild->moveToStart();
            }

        }
    }

    if(isValid(activeChild)){
        int row = children.indexOf(activeChild);
        activeChild = newRow(row+1);
    }else{
        activeChild = newRow(0);
    }

    std::shared_ptr<FmaRowInputDelete> op
            = std::make_shared<FmaRowInputDelete>(true);
    activeChild->buildIndexes(op->editRowIndex);

    return op;
}


FmaRow *FmaMath::activeRow(bool create)
{
    FmaRow* curRow = nullptr;
    if(isValid(activeChild)){
        curRow = (FmaRow*)activeChild;
    }
    if(curRow){
        IFmaBase *t = curRow->getActiveChild();
        while(isValid(t)){
            if(t->isRow()){
                curRow = (FmaRow*)t;
                t = curRow->getActiveChild();
            }else if(t->isContainer()){
                t = ((IFmaContainer*)t)->getActiveChild();
            }else{
                return curRow;
            }
        }
    }else{
        if(create){
            if(children.isEmpty()){
                activeChild = newRow(0);
            }else{
                activeChild = children[0];
            }
            curRow = (FmaRow*)activeChild;
        }
    }
    return curRow;
}

FmaText *FmaMath::activeText()
{
    FmaRow* curRow = activeRow();
    if(curRow){
        return curRow->rowCurrentText();
    }
    return nullptr;
}

IFmaBase *FmaMath::activeBlock(bool create)
{
    IFmaContainer* c = nullptr;
    if(isValid(activeChild)){
        c = (IFmaContainer*)activeChild;
    }

    if(c){
        IFmaBase *t = c->getActiveChild();
        while(isValid(t)){
            if(t->isContainer()){
                c = ((IFmaContainer*)t);
                t = c->getActiveChild();
            }else{
                return t;
            }
        }
        return c;
    }else{
        if(create){
            if(children.isEmpty()){
                activeChild = newRow(0);
            }
            c = (FmaRow*)activeChild;
        }
    }
    return c;
}

bool FmaMath::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    bool ret = false;
    if(mode == QTextCursor::KeepAnchor){
        markSelectStart();
        ret = moveCursor(op);
        QVector<int> start = selectStartIndexes();
        QVector<int> end = activeIndexes();
        doSelectRange(start, end);
    }else{
        deSelect();
        ret = moveCursor(op);
    }

    updateCaret();
    return ret;
}

bool FmaMath::mouseClick(const QPointF &p)
{
    deSelect();
    QRectF r = bounds();
    if((r.top() <= p.y()) && (r.bottom() >= p.y())){
        FmaRows::mouseClickIgnoreWidth(p);
    }else{
        moveToEnd();
    }
    updateCaret();
    return true;
}

bool FmaMath::mouseRelese(const QPointF &p)
{
    markSelectStart();
    bool ret = FmaRows::mouseClick(p);
    if(isValid(activeChild)){
        QVector<int> start = selectStartIndexes();
        QVector<int> end = activeIndexes();
        doSelectRange(start, end);
    }else{
        deSelect();
    }
    updateCaret();
    return ret;
}

bool FmaMath::mouseSelectMove(const QPointF &p)
{
    markSelectStart();
    bool ret = FmaRows::mouseClick(p);
    if(isValid(activeChild)){
        QVector<int> start = selectStartIndexes();
        QVector<int> end = activeIndexes();
        doSelectRange(start, end);
    }
    updateCaret();
    return ret;
}

QVector<int> FmaMath::selectStartIndexes()
{
    QVector<int> ret;

    if(cursorState & PartialSelect){
        int startId = partSelectStartId;
        IFmaContainer* c = this;

        while(c){
            if(startId >=0 && startId< c->children.size()){
                ret.append(startId);
                IFmaBase* b = c->children[startId];
                if(b->cursorState & PartialSelect){
                    if(b->isContainer()){
                        c = (IFmaContainer*)b;
                        startId = c->partSelectStartId;
                    }else if(b->isText()){
                        startId = b->partSelectStartId;
                        ret.append(startId);
                        break;
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }else{
                if(startId<0){
                    ret.append(-1);
                }
                break;
            }
        }
    }
    return ret;
}

QVector<int> FmaMath::selectEndIndexes()
{
    QVector<int> ret;

    if(cursorState & PartialSelect){
        int endId = partSelectEndId;
        IFmaContainer* c = this;

        while(c){
            if(endId >=0 && endId< c->children.size()){
                ret.append(endId);
                IFmaBase* b = c->children[endId];
                if(b->isContainer()){
                    c = (IFmaContainer*)b;
                    endId = c->partSelectEndId;
                }else if(b->isText()){
                    endId = b->partSelectEndId;
                    ret.append(endId);
                    break;
                }else{
                    break;
                }
            }else{
                if(endId<0){
                    ret.append(-1);
                }
                break;
            }
        }
    }
    return ret;
}

QVector<int> FmaMath::activeIndexes()
{
    QVector<int> ret;
    IFmaBase* b = activeChild;
    IFmaContainer* c = this;
    while(b){
        if(isStart(b)){
            ret.append(-1);
            break;
        }else{
            int index = c->children.indexOf(b);
            if(index>=0){
                ret.append(index);
                if(b->isContainer()){
                    c = (IFmaContainer*)b;
                    b = c->activeChild;
                }else if(b->isText()){
                    FmaText* t = (FmaText*) b;
                    ret.append(t->m_activePos);
                    break;
                }else{
                    break;
                }
            }else{
                break;
            }
        }
    }
    return ret;
}

bool FmaMath::setActiveIndexes(const QVector<int> &indexes)
{
    activeChild = nullptr;
    FmaText* b = nullptr;
    IFmaContainer* c = this;
    int index = 0;
    int i=0;
    for(; i<indexes.size(); ++i){
        index = indexes[i];
        if(b!= nullptr){
            b->setActivePos(index);
            break;
        }else{
            if(c->children.size()> index && index>=0){
                c->activeChild = c->children[index];
                if(c->activeChild->isContainer()){
                    c = (IFmaContainer*)(c->activeChild);
                    c->setActiveChild(nullptr);
                }else if(c->activeChild->isText()){
                    b = (FmaText*)(c->activeChild);
                }else{
                    return false;
                }
            }else{
                if(index <0){
                    c->activeChild = &startBase;
                }else{
                    c->activeChild = nullptr;
                }
                return true;
            }
        }
    }
    return i==indexes.size();
}

FmaIndexItem FmaMath::indexItem(const QVector<int> &indexes)
{
    FmaIndexItem ret;
    if(indexes.isEmpty()){
        ret.baseItem = this;
        return ret;
    }

    if(indexes[0]<0){
        ret.baseItem = &startBase;
        return ret;
    }

    if(indexes[0] >=children.size()){
        return ret;
    }

    IFmaBase* t = this;
    FmaText* b = nullptr;
    IFmaContainer* c = this;
    int index = 0;
    int i=0;
    for(; i<indexes.size(); ++i){
        ret.validLevel = i;
        index = indexes[i];

        if(c->children.size()> index && index>=0){
            t = c->children[index];
            if(t->isContainer()){
                c = (IFmaContainer*)(t);
                ret.baseItem = c;
            }else if(t->isText()){
                b = (FmaText*)(t);
                ret.baseItem = b;
                ++i;
                if(indexes.size() > i){
                    ret.validLevel = i;
                    index = indexes[i];
                    if(index>=0 && index<=b->text.size()){
                        ret.textPos = index;
                    }
                }
                return ret;
            }else{
                ret.baseItem = t;
                return ret;
            }
        }else{
            if(index <0){
                ret.baseItem = &startBase;
            }else{
                ret.baseItem = nullptr;
            }
            break;
        }
    }
    return ret;
}

QVector<int> FmaMath::selectItemIndexes()
{
    QVector<int> ret;

    if(cursorState & PartialSelect){
        int startId = partSelectStartId;
        int endId = partSelectEndId;
        IFmaContainer* c = this;

        while(c){
            if(startId == endId){
                if(startId >=0 && startId< c->children.size()){
                    ret.append(startId);
                    IFmaBase* b = c->children[startId];
                    if(b->cursorState & PartialSelect){
                        if(b->isContainer()){
                            c = (IFmaContainer*)b;
                            startId = c->partSelectStartId;
                            endId = c->partSelectEndId;
                        }else if(b->isText()){
                            ret.append(b->partSelectStartId);
                            break;
                        }else{
                            break;
                        }
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            }else{
                break;
            }
        }
    }
    return ret;
}


QRectF FmaCaret::caretRect()
{
    if(isHorizontal){
        return QRectF(pos.x(), pos.y(), length, width);
    }else{
        return QRectF(pos.x(), pos.y(), width, length);
    }
}
