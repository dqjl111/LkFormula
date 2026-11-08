#ifndef FMAMATH_H
#define FMAMATH_H

#include "fmarows.h"
#include "cmd/fmaeditop.h"
#include <QTextCursor>

class FmaCaret
{
public:
    FmaCaret(){}

    QPoint pos = QPoint(0,0);
    qreal length = 0;
    bool isHorizontal = false;
    int width = 1;

    QRectF caretRect();
};

class FmaIndexItem
{
public:
    IFmaBase* baseItem = nullptr;
    int textPos=-1;
    int validLevel=-1;
};


class FmaRow;
class FmaText;
class FmaDoc;
class FmaRowInputDelete;
class FmaMath : public FmaRows
{
public:
    FmaMath();

    virtual FmaType type() override;

    virtual void calculateRect() override;
    virtual void paint( QPainter *painter) override;
    void paintCursor(QPainter *painter);
    void paintChilds(QPainter *painter);
    QList<std::shared_ptr<FmaEditOp>> doInput(const QString& text);

    QList<std::shared_ptr<FmaEditOp>> doDelete(bool isForward = false);
    QList<std::shared_ptr<FmaEditOp>> doPaste(const QCborMap& cbor);

    std::shared_ptr<FmaRowInputDelete> enterNewRow();

    void doSelectRange(const QVector<int>& start, const QVector<int>& end);
    void updateCaret();

    FmaRow* activeRow(bool create = false);
    FmaText* activeText();
    IFmaBase* activeBlock(bool create = false);

    virtual bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode);
    virtual bool mouseClick(const QPointF& p) override;
    bool mouseRelese(const QPointF& p);
    bool mouseSelectMove(const QPointF& p);

    bool cursorOn = false;

    QVector<int> selectStartIndexes();
    QVector<int> selectEndIndexes();
    QVector<int> activeIndexes();

    bool setActiveIndexes(const QVector<int>& indexes);
    FmaIndexItem indexItem(const QVector<int>& indexes);
    QVector<int> selectItemIndexes();

//    virtual qreal marginLeft() const override{ return 4;};
//    virtual qreal marginTop() const override{ return 4;};
//    virtual qreal marginRight() const override{ return 4;};
//    virtual qreal marginBottom() const override{ return 4;};
protected:
    FmaCaret vCaret;
    FmaCaret hCaret;
    FmaRow* newRow(int pos = 0);
    QList<std::shared_ptr<FmaEditOp>> doInput(IFmaBase* item);
    bool doDelete(const QVector<int>& indexes);

    friend class FmaDoc;
    friend class FmaRowInputDelete;
 };

#endif // FMAMATH_H
