#ifndef IFMACONTAINER_H
#define IFMACONTAINER_H

#include "ifmabase.h"

class FmaMath;
class IFmaContainer : public IFmaBase
{
public:
    IFmaContainer();
    IFmaContainer(const QCborMap& cborMap, int initChildNum = 0);

    virtual ~IFmaContainer();
    virtual bool isContainer() final;
    virtual void paint( QPainter *painter) override;
    virtual void paintSelect(QPainter *painter) override;

    virtual void calculateRect();
    virtual void calculatePos();

    virtual FmaType type() override;
    virtual bool isDynChild();

    virtual QCborMap toCbor(bool filterSelect = false) override;
    QCborArray childrenCbor(int pos=0, int len=-1);

    const QList<IFmaBase*>& getChildren();
    IFmaBase* getActiveChild();
    void setActiveChild(IFmaBase* _activeChild);
    bool deleteChild(IFmaBase* child);
    bool take(IFmaBase* child);
    int indexOfChild(IFmaBase* child);
    virtual int del(int pos, int length=1);
    virtual bool insert(IFmaBase* child, int pos = 0);

    bool hasChilds();

    bool isAncestor(IFmaBase* t);
    bool isAcceptableChild(IFmaBase* t);

    IFmaBase* firstBlock();
    IFmaBase* nextBlock();
    IFmaBase* previousBlock();
    IFmaBase* lastBlock();

    int childCount();

    virtual bool hasSelect() override;
    virtual bool isAllSelect() override;

    virtual void deSelect() override;
    virtual void selectAll(bool isLeftToRight = true) override;

    virtual  QList<std::shared_ptr<FmaEditOp>> removeSelection() override;

    virtual bool moveCursor(QTextCursor::MoveOperation op) override;
    virtual bool mouseClick(const QPointF& p) override;

    virtual int maxIndex() override;
    virtual int minIndex() override;
    virtual int activePos() override;
    virtual void setActivePos(int pos) override;

    virtual bool setToRowChild(IFmaBase* t, int index);
    virtual bool canStrechChild(IFmaBase* ch);

    virtual bool isStretchX() override;
    virtual bool isStretchY() override;

    virtual bool isAtStart()override;
    virtual bool isAtEnd()override;

    virtual void moveToStart() override;
    virtual void moveToEnd() override;
protected:
    virtual void markSelectStart() override;
    virtual void calculateSize() override;
    virtual void stretchSize() override;
    virtual void selectFrom(const QVector<int>& indexes, int level, quint8 type=SelectAsValue) override;
    virtual void selectTo(const QVector<int>& indexes, int level, quint8 type=SelectAsValue) override;
    virtual void selectRange(const QVector<int>& start,
                     const QVector<int>& end,
                     int level,
                     quint8 type=SelectAsValue) override;
    virtual void clear() override;

    QList<IFmaBase*> children;
    IFmaBase* activeChild = nullptr;
    friend class FmaMath;

    void initFixNumChildren(const QCborMap& cborMap, int childLimit=1);
};

class FmaRow;
class FmaBuddyBase : public IFmaContainer
{
public:
    FmaBuddyBase(IFmaBase* buddy= nullptr);
    FmaBuddyBase(const QCborMap& cborMap, int childLimit=1);

    void setBuddy(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setBuddy(IFmaBase* buddy);
    virtual bool canStrechChild(IFmaBase* ch);
    FmaRow* getBuddy();
protected:
};

//////////////////////////

class FmaText;
class FmaFenced : public FmaBuddyBase
{
public:
    FmaFenced(const QString openStr="(", const QString closeStr=")", IFmaBase* child = nullptr);
    FmaFenced(const QCborMap& cborMap);

    virtual ~FmaFenced();
    virtual FmaType type() override;
    virtual void paint( QPainter *painter) override;
    virtual void calculatePos() override;

    virtual QCborMap toCbor(bool filterSelect = false) override;
protected:
    virtual void calculateSize() override;
    virtual void stretchSize() override;

    void setOpenClose(const QString& openStr="(", const QString& closeStr=")");

    FmaText* open = nullptr;
    FmaText* close = nullptr;
};



#endif // IFMACONTAINER_H
