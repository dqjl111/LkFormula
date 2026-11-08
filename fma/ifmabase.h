#ifndef IFMABASE_H
#define IFMABASE_H

#include <QString>
#include <QJsonObject>
#include <QCborValue>
#include <QColor>
#include <QRectF>
#include <QFont>
#include <QTextCursor>
#include "fmaoperdict.h"
#include "fma/cmd/fmaeditop.h"
#include "wmf/fmawmf.h"

namespace fmaCon{
const static qreal defaultThickFactor = 1;
const static qreal maxThick = 3;
}

class FmaNodeType
{
    Q_GADGET
public:
    enum Type{
        mno = 0, mi, mn, mo, mfrac, mrow,
        msqrt, mroot, msup, msub, msubsup,
        mstyle, mphantom, mfenced,
        mtable, mtr, mtd, mover, munder,
        munderover, mtext, mpadded, mspace,
        malignmark, menclose, mstack, mlongdiv,
        mmultiscripts, math, merror, munknown
    };
    Q_ENUM(Type);

    static QString toString(quint16 type);
    static Type fromString(const QString& key);
};

typedef FmaNodeType::Type FmaType;

enum class FmaProp{
Scriptlevel,
Displaystyle,
Scriptsizemultiplier,
Scriptminsize,
Infixlinebreakstyle,
Decimalpoint
};

enum class FmaLineBreakStyle{
Before,
After,
Duplicate
};

enum class FmaCborId{
Type,
Style,
Text,
Children,
Notation,
Bevelled,
RowNum,
ColumnNum,
RowSpacing,
ColumnSpacing,
Dir,
Open,
Close
};

//class FmaSelectUnit
//{
//public:
//    int partSelectStartId = 0;
//    int partSelectEndId = 0;
//    QCborMap bodyCbor;
//};

class IFmaContainer;
class FmaMath;
class FmaOpsCommand;
class IFmaBase
{
public:
    IFmaBase();
    IFmaBase(const QCborMap& cborMap);

    virtual ~IFmaBase();

    static int compareIndexes(const QVector<int>& i1, const QVector<int>& i2);
    static FmaType parseTextType(const QString& t, FmaOperDict::Form& form);

    virtual FmaType type();
    virtual QCborMap toCbor(bool filterSelect = false);
    void parseBase(const QCborMap& cborMap);
    void parseStyle(const QCborMap& styleMap);

//    virtual std::shared_ptr<FmaSelectUnit> selectToUnits();

    virtual bool isText();
    virtual bool isContainer();
    virtual bool isRow();
    virtual void paint( QPainter *painter);
    virtual void paintSelect(QPainter *painter);


    virtual bool isAtStart();
    virtual bool isAtEnd();

    virtual void moveToStart(){};
    virtual void moveToEnd(){};

    qreal fullWidth() const;
    qreal fullHeight() const;

    int scriptLevel() const;
    int getLevelOffset() const{
        return levelOffset;
    }

    QRectF rect=QRectF(0,0,0,0);


    virtual qreal upHeight();
    virtual qreal downHeight();
    qreal midX();
    qreal midY();

    IFmaContainer* parent = nullptr;
    int partSelectStartId = 0;
    int partSelectEndId = 0;

    enum FmaSelectState{NoSelect=0x00, PartialSelect=0x01};
    quint8 cursorState = NoSelect;

    QRectF bounds();

    virtual bool hasSelect();
    virtual bool isAllSelect();

    virtual void deSelect();
    virtual void selectAll(bool isLeftToRight = true);
    virtual QList<std::shared_ptr<FmaEditOp>> removeSelection();
    virtual bool moveCursor(QTextCursor::MoveOperation) { return false; }
    virtual bool mouseClick(const QPointF& p);

    static void fillBackground(QPainter *p, const QRectF &rect);

    qreal lineThickness(qreal factor = fmaCon::defaultThickFactor) const;

    void buildIndexes(QVector<int>& indexes);
    QVector<int> fullIndexes();
    static QString indexesString(const QVector<int>& indexes);

    virtual int maxIndex() { return 0;};
    virtual int minIndex() { return 0;};
    virtual int activePos() { return 0; };
    virtual void setActivePos(int /*pos*/) {};

    virtual qreal marginLeft() const { return 0;};
    virtual qreal marginTop() const { return 0;};
    virtual qreal marginRight() const { return 0;};
    virtual qreal marginBottom() const { return 0;};

    virtual bool isStretchX();
    virtual bool isStretchY();

    bool isDisplayStyle() const;
    int stylecSriptlevel() const;

    void setDisplayStyle(bool isDisplay);
    void setStyleScriptLevel(int level);

    void setStyleValue(FmaProp propId, QVariant var);
    QVariant styleValue(FmaProp propId) const;

    static bool isValid(IFmaBase* base);
    static bool isStart(IFmaBase* base);
    static QCborMap::const_iterator findCbor(const QCborMap& cborMap, int key);

    FmaMath* getMath();
protected:
    QMap<FmaProp, QVariant> mstyle;

    inline qreal baseFullWidth() const{
        return rect.width() + marginLeft() + marginRight();
    }
    inline qreal baseFullHeight() const{
        return rect.height() + marginTop() + marginBottom();
    }
    virtual void stretchSize();
    virtual void calculateSize();
    virtual void markSelectStart();
    enum FmaSelectEndType{SelectAsValue, SelectToLeft, SelectToRight};
    virtual void selectFrom(const QVector<int>& indexes, int level, quint8 type=SelectAsValue);
    virtual void selectTo(const QVector<int>& indexes, int level, quint8 type=SelectAsValue);
    virtual void selectRange(const QVector<int>& start,
                     const QVector<int>& end,
                     int level,
                     quint8 type=SelectAsValue);
    virtual void clear();

    int levelOffset = 0;
    inline static void setLevelOffset(IFmaBase* t, int _levelOffset){
        t->levelOffset = _levelOffset;
    }

    static IFmaBase startBase;


    friend class IFmaContainer;
    friend class FmaMath;
    friend class FmaOpsCommand;
};


#endif // IFMABASE_H
