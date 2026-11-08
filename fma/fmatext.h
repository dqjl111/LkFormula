#ifndef FMATEXT_H
#define FMATEXT_H

#include "ifmabase.h"
#include "fmatextformats.h"

class FmaMath;
class FmaText : public IFmaBase
{
public:
    FmaText(FmaType type = FmaType::munknown,
            QString str = "",
            FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    FmaText(const QCborMap& cborMap);

    virtual bool isText() override;
    virtual FmaType type() override;
    virtual void calculateSize() override;
    virtual void paint( QPainter *painter) override;
    virtual void paintSelect(QPainter *painter) override;

    virtual QCborMap toCbor(bool filterSelect = false) override;

    void reparse(FmaType type, FmaOperDict::FormFix formFix);

    void input(const QString& v);


    QRectF textRect(int i=0);
    QFont font() const;

    QString text;
    void asText();
    void asMn();  //数字文字
    void asMi();  //用于指定变量名称，函数或常量等标识符
    void asMo(FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);  //+，- 等操作符

    void setFontMv(quint8 mv);
    quint8 fontMv() const;

    virtual int activePos() override;
    virtual void setActivePos(int pos) override;
    virtual bool isAtStart() override;
    virtual bool isAtEnd() override;
    virtual void moveToStart() override;
    virtual void moveToEnd() override;

    virtual QList<std::shared_ptr<FmaEditOp>> removeSelection() override;
    bool deleteActive(bool isForward);
    bool deletePos(int pos, int length = 1);

    virtual bool moveCursor(QTextCursor::MoveOperation op) override;
    virtual bool mouseClick(const QPointF& p) override;

    virtual int maxIndex() override;

    bool isInvisibleOperator() const;
    virtual bool isStretchX() override;
    virtual bool isStretchY() override;
    int fontLevel() const {
        int level = scriptLevel();
        if(opForm.prop & FmaOperDict::mmlLargeOp){
            level --;
        }
        return level;
    }

    virtual qreal marginLeft() const override;
    virtual qreal marginRight() const override;

    void buildTextIndexes(QVector<int>& indexes);
    QVector<int> textIndexes();

    QString searchText() const;
protected:
    void setType(FmaType type, FmaOperDict::FormFix formFix = FmaOperDict::mmlNofix);
    virtual void clear() override;

    FmaType m_type=FmaType::munknown;
    quint8 m_fontMv =FmaTextFormats::InvalidMV;
    int m_activePos = 0;

    FmaOperDict::Form opForm;

    friend class FmaMath;
};



#endif // FMATEXT_H
