#ifndef FMAENCLOSE_H
#define FMAENCLOSE_H

#include "ifmacontainer.h"

class FmaEnclose: public FmaBuddyBase
{
public:
    enum Notation{
        LongDiv = 0x00,
        Top = 0x01,
        Bottom=0x02,
        Left=0x04,
        Right=0x08,

        Updiagonalstrike=0x10,
        Downdiagonalstrike=0x20,
        Horizontalstrike=0x40,
        Verticalstrike=0x80,

        TopLeft=Top|Left,
        Actuarial=Top|Right,
        TopRight=Actuarial,
        BottomLeft=Bottom|Left,
        BottomRight=Bottom|Right,
        Box=Top|Bottom|Left|Right,
    };


    FmaEnclose(int _notation=LongDiv, IFmaBase* child = nullptr);
    FmaEnclose(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void paint( QPainter *painter) override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    virtual QCborMap toCbor(bool filterSelect = false) override;
    int notation = LongDiv;

    static QString notationString(int notation);
    static int stringToNotation(const QString& str);
protected:
    virtual void calculateSize() override;

};



#endif // FMAENCLOSE_H
