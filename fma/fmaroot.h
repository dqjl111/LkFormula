#ifndef FMAROOT_H
#define FMAROOT_H
#include "ifmacontainer.h"

class FmaSquareRoot : public FmaBuddyBase
{
public:
    FmaSquareRoot(IFmaBase* buddy = nullptr);
    FmaSquareRoot(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;

    virtual void paint( QPainter *painter) override;
    virtual qreal upHeight() override;
    virtual qreal marginTop() const override;

protected:
    FmaSquareRoot(const QCborMap& cborMap, int limit);
};

class FmaRoot : public FmaSquareRoot
{
public:
    FmaRoot(IFmaBase* buddy = nullptr);
    FmaRoot(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;

    void setPower(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setPower(IFmaBase* power);
    FmaRow* getPower();
    virtual qreal marginTop() const override;
};


#endif // FMAROOT_H
