#ifndef FMAFRAC_H
#define FMAFRAC_H

#include "ifmacontainer.h"

class FmaFrac : public IFmaContainer
{
public:
    FmaFrac();
    FmaFrac(const QCborMap& cborMap);

    virtual FmaType type() override;

    virtual void paint( QPainter *painter) override;
    virtual qreal upHeight() override;

    void setUp(const QString& text,
               FmaType type = FmaType::munknown,
               FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setUp(IFmaBase* buddy);
    FmaRow* getUp();

    void setDown(const QString& text,
                 FmaType type = FmaType::munknown,
                 FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    void setDown(IFmaBase* buddy);
    FmaRow* getDown();
    virtual bool moveCursor(QTextCursor::MoveOperation op) override;
    virtual QCborMap toCbor(bool filterSelect = false) override;

    bool bevelled = false;
protected:
    virtual void calculateSize() override;
    virtual void calculatePos() override;
};

#endif // FMAFRAC_H
