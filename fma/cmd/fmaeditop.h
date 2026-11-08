#ifndef FMAEDITOP_H
#define FMAEDITOP_H

#include <QUndoCommand>
#include <QVector>
#include <QCborArray>

class FmaMath;
class FmaEditOp
{
public:
    FmaEditOp();
    enum Type{TextMerge, TextSplit,
              TextInput, TextDelete,
              ItemInput, ItemDelete,
              RowInput, RowDelete};
    virtual void undo(FmaMath* math)=0;
    virtual void redo(FmaMath* math)=0;
protected:
    int type = TextMerge;
    bool isDone = false;
};

class FmaTextMergeSplit : public FmaEditOp{
public:
    FmaTextMergeSplit(bool isMerge = true){
        if(isMerge){
            type = TextMerge;
        }else{
            type = TextSplit;
        }
    }

    QVector<int> mergeItemIndex;
    QString text;
    bool hasSplit = false;

    virtual void undo(FmaMath* math) override;
    virtual void redo(FmaMath* math) override;
protected:
    void split(FmaMath* math);
    void merge(FmaMath* math);
};

class FmaTextInputDelete: public FmaEditOp{
public:
    FmaTextInputDelete(bool isInput = true){
        if(isInput){
            type = TextInput;
        }else{
            type = TextDelete;
        }
    }

    QVector<int> editItemIndex;
    QString text;

    virtual void undo(FmaMath* math) override;
    virtual void redo(FmaMath* math) override;
protected:
    void input(FmaMath* math);
    void del(FmaMath* math);
};

class FmaItemInputDelete : public FmaEditOp{
public:
    FmaItemInputDelete(bool isInput = true){
        if(isInput){
            type = ItemInput;
        }else{
            type = ItemDelete;
        }
    }

    QVector<int> editContainerIndex;
    int pos=0;
    int validNum=1;
    QCborArray inputCbor;

    virtual void undo(FmaMath* math) override;
    virtual void redo(FmaMath* math) override;
protected:
    void input(FmaMath* math);
    void del(FmaMath* math);
};

class FmaRowInputDelete : public FmaEditOp{
public:
    FmaRowInputDelete(bool isInput = true){
        if(isInput){
            type = RowInput;
        }else{
            type = RowDelete;
        }
    }

    QVector<int> editRowIndex;
    virtual void undo(FmaMath* math) override;
    virtual void redo(FmaMath* math) override;
protected:
    void input(FmaMath* math);
    void del(FmaMath* math);
};

#endif // FMAEDITOP_H
