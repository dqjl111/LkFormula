#ifndef FMAUNDEROVER_H
#define FMAUNDEROVER_H

#include "ifmacontainer.h"

class FmaUnder : public FmaBuddyBase
{
public:
    FmaUnder(IFmaBase* buddy = nullptr);
    FmaUnder(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setUnder(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setUnder(IFmaBase* under);
    FmaRow* getUnder();

    virtual bool canStrechChild(IFmaBase* ch) override;

    qreal spacing = 0;
};

class FmaOver : public FmaBuddyBase
{
public:
    FmaOver(IFmaBase* buddy = nullptr);
    FmaOver(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setOver(const QString& text,
                 FmaType type = FmaType::munknown,
                 FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setOver(IFmaBase* over);
    FmaRow* getOver();

    virtual bool canStrechChild(IFmaBase* ch) override;

    qreal spacing = 0;
};


class FmaUnderOver : public FmaBuddyBase
{
public:
    FmaUnderOver(IFmaBase* buddy = nullptr);
    FmaUnderOver(const QCborMap& cborMap);

    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void calculatePos() override;
    virtual qreal upHeight() override;

    void setUnder(const QString& text,
                  FmaType type = FmaType::munknown,
                  FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setUnder(IFmaBase* under);
    FmaRow* getUnder();

    void setOver(const QString& text,
                 FmaType type = FmaType::munknown,
                 FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setOver(IFmaBase* over);
    FmaRow* getOver();

    virtual bool canStrechChild(IFmaBase* ch) override;

    qreal overSpacing = 0;
    qreal underSpacing = 0;
};

#endif // FMAUNDEROVER_H
