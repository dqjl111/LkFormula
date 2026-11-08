#ifndef FMAROW_H
#define FMAROW_H

#include "ifmacontainer.h"
#include "cmd/fmaeditop.h"

class FmaText;
class FmaRow : public IFmaContainer
{
public:
    FmaRow(IFmaBase* child = nullptr);
    FmaRow(const QCborMap& cborMap);


    virtual bool isRow() final;

    virtual FmaType type() override;
    virtual bool isDynChild() override;

    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    virtual void paint( QPainter *painter) override;

    QList<std::shared_ptr<FmaEditOp>> doInput(const QChar ch);

    enum MergeType{NoMerge, Merge, MergeDelete};
    //enum PutRowType{EditText, InsertOp, DeleteOp};
    QList<std::shared_ptr<FmaEditOp>> putToActive(IFmaBase* child,bool isForward=true, MergeType mergeType =NoMerge);

    bool append(IFmaBase* child);
    bool prepend(IFmaBase* child);

    virtual bool setToRowChild(IFmaBase* t, int index) override;

    bool isSingleText();
    IFmaBase* merge(FmaRow* row, int pos = 0, bool deleteMerge = true);
    FmaText* blockCurrentText();

    FmaText* rowCurrentText();
    FmaText* rowFirstText();
    FmaText* rowNextText();
    FmaText* rowPreviousText();
    FmaText* rowLastText();

    virtual bool mouseClick(const QPointF& p) override;
    virtual bool moveCursor(QTextCursor::MoveOperation op) override;

    QMarginsF margin = QMarginsF(0, 0, 0, 0);
    virtual qreal marginLeft() const override { return margin.left();};
    virtual qreal marginTop() const override { return margin.top();};
    virtual qreal marginRight() const override { return margin.right();};
    virtual qreal marginBottom() const override { return margin.bottom();};

protected:
    qreal baseUpHeight=0;
    qreal baseHeight=0;
};


#endif // FMAROW_H
