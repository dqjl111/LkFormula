#ifndef FMAROWS_H
#define FMAROWS_H

#include "ifmacontainer.h"

class FmaRow;
class FmaRows : public IFmaContainer
{
public:
    FmaRows();

    void fromCborMap(const QCborMap& cborMap);

    virtual bool isDynChild() override;

    bool append(FmaRow* child);
    bool prepend(FmaRow* child);
    virtual bool insert(IFmaBase* child, int pos = 0) override;

    virtual bool setToRowChild(IFmaBase* t, int index) override;

    FmaRow* currentRow();
    FmaRow* firstRow();
    FmaRow* nextRow();
    FmaRow* previousRow();
    FmaRow* lastRow();


    virtual void calculateSize() override;
    virtual void calculatePos() override;

    virtual bool mouseClick(const QPointF& p) override;
    virtual bool moveCursor(QTextCursor::MoveOperation op) override;
protected:
    virtual bool mouseClickIgnoreWidth(const QPointF& p);
};

#endif // FMAROWS_H
