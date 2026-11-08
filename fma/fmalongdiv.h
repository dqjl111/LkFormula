#ifndef FMALONGDIV_H
#define FMALONGDIV_H

#include "ifmacontainer.h"
//Todo 长除竖式
class FmaLongdiv: public FmaBuddyBase
{
public:
    FmaLongdiv(IFmaBase* child = nullptr);
    FmaLongdiv(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void paint( QPainter *painter) override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

protected:
    virtual void calculateSize() override;
};

#endif // FMALONGDIV_H
