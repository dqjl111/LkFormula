#ifndef FmaBuddyBase_H
#define FmaBuddyBase_H

#include "fmarow.h"

class FmaSubSuperBase: public FmaBuddyBase
{
public:
    FmaSubSuperBase(IFmaBase* buddy= nullptr);
    Qt::LayoutDirection dir = Qt::LeftToRight;

    virtual QCborMap toCbor(bool filterSelect = false) override;
protected:
    FmaSubSuperBase(const QCborMap& cborMap, int limit);
};

class FmaSub : public FmaSubSuperBase
{
public:
    FmaSub(IFmaBase* _buddy= nullptr, IFmaBase* sub = nullptr);
    FmaSub(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setSub(const QString& text,
                FmaType type = FmaType::munknown,
                FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setSub(IFmaBase* sub);
    FmaRow* getSub();
};


class FmaSuper : public FmaSubSuperBase
{
public:
    FmaSuper(IFmaBase* buddy= nullptr, IFmaBase* super = nullptr);
    FmaSuper(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setSuper(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setSuper(IFmaBase* super);
    FmaRow* getSuper();
};

class FmaSubSuper : public FmaSubSuperBase
{
public:
    FmaSubSuper(IFmaBase* buddy= nullptr, IFmaBase* sub = nullptr, IFmaBase* super = nullptr);
    FmaSubSuper(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setSub(const QString& text,
                FmaType type = FmaType::munknown,
                FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setSub(IFmaBase* sub);
    FmaRow* getSub();

    void setSuper(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setSuper(IFmaBase* super);
    FmaRow* getSuper();
};

#endif // FmaBuddyBase_H
