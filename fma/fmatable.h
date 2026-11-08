#ifndef FMATABLE_H
#define FMATABLE_H

#include "ifmacontainer.h"
#include "fmatextformats.h"

class FmaTable: public IFmaContainer
{
public:
    FmaTable(uint _rowNum, uint _colNum);
    FmaTable(const QCborMap& cborMap);

    int rowspacing = FmaTextFormats::Nonespace;
    int columnspacing = FmaTextFormats::Veryverythickmathspace;

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual QCborMap toCbor(bool filterSelect = false) override;

    uint getRowNum();
    uint getColNum();

    void setCell(uint row, uint col, const QString& text, FmaType type = FmaType::munknown);
    void setCell(uint row, uint col, IFmaBase* cell);
    FmaRow* getCell(uint row, uint col);

    virtual bool moveCursor(QTextCursor::MoveOperation op) override;
protected:
    unsigned int rowNum=0, colNum=0;
};

#endif // FMATABLE_H
