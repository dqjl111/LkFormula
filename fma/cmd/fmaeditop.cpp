#include "fmaeditop.h"
#include "fma/fmadoc.h"

FmaEditOp::FmaEditOp()
{

}

void FmaTextMergeSplit::undo(FmaMath* math)
{
    if(type == TextMerge){
        split(math);
    }else{
        merge(math);
    }
}

void FmaTextMergeSplit::redo(FmaMath* math)
{
    if(type == TextMerge){
        merge(math);
    }else{
        split(math);
    }
}

void FmaTextMergeSplit::split(FmaMath* math)
{
    if(math){
        FmaIndexItem item = math->indexItem(mergeItemIndex);
        if(item.baseItem){
            math->setActiveIndexes(mergeItemIndex);
            if(item.textPos>=0){
                FmaText* textItem = ((FmaText*)(item.baseItem));
                QString prev = textItem->text.left(textItem->activePos());
                text = textItem->text.mid(textItem->activePos());
                if(prev.isEmpty()){
                    hasSplit = false;
                    if(type == TextMerge){
                        textItem->text.clear();
                        textItem->setActivePos(0);
                    }else{
                        text.clear();
                        textItem->setActivePos(0);
                    }
                }else{
                    if(!text.isEmpty()){
                        textItem->text = prev;
                        textItem->reparse(FmaType::munknown, FmaOperDict::mmlNofix);
                        textItem->setActivePos(textItem->text.size());
                        textItem = new FmaText(FmaType::munknown, text, FmaOperDict::mmlNofix);

                        if(item.baseItem->parent){
                            if(item.baseItem->parent->isRow()){
                                FmaRow* row = (FmaRow*)(item.baseItem->parent);
                                int pos = row->indexOfChild(item.baseItem);
                                if(pos>=0){
                                    row->insert(textItem, pos+1);
                                }
                                hasSplit = true;
                            }
                        }
                    }else{
                        hasSplit = false;
                    }
                }
            }
        }
    }
}

void FmaTextMergeSplit::merge(FmaMath* math)
{
    if(math){
        FmaIndexItem item = math->indexItem(mergeItemIndex);
        if(item.baseItem){
            math->setActiveIndexes(mergeItemIndex);
            if(item.textPos>=0){
                if(hasSplit){
                    QVector<int> splitItemIndex;
                    for(int i=0; i<item.validLevel-2; ++i){
                        splitItemIndex.append(mergeItemIndex[i]);
                    }

                    if(item.validLevel>1){
                        splitItemIndex.append(mergeItemIndex[item.validLevel-1]+1);
                    }

                    FmaIndexItem splitItem = math->indexItem(splitItemIndex);
                    if(splitItem.baseItem){
                        if(splitItem.baseItem->isText()){
                            text = ((FmaText*)(splitItem.baseItem))->text;
                            if(splitItem.baseItem->parent){
                                splitItem.baseItem->parent->deleteChild(item.baseItem);
                            }else{
                                delete splitItem.baseItem;
                            }
                        }
                    }
                }

                FmaText* textItem = ((FmaText*)(item.baseItem));
                textItem->setActivePos(item.textPos);
                textItem->input(text);
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////
void FmaTextInputDelete::undo(FmaMath* math)
{
    if(type == TextInput){
        del(math);
    }else{
        input(math);
    }
}

void FmaTextInputDelete::redo(FmaMath* math)
{
    if(type == TextInput){
        input(math);
    }else{
        del(math);
    }
}

void FmaTextInputDelete::input(FmaMath* math)
{
    if(math){
        FmaIndexItem item = math->indexItem(editItemIndex);
        if(item.baseItem){
            math->setActiveIndexes(editItemIndex);
            if(item.textPos>=0){
                FmaText* textItem = ((FmaText*)(item.baseItem));
                textItem->setActivePos(item.textPos);
                textItem->input(text);
            }
        }
    }
}

void FmaTextInputDelete::del(FmaMath* math)
{
    if(math){
        FmaIndexItem item = math->indexItem(editItemIndex);
        if(item.baseItem){
            math->setActiveIndexes(editItemIndex);
            if(item.textPos>=0){
                FmaText* textItem = ((FmaText*)(item.baseItem));
                textItem->deletePos(item.textPos, text.size());
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////
void FmaItemInputDelete::undo(FmaMath* math)
{
    if(type == ItemInput){
        del(math);
    }else{
        input(math);
    }
}

void FmaItemInputDelete::redo(FmaMath* math)
{
    if(type == ItemInput){
        input(math);
    }else{
        del(math);
    }
}

void FmaItemInputDelete::input(FmaMath *math)
{
    if(math){
        FmaIndexItem item = math->indexItem(editContainerIndex);
        if(item.baseItem){
            math->setActiveIndexes(editContainerIndex);
            if(item.baseItem->isContainer()){
                IFmaContainer* c = (IFmaContainer*)item.baseItem;

                QCborArray::iterator iter = inputCbor.begin();

                int tPos = pos;
                IFmaBase* child = nullptr;
                for(; iter != inputCbor.end(); ++iter){
                    IFmaBase* child = FmaDoc::fromCbor(iter->toMap());
                    if(child){
                        c->insert(child, tPos++);
                    }
                }
                if(child){
                    c->setActiveChild(child);
                    child->moveCursor(QTextCursor::EndOfBlock);
                }
                validNum = tPos-pos;
            }
        }
    }
}

void FmaItemInputDelete::del(FmaMath *math)
{
    if(math){
        FmaIndexItem item = math->indexItem(editContainerIndex);
        if(item.baseItem){
            math->setActiveIndexes(editContainerIndex);
            if(item.baseItem->isContainer()){
                IFmaContainer* c = (IFmaContainer*)item.baseItem;
                if(inputCbor.isEmpty()){
                    inputCbor = c->childrenCbor(pos, validNum);
                }
                c->del(pos, validNum);
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////
void FmaRowInputDelete::undo(FmaMath *math)
{
    if(type == RowInput){
        del(math);
    }else{
        input(math);
    }
}

void FmaRowInputDelete::redo(FmaMath *math)
{
    if(type == RowInput){
        input(math);
    }else{
        del(math);
    }
}

void FmaRowInputDelete::input(FmaMath *math)
{
    if(math){
        if(editRowIndex.size()>0){
            if(editRowIndex[0]>=0){
                math->newRow(editRowIndex[0]);
            }else{
                math->newRow(0);
            }
        }else{
            math->newRow(0);
        }
    }
}

void FmaRowInputDelete::del(FmaMath *math)
{
    if(math){
        math->doDelete(editRowIndex);
    }
}
