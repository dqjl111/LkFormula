#include "fmatable.h"
#include "fmarow.h"
#include "fmatext.h"
#include <QDebug>

FmaTable::FmaTable(uint _rowNum, uint _colNum)
{
    unsigned int k =0;
    for(unsigned int i=0; i<_rowNum; ++i){
        for(unsigned int j=0; j<_colNum; ++j){
            children.append(new FmaRow);
            setLevelOffset(children[k], 0);
            children[k]->parent = this;
            k++;
        }
    }
    rowNum = _rowNum;
    colNum = _colNum;
}

FmaTable::FmaTable(const QCborMap &cborMap):IFmaContainer(cborMap)
{
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::RowNum));
    if(iter != cborMap.end()){
        rowNum = iter.value().toInteger();
    }

    iter = findCbor(cborMap, int(FmaCborId::ColumnNum));
    if(iter != cborMap.end()){
        colNum = iter.value().toInteger();
    }

    iter = findCbor(cborMap, int(FmaCborId::RowSpacing));
    if(iter != cborMap.end()){
        rowspacing = iter.value().toInteger();
    }

    iter = findCbor(cborMap, int(FmaCborId::ColumnSpacing));
    if(iter != cborMap.end()){
        columnspacing = iter.value().toInteger();
    }

    if(rowNum>0 && colNum >0){
        initFixNumChildren(cborMap, rowNum*colNum);
    }
}

FmaType FmaTable::type()
{
    return FmaType::mtable;
}

void FmaTable::calculateSize()
{
    IFmaContainer::calculateSize();
    qreal thick = FmaTextFormats::spacing(rowspacing, scriptLevel());

    qreal height = 0, width = 0;
    qreal colWidth = 0;
    qreal rowHeight = 0;
    FmaRow *cell = nullptr;
    for(uint i=0; i<rowNum; ++i){
        rowHeight = 0;
        for(uint j=0; j<colNum; ++j){
            cell = getCell(i, j);
            rowHeight = qMax(rowHeight, cell->fullHeight());
        }
        height += rowHeight;
        if((i+1) != rowNum){
            height += thick;
        }
    }

    thick = FmaTextFormats::spacing(columnspacing, scriptLevel());
    for(uint j=0; j<colNum; ++j){
        colWidth = 0;
        for(uint i=0; i<rowNum; ++i){
            cell = getCell(i, j);
            colWidth = qMax(colWidth, cell->fullWidth());
        }
        width += colWidth;
        if((j+1) != colNum){
            width += thick;
        }
    }

    rect.setHeight(height);
    rect.setWidth(width);
}

void FmaTable::calculatePos()
{
    //qWarning() << "Table rect:" << rect;

    qreal thick = FmaTextFormats::spacing(rowspacing, scriptLevel());
    QVector<qreal> heights;
    QVector<qreal> xs;
    QVector<qreal> widths;
    QVector<qreal> ys;

    qreal x = rect.x(), y = rect.y();
    qreal colWidth = 0;
    qreal rowHeight = 0;
    FmaRow *cell = nullptr;
    for(uint i=0; i<rowNum; ++i){
        rowHeight = 0;
        for(uint j=0; j<colNum; ++j){
            cell = getCell(i, j);
            rowHeight = qMax(rowHeight, cell->fullHeight());
        }
        heights.append(rowHeight);
        ys.append(y);
        y += rowHeight;
        if((i+1) != rowNum){
            y += thick;
        }
    }

    thick = FmaTextFormats::spacing(columnspacing, scriptLevel());
    for(uint j=0; j<colNum; ++j){
        colWidth = 0;
        for(uint i=0; i<rowNum; ++i){
            cell = getCell(i, j);
            colWidth = qMax(colWidth, cell->fullWidth());
        }
        widths.append(colWidth);
        xs.append(x);
        x += colWidth;
        if((j+1) != colNum){
            x += thick;
        }
    }

    qreal centerX =0, centerY =0;
    for(uint i=0; i<rowNum; ++i){
        for(uint j=0; j<colNum; ++j){
            cell = getCell(i, j);
            x = xs[j];
            y = ys[i];

            colWidth = widths[j];
            rowHeight = heights[i];

            centerX = x + colWidth/2;
            centerY = y + rowHeight/2;
            cell->rect.moveLeft(centerX - cell->rect.width()/2);
            cell->rect.moveTop(centerY - cell->rect.height()/2);
        }
    }

    for (IFmaBase* eb : children)
    {
        //qWarning() << "Cell rect:" << eb->rect;

        if(eb->isContainer()){
            ((IFmaContainer*)eb)->calculatePos();
        }
    }
}

bool FmaTable::mouseClick(const QPointF &p)
{
    if(!rect.contains(p)){
        return false;
    }

    // 空单元格的 rect 只是中间的小方块，按最近单元格中心命中，
    // 保证点击单元格区域任意位置都能进入对应单元格。
    FmaRow *best = nullptr;
    qreal bestDistance = -1;
    for(IFmaBase *child : children){
        if(!child){
            continue;
        }
        const QPointF center = child->rect.center();
        const qreal dx = center.x() - p.x();
        const qreal dy = center.y() - p.y();
        const qreal distance = dx * dx + dy * dy;
        if(bestDistance < 0 || distance < bestDistance){
            bestDistance = distance;
            best = static_cast<FmaRow*>(child);
        }
    }

    if(best){
        setActiveChild(best);
        best->mouseClick(p);
        return true;
    }
    return false;
}

QCborMap FmaTable::toCbor(bool filterSelect)
{
    QCborMap ret = IFmaContainer::toCbor(filterSelect);
    ret.insert(int(FmaCborId::RowNum), rowNum);
    ret.insert(int(FmaCborId::ColumnNum), colNum);
    ret.insert(int(FmaCborId::RowSpacing), rowspacing);
    ret.insert(int(FmaCborId::ColumnSpacing), columnspacing);
    return ret;
}

uint FmaTable::getRowNum()
{
    return rowNum;
}

uint FmaTable::getColNum()
{
    return colNum;
}

void FmaTable::setCell(uint row, uint col, const QString &text, FmaType type)
{
    setCell(row, col, new FmaText(type, text));
}

void FmaTable::setCell(uint row, uint col, IFmaBase *cell)
{
    if(row <rowNum && col <colNum){
        int index = row*colNum + col;
        if(setToRowChild(cell, row*colNum + col)){
            setLevelOffset(children[index], 0);
        }
    }
}

FmaRow *FmaTable::getCell(uint row, uint col)
{
    if(row <rowNum && col <colNum){
        return (FmaRow *)children[row*colNum + col];
    }
    return nullptr;
}

bool FmaTable::moveCursor(QTextCursor::MoveOperation op)
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

        IFmaBase *t = nullptr;
        if(isValid(activeChild)){
            int i= children.indexOf(activeChild);
            if(i>=0){
                int row = i/colNum;
                int col = i%colNum;

                if(op == QTextCursor::Up){
                    if(row == 0){
                        int prevRow = rowNum-1;
                        t = getCell(prevRow, col);
                    }else{
                        int prevRow = (row-1)%rowNum;
                        t = getCell(prevRow, col);
                    }
                }else{
                    int nextRow = (row+1)%rowNum;
                    t = getCell(nextRow, col);
                }
            }else{
                t = getCell(0, 0);
            }
        }else{
            t = getCell(0, 0);
        }


        if(isValid(t) && t != activeChild){
            activeChild = t;
            return activeChild->moveCursor(QTextCursor::End);
        }
        break;
    }
    default:{
        break;
    }
    }
    return IFmaContainer::moveCursor(op);
}
