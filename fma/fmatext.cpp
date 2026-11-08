#include "fmatext.h"
#include "fmatextformats.h"
#include "ifmacontainer.h"
#include "fmarow.h"
#include "fmaoperdict.h"
#include <QPainter>
#include <QDebug>
#include "fmammltable.h"
#include <QPolygonF>
#include <QPainterPath>

namespace {

struct ArrowStyle {
    int direction = 0; // -1 left, 1 right, 0 both
    bool doubleLine = false;
    bool headUp = true;
    bool headDown = true;
};

bool arrowStyle(const QString &text, ArrowStyle &style)
{
    if (text.size() != 1)
        return false;

    switch (text.at(0).unicode()) {
    case 0x2192: case 0x27F6: case 0x21A6: // → ⟶ ↦
        style.direction = 1;
        style.doubleLine = false;
        break;
    case 0x21C0: // ⇀ 单肩上
        style.direction = 1;
        style.doubleLine = false;
        style.headDown = false;
        break;
    case 0x21C1: // ⇁ 单肩下
        style.direction = 1;
        style.doubleLine = false;
        style.headUp = false;
        break;
    case 0x21D2: case 0x27F9: // ⇒ ⟹
        style.direction = 1;
        style.doubleLine = true;
        break;
    case 0x2190: case 0x27F5: case 0x21A4: // ← ⟵ ↤
        style.direction = -1;
        style.doubleLine = false;
        break;
    case 0x21BC: // ↼ 单肩上
        style.direction = -1;
        style.doubleLine = false;
        style.headDown = false;
        break;
    case 0x21BD: // ↽ 单肩下
        style.direction = -1;
        style.doubleLine = false;
        style.headUp = false;
        break;
    case 0x21D0: case 0x27F8: // ⇐ ⟸
        style.direction = -1;
        style.doubleLine = true;
        break;
    case 0x2194: case 0x27F7: // ↔ ⟷
        style.direction = 0;
        style.doubleLine = false;
        break;
    case 0x21D4: case 0x27FA: // ⇔ ⟺
        style.direction = 0;
        style.doubleLine = true;
        break;
    default:
        return false;
    }
    return true;
}

// 用线条绘制可拉伸的水平箭头，避免 QFont::setStretch 把箭头头部拉变形。
bool drawStretchedArrow(QPainter *painter, const QRectF &rect, const QString &text)
{
    ArrowStyle style;
    if (!arrowStyle(text, style))
        return false;
    if (rect.width() < 6.0 || rect.height() < 2.0)
        return false;

    const qreal y = rect.center().y();
    const qreal lineWidth = qMax(1.0, rect.height() * 0.06);
    const qreal headLength = qMin(qMax(6.0, rect.height() * 0.45), rect.width() * 0.45);
    const qreal headWidth = qMax(4.0, rect.height() * 0.5);
    const qreal gap = style.doubleLine ? qMax(2.0, lineWidth * 2.5) : 0.0;

    const qreal left = rect.left();
    const qreal right = rect.right();
    const qreal lineLeft = left;
    const qreal lineRight = right;

    QPen pen = painter->pen();
    pen.setWidthF(lineWidth);
    pen.setCapStyle(Qt::FlatCap);

    painter->save();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const auto drawLine = [&](qreal lineY) {
        if (lineRight > lineLeft)
            painter->drawLine(QPointF(lineLeft, lineY), QPointF(lineRight, lineY));
    };
    if (style.doubleLine) {
        drawLine(y - gap / 2);
        drawLine(y + gap / 2);
    } else {
        drawLine(y);
    }

    const auto drawHead = [&](qreal x, int dir) {
        if (style.headUp) {
            painter->drawLine(QPointF(x, y),
                              QPointF(x - dir * headLength, y - headWidth / 2));
        }
        if (style.headDown) {
            painter->drawLine(QPointF(x, y),
                              QPointF(x - dir * headLength, y + headWidth / 2));
        }
    };
    if (style.direction >= 0)
        drawHead(right, 1);
    if (style.direction <= 0)
        drawHead(left, -1);

    painter->restore();
    return true;
}

bool isStretchRule(const QString &text)
{
    if (text.size() != 1)
        return false;
    switch (text.at(0).unicode()) {
    case 0x003D: case 0x2261: case 0x2014: case 0x2015:
    case 0x22A2: case 0x22A3: case 0x22A4: case 0x22A5:
    case 0x27C2: case 0x22A8: case 0x2223:
        return true;
    default:
        return false;
    }
}

bool isDecoLine(const QString &text)
{
    if (text.size() != 1)
        return false;
    switch (text.at(0).unicode()) {
    case 0x007E: // ~
    case 0x02DC: // ˜
    case 0x0303: // combining tilde
    case 0x005E: // ^
    case 0x02C6: // ˆ
    case 0x0302: // combining hat
    case 0x005F: // _
    case 0x2017: // ‾
    case 0x203E: // ‾ overline
    case 0x00AF: // ¯ macron
    case 0x02C9: // ˉ modifier macron
    case 0x0305: // combining overline
    case 0x0332: // combining low line
    case 0x20D7: // combining vector arrow
    case 0x23DC: // ⏜
    case 0x23DD: // ⏝
    case 0x23DE: // ⏞
    case 0x23DF: // ⏟
    case 0x23CB: // ⏋
    case 0x23CC: // ⏌
    case 0xFE39: // 上括号
    case 0xFE3A: // 下括号
    case 0x2192: case 0x2190: case 0x21C0: case 0x21C1:
    case 0x21BC: case 0x21BD: case 0x21A4: case 0x21A6:
    case 0x2194:
        return true;
    default:
        return false;
    }
}

bool isFenceChar(const QString &text)
{
    if (text.size() != 1)
        return false;
    switch (text.at(0).unicode()) {
    case 0x0028: case 0x0029: // ( )
    case 0x005B: case 0x005D: // [ ]
    case 0x007B: case 0x007D: // { }
    case 0x007C: // |
    case 0x2016: // ‖
    case 0x2223: // ∣
    case 0x27E8: case 0x27E9: // ⟨ ⟩
    case 0x2308: case 0x2309: // ⌈ ⌉
    case 0x230A: case 0x230B: // ⌊ ⌋
    case 0x27E6: case 0x27E7: // ⟦ ⟧
    case 0x231C: case 0x231D: case 0x231E: case 0x231F: // ⌜ ⌝ ⌞ ⌟
    case 0x2983: case 0x2984: // ⦃ ⦄
    case 0x27EE: case 0x27EF: // ⟮ ⟯
    case 0x2772: case 0x2773: // ❲ ❳
        return true;
    default:
        return false;
    }
}

bool drawHorizontalRule(QPainter *painter, const QRectF &r, const QString &text)
{
    if (text.size() != 1)
        return false;

    // 波浪/括号/双线需要比字体墨迹更高的绘制范围，否则会被压扁。
    QRectF box = r;
    const qreal minHeight = qMax(12.0, box.height());
    if (box.height() < minHeight) {
        box.setHeight(minHeight); // 向下扩展，over 行由调用方额外向上扩展
    }

    const qreal lw = qMax(1.0, box.height() * 0.05);
    const qreal gap = qMax(3.0, box.height() * 0.22);
    const qreal left = box.left();
    const qreal right = box.right();
    const qreal centerX = box.center().x();
    const qreal centerY = box.center().y();

    QPen pen = painter->pen();
    pen.setWidthF(1.0);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setMiterLimit(10.0);

    painter->save();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const auto hline = [&](qreal y) {
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    };
    const auto vline = [&](qreal x, qreal y1, qreal y2) {
        painter->drawLine(QPointF(x, y1), QPointF(x, y2));
    };

    bool ok = true;
    switch (text.at(0).unicode()) {
    case 0x003D: // =
        hline(centerY - gap / 2);
        hline(centerY + gap / 2);
        break;
    case 0x2261: // ≡
        hline(centerY - gap);
        hline(centerY);
        hline(centerY + gap);
        break;
    case 0x2014: case 0x2015: // — ―
        hline(centerY);
        break;
    case 0x22A2: // ⊢
        hline(centerY);
        vline(left, box.top(), box.bottom());
        break;
    case 0x22A3: // ⊣
        hline(centerY);
        vline(right, box.top(), box.bottom());
        break;
    case 0x22A4: // ⊤
        hline(box.top() + lw / 2);
        vline(centerX, box.top() + lw, box.bottom());
        break;
    case 0x22A5: // ⊥
        hline(box.bottom() - lw / 2);
        vline(centerX, box.top(), box.bottom() - lw);
        break;
    case 0x27C2: // ⟂
        hline(centerY);
        vline(centerX, box.top(), box.bottom());
        break;
    case 0x22A8: // ⊨
        hline(centerY);
        vline(left, box.top(), box.bottom());
        vline(left + gap, box.top(), box.bottom());
        break;
    case 0x2223: // ∣
        vline(centerX, box.top(), box.bottom());
        break;
    case 0x005F: case 0x2017: case 0x203E:
    case 0x00AF: case 0x02C9: case 0x0305: case 0x0332: { // _ ‾ ¯
        hline(centerY);
        break;
    }
    case 0x007E: case 0x02DC: case 0x0303: { // ~ ˜ ̃
        QPainterPath path;
        const qreal amp = box.height() * 0.35;
        const int segments = qBound(1, int(box.width() / 20.0), 8);
        const qreal step = box.width() / segments;
        path.moveTo(left, centerY);
        for (int i = 0; i < segments; ++i) {
            const qreal x0 = left + i * step;
            path.cubicTo(x0 + step * 0.33, centerY - amp,
                         x0 + step * 0.66, centerY + amp,
                         x0 + step, centerY);
        }
        painter->drawPath(path);
        break;
    }
    case 0x005E: case 0x02C6: case 0x0302: { // ^ ˆ ̂
        const qreal topY = box.top() + box.height() * 0.22;
        QPainterPath path;
        path.moveTo(left, box.bottom());
        path.lineTo(centerX, topY);
        path.lineTo(right, box.bottom());
        painter->drawPath(path);
        break;
    }
    case 0x23DC: { // ⏜ 上圆弧括号
        const qreal bottomY = box.bottom() - box.height() * 0.15;
        const qreal topY = box.top() + box.height() * 0.22;
        QPainterPath path;
        path.moveTo(left, bottomY);
        path.cubicTo(left + box.width() * 0.25, topY,
                     right - box.width() * 0.25, topY,
                     right, bottomY);
        painter->drawPath(path);
        break;
    }
    case 0x23DD: { // ⏝ 下圆弧括号
        const qreal topY = box.top() + box.height() * 0.15;
        const qreal bottomY = box.bottom() - box.height() * 0.22;
        QPainterPath path;
        path.moveTo(left, topY);
        path.cubicTo(left + box.width() * 0.25, bottomY,
                     right - box.width() * 0.25, bottomY,
                     right, topY);
        painter->drawPath(path);
        break;
    }
    case 0x23DE: case 0xFE39: { // ⏞ 上平顶括号
        const qreal bottomY = box.top() + box.height() * 0.62;
        const qreal midY = box.top() + box.height() * 0.24;
        const qreal inset = box.width() * 0.12;
        painter->drawPolyline(QPolygonF()
                              << QPointF(left, bottomY)
                              << QPointF(left + inset, midY)
                              << QPointF(right - inset, midY)
                              << QPointF(right, bottomY));
        break;
    }
    case 0x23DF: case 0xFE3A: { // ⏟ 下平顶括号
        const qreal topY = box.bottom() - box.height() * 0.62;
        const qreal midY = box.bottom() - box.height() * 0.24;
        const qreal inset = box.width() * 0.12;
        painter->drawPolyline(QPolygonF()
                              << QPointF(left, topY)
                              << QPointF(left + inset, midY)
                              << QPointF(right - inset, midY)
                              << QPointF(right, topY));
        break;
    }
    case 0x23CB: // ⏋
        hline(box.top() + lw / 2);
        vline(left, box.top(), box.bottom());
        vline(right, box.top(), box.bottom());
        break;
    case 0x23CC: // ⏌
        hline(box.bottom() - lw / 2);
        vline(left, box.top(), box.bottom());
        vline(right, box.top(), box.bottom());
        break;
    default:
        ok = false;
        break;
    }

    painter->restore();
    return ok;
}

bool drawVerticalSymbol(QPainter *painter, const QRectF &r, const QString &text)
{
    if (text.size() != 1)
        return false;

    const qreal lw = qMax(1.0, qMin(1.2, r.height() * 0.02));
    const qreal gap = qMax(3.0, lw * 3.0);
    const qreal left = r.left();
    const qreal right = r.right();
    const qreal top = r.top();
    const qreal bottom = r.bottom();
    const qreal centerX = r.center().x();
    const qreal centerY = r.center().y();

    QPen pen = painter->pen();
    pen.setWidthF(1.0);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    painter->save();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const auto vline = [&](qreal x, qreal y1, qreal y2) {
        painter->drawLine(QPointF(x, y1), QPointF(x, y2));
    };
    const auto hline = [&](qreal y, qreal x1, qreal x2) {
        painter->drawLine(QPointF(x1, y), QPointF(x2, y));
    };

    bool ok = true;
    switch (text.at(0).unicode()) {
    case 0x007C: case 0x2223: // | ∣
        vline(centerX, top, bottom);
        break;
    case 0x2016: // ‖
        vline(centerX - gap / 2, top, bottom);
        vline(centerX + gap / 2, top, bottom);
        break;
    case 0x005B: // [
        vline(left, top, bottom);
        hline(top, left, right);
        hline(bottom, left, right);
        break;
    case 0x005D: // ]
        vline(right, top, bottom);
        hline(top, left, right);
        hline(bottom, left, right);
        break;
    case 0x2308: // ⌈
        vline(left, top, bottom);
        hline(top, left, right);
        break;
    case 0x2309: // ⌉
        vline(right, top, bottom);
        hline(top, left, right);
        break;
    case 0x230A: // ⌊
        vline(left, top, bottom);
        hline(bottom, left, right);
        break;
    case 0x230B: // ⌋
        vline(right, top, bottom);
        hline(bottom, left, right);
        break;
    case 0x0028: { // (
        QPainterPath path;
        path.moveTo(right, top);
        path.cubicTo(left, top + (bottom - top) * 0.25,
                     left, bottom - (bottom - top) * 0.25,
                     right, bottom);
        painter->drawPath(path);
        break;
    }
    case 0x0029: { // )
        QPainterPath path;
        path.moveTo(left, top);
        path.cubicTo(right, top + (bottom - top) * 0.25,
                     right, bottom - (bottom - top) * 0.25,
                     left, bottom);
        painter->drawPath(path);
        break;
    }
    case 0x27E8: // ⟨
        painter->drawPolyline(QPolygonF() << QPointF(right, top)
                                          << QPointF(left, centerY)
                                          << QPointF(right, bottom));
        break;
    case 0x27E9: // ⟩
        painter->drawPolyline(QPolygonF() << QPointF(left, top)
                                          << QPointF(right, centerY)
                                          << QPointF(left, bottom));
        break;
    case 0x007B: { // {
        QPainterPath path;
        path.moveTo(right, top);
        path.cubicTo(left, top, right, centerY, left, centerY);
        path.cubicTo(right, centerY, left, bottom, right, bottom);
        painter->drawPath(path);
        break;
    }
    case 0x007D: { // }
        QPainterPath path;
        path.moveTo(left, top);
        path.cubicTo(right, top, left, centerY, right, centerY);
        path.cubicTo(left, centerY, right, bottom, left, bottom);
        painter->drawPath(path);
        break;
    }
    case 0x27E6: case 0x27E7: { // ⟦ ⟧
        const bool isLeft = (text.at(0).unicode() == 0x27E6);
        const qreal x1 = isLeft ? left : right;
        const qreal x2 = isLeft ? left + gap : right - gap;
        vline(x1, top, bottom);
        vline(x2, top, bottom);
        hline(top, left, right);
        hline(bottom, left, right);
        break;
    }
    case 0x231C: // ⌜
        vline(left, top, bottom);
        hline(top, left, right);
        break;
    case 0x231D: // ⌝
        vline(right, top, bottom);
        hline(top, left, right);
        break;
    case 0x231E: // ⌞
        vline(left, top, bottom);
        hline(bottom, left, right);
        break;
    case 0x231F: // ⌟
        vline(right, top, bottom);
        hline(bottom, left, right);
        break;
    default:
        if(isFenceChar(text)){
            vline(centerX, top, bottom);
        }else{
            ok = false;
        }
        break;
    }

    painter->restore();
    return ok;
}

bool drawStretchedSymbol(QPainter *painter, const QRectF &rect, const QString &text,
                         bool stretchX, bool stretchY)
{
    if (text.size() != 1)
        return false;
    if (stretchX && drawStretchedArrow(painter, rect, text))
        return true;
    if (stretchX && drawHorizontalRule(painter, rect, text))
        return true;
    if (stretchY && drawVerticalSymbol(painter, rect, text))
        return true;
    return false;
}

} // namespace


FmaText::FmaText(FmaType type, QString str, FmaOperDict::FormFix formFix): text(str)
{
    reparse(type, formFix);
    m_activePos = text.size();
}

FmaText::FmaText(const QCborMap &cborMap):IFmaBase(cborMap)
{
    FmaType type = FmaType::munknown;
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::Type));
    if(iter != cborMap.end()){
        type = (FmaType)(iter.value().toInteger());
    }

    iter = findCbor(cborMap, int(FmaCborId::Text));
    if(iter != cborMap.end()){
        text = iter.value().toString();
        reparse(type, FmaOperDict::mmlNofix);
        m_activePos = text.size();
    }
}

bool FmaText::isText()
{
    return true;
}

FmaType FmaText::type()
{
    return m_type;
}

bool FmaText::isInvisibleOperator() const
{
    //特殊字符查看https://unicode-table.com/cn/blocks/
    //u200B零宽间隔.
    //u2061函数应用程序.
    //u2062不可见乘号.
    //u2063不可见分隔符.
    //u2064不可见正号.
    QString ttext = searchText();
    return    ttext == QString( QChar( 0x61, 0x20 ) )  // &ApplyFunction;
            || ttext == QString( QChar( 0x62, 0x20 ) )  // &InvisibleTimes;
            || ttext == QString( QChar( 0x63, 0x20 ) )  // &InvisibleComma;
            || ttext == QString( QChar( 0x64, 0x20 ) )  // Invisible addition
            || ttext == QString( QChar( 0x0B, 0x20 ) );
}

QRectF textSize(const QString& str, const QFontMetricsF& metrics){
    QRectF ret;
    if(str.isEmpty()){
        QRectF rectT = metrics.boundingRect('x');  //
        ret.setWidth(rectT.width());
        ret.setHeight(rectT.height());
    }else{
        QRectF rectT = metrics.boundingRect(str );
        //ret.setWidth(rectT.width() + metrics.leftBearing(str[0]) + metrics.rightBearing(str[str.length()-1]));
        ret.setWidth(rectT.width());
        ret.setHeight(rectT.height());
    }

    return ret;
}

void FmaText::calculateSize()
{
    QFontMetricsF metrics = QFontMetricsF( font() );
    QString ttext = searchText();
    rect = textSize(ttext, metrics);
}

void FmaText::stretchSize()
{
    if(!parent || !parent->canStrechChild(this)){
        return;
    }

    const bool stretchX = isStretchX();
    const bool stretchY = isStretchY();
    if(!stretchX && !stretchY){
        return;
    }

    // mover/munder/munderover 的上/下元素按 base 的尺寸拉伸，
    // 而不是按包含长上/下标的整行宽度拉伸。
    IFmaBase *target = parent;
    if(parent->parent && parent->parent->isContainer()){
        const FmaType pType = parent->parent->type();
        if(pType == FmaType::mover || pType == FmaType::munder || pType == FmaType::munderover){
            FmaBuddyBase *buddyBase = dynamic_cast<FmaBuddyBase*>(parent->parent);
            if(buddyBase && buddyBase->getBuddy() && buddyBase->getBuddy() != parent){
                target = buddyBase->getBuddy();
            }
        }
    }

    const QRectF pRect = target->rect;
    if(stretchX && pRect.width() > baseFullWidth()){
        rect.setWidth(pRect.width() - marginLeft() - marginRight());
    }
    if(stretchY && pRect.height() > baseFullHeight()){
        rect.setHeight(pRect.height() - marginTop() - marginBottom());
    }
}

void FmaText::paint(QPainter *painter)
{
    QString ttext = searchText();
    if(!ttext.isEmpty()){
        painter->save();
        QFont tFont = font();

        QFontMetricsF metrics = QFontMetricsF( tFont);
        if(m_type == FmaType::mo || isStretchX() || isStretchY()){
            if(opForm.type != FmaOperDict::mmlNofix || isStretchX() || isStretchY()){
                if((opForm.prop & FmaOperDict::mmlStretchHorizontal)
                        || (opForm.prop & FmaOperDict::mmlStretchVertical)
                        || isStretchX()
                        || isStretchY()){
                    // 可拉伸符号手绘，避免字体拉伸的空白边缘导致起点/长度异常。
                    const bool stretchX = isStretchX()
                            && rect.width() > metrics.boundingRect(ttext).width() * 1.5;
                    const bool stretchY = isStretchY()
                            && rect.height() > metrics.height() * 1.2;

                    // 上下装饰线直接以 base 内容的左右边界为准，避免居中后起点偏移。
                    QRectF drawRect = rect;
                    bool isOverRow = false;
                    if(stretchX && parent && parent->parent){
                        const FmaType pType = parent->parent->type();
                        if(pType == FmaType::mover || pType == FmaType::munder
                                || pType == FmaType::munderover){
                            IFmaContainer *container = dynamic_cast<IFmaContainer*>(parent->parent);
                            FmaBuddyBase *buddy = dynamic_cast<FmaBuddyBase*>(container);
                            if(buddy && buddy->getBuddy()){
                                drawRect.setLeft(buddy->getBuddy()->rect.left());
                                drawRect.setWidth(buddy->getBuddy()->rect.width());
                            }
                            if(container){
                                const int index = container->getChildren().indexOf(parent);
                                isOverRow = (pType == FmaType::mover && index == 1)
                                        || (pType == FmaType::munderover && index == 2);
                            }
                        }
                    }
                    if(stretchX && isOverRow && drawRect.height() < 12.0){
                        const qreal extra = 12.0 - drawRect.height();
                        drawRect.setTop(drawRect.top() - extra);
                        drawRect.setHeight(12.0);
                    }

                    if((stretchX || stretchY)
                            && drawStretchedSymbol(painter, drawRect, ttext, stretchX, stretchY)){
                        painter->restore();
                        return;
                    }

                    QRectF symbolRect = metrics.tightBoundingRect( ttext );
                    if(symbolRect.width() >0 && symbolRect.height() >0){
                        qreal wRatio = 1;
                        qreal hRatio = 1;

                        if(opForm.prop & FmaOperDict::mmlStretchHorizontal){
                            wRatio = (rect.width())/symbolRect.width();

                        }

                        if(opForm.prop & FmaOperDict::mmlStretchVertical){
                            hRatio = rect.height()/symbolRect.height();
                        }

                        // 限制字体伸缩比例，避免极端比例把字形拉坏。
                        wRatio = qBound(0.5, wRatio, 3.0);
                        hRatio = qBound(0.5, hRatio, 4.0);

                        tFont.setPixelSize(qMax(1, int(tFont.pixelSize()*hRatio)));
                        int t = qBound(50, int(wRatio*100.0/hRatio), 300);
                        tFont.setStretch(t);
                        painter->setFont( tFont );
                        QFontMetricsF metrics2 = QFontMetricsF( tFont);
                        qreal d = -(metrics2.ascent()/2-metrics2.descent()-metrics2.lineWidth()/3);

                        painter->drawText(rect.x(), rect.y()+metrics2.ascent()+d, ttext);
                        painter->restore();

                        return;
                    }
                }
            }
        }

        painter->setFont( tFont );
        painter->drawText(rect.x(), rect.y()+metrics.ascent(), ttext);
        painter->restore();
    }
}

void FmaText::paintSelect(QPainter *p)
{
    if(cursorState == NoSelect){
        return;
    }

    if(isAllSelect()){
        QRectF f = bounds();
        p->save();
        p->setBrushOrigin(f.topLeft());
        p->fillRect(f, p->background());
        p->restore();
    }else{
        if(partSelectStartId == partSelectEndId){
            return;
        }
        QFontMetricsF metrics = QFontMetricsF( font() );
        QRectF rectStart, rectEx;
        if(partSelectStartId < partSelectEndId){
            rectStart = metrics.boundingRect(text.mid(0, partSelectStartId));
            rectEx = textSize(text.mid(partSelectStartId, partSelectEndId-partSelectStartId), metrics);
        }else{
            rectStart = metrics.boundingRect(text.mid(0, partSelectEndId));
            rectEx = textSize(text.mid(partSelectEndId, partSelectStartId-partSelectEndId), metrics);
        }

        QRectF selectRect;

        if(partSelectStartId == 0 || partSelectEndId == 0){
            selectRect = QRectF(rect.x()-marginLeft(),
                              rect.y() - marginTop(),
                              rectEx.width() + marginLeft(),
                              fullHeight());
        }else if(partSelectStartId == text.length() || partSelectEndId == text.length()){
            selectRect = QRectF(rect.x() + rectStart.width(),
                              rect.y() - marginTop(),
                              rectEx.width() + marginRight(),
                              fullHeight());
        }else{
            selectRect = QRectF(rect.x() + rectStart.width(),
                              rect.y() - marginTop(),
                              rectEx.width(),
                              fullHeight());
        }


        p->save();
        p->setBrushOrigin(selectRect.topLeft());
        p->fillRect(selectRect, p->background());
        p->restore();
    }
}

void FmaText::clear()
{
    text.clear();
    m_activePos = 0;
}

QCborMap FmaText::toCbor(bool filterSelect)
{
    QCborMap ret = IFmaBase::toCbor(filterSelect);
    if(filterSelect){
        if(isAllSelect()){
            ret.insert(int(FmaCborId::Text),text);
        }else{
            QString t;
            if(partSelectStartId < partSelectEndId){
                t = text.mid(partSelectStartId, partSelectEndId-partSelectStartId);
            }else{
                t = text.mid(partSelectEndId, partSelectStartId-partSelectEndId);
            }
            ret.insert(int(FmaCborId::Text),t);
        }
    }else{
        ret.insert(int(FmaCborId::Text),text);
    }
    return ret;
}

void FmaText::reparse(FmaType type, FmaOperDict::FormFix formFix)
{
    opForm.type = formFix;
    QString ttext = searchText();
    if(type == FmaType::munknown){
        m_type = parseTextType(ttext, opForm);
    }else if(ttext.isEmpty()){
        setType(type);
    }else{
        m_type = parseTextType(ttext, opForm);
    }
}



void FmaText::input(const QString &v)
{
    text = text.insert(m_activePos, v);
    m_activePos += v.size();
    QString t = FmaMmlTable::searchByText(text);
    if(!t.isEmpty()){
        m_type = parseTextType(t, opForm);
    }
}

QRectF FmaText::textRect(int i)
{
    QString t = FmaMmlTable::searchByText(text);
    if(!t.isEmpty()){
        if(i==0 ){
            return  QRectF(rect.x(), rect.y(), 0, rect.height());
        }else{
            QFontMetricsF metrics = QFontMetricsF( font() );
            QRectF rectT = metrics.boundingRect(t);
            return  QRectF(rect.x(), rect.y(), rectT.width(), rect.height());
        }
    }

    QFontMetricsF metrics = QFontMetricsF( font() );
    QRectF rectT = metrics.boundingRect(text.left(i) );
    return  QRectF(rect.x(), rect.y(), rectT.width(), rect.height());
}

QFont FmaText::font() const
{
    QFont font = FmaTextFormats::getScriptFont(fontMv(), fontLevel());
    if( m_type == FmaType::mi ){
        if(!text.startsWith("&")){
            font.setItalic( true );
        }
    }else if ( m_type == FmaType::mo )
    {
        font.setItalic( false );
        font.setBold( false );
    }
    font.setKerning(false);
    font.setLetterSpacing(QFont::AbsoluteSpacing,1);
    return font;
}

void FmaText::asText()
{
    setType(FmaType::mtext);
}

void FmaText::asMn()
{
    setType(FmaType::mn);
}

void FmaText::asMi()
{
    setType(FmaType::mi);
}

void FmaText::asMo(FmaOperDict::FormFix formFix)
{
    setType(FmaType::mo, formFix);
}

void FmaText::setType(FmaType type, FmaOperDict::FormFix formFix)
{
    switch (type) {
    case FmaType::mtext:
    case FmaType::mi:
    case FmaType::mn:{
        m_type = type;
        break;
    }
    case FmaType::mo:{
        m_type = type;
        opForm.type = formFix;
        FmaOperDict::findForm(text, opForm);
        break;
    }
    default:{
        m_type = FmaType::mtext;
        break;
    }
    }
}

bool FmaText::isStretchX()
{
    if(!parent || !parent->parent){
        return false;
    }

    const FmaType pType = parent->parent->type();

    // menclose 等结构保持原有横向拉伸。
    if(pType == FmaType::menclose){
        FmaBuddyBase* buddyBase = (FmaBuddyBase*)(parent->parent);
        if((IFmaContainer*)(buddyBase->getBuddy()) != parent){
            if(m_type == FmaType::mo && opForm.type != FmaOperDict::mmlNofix){
                return opForm.prop & FmaOperDict::mmlStretchHorizontal;
            }
        }
        return false;
    }

    if(pType != FmaType::mover && pType != FmaType::munder && pType != FmaType::munderover){
        return false;
    }

    IFmaContainer *container = dynamic_cast<IFmaContainer*>(parent->parent);
    if(!container){
        return false;
    }
    const int index = container->getChildren().indexOf(parent);
    const bool isOver = (pType == FmaType::mover && index == 1)
            || (pType == FmaType::munderover && index == 2);
    const bool isUnder = (pType == FmaType::munder && index == 1)
            || (pType == FmaType::munderover && index == 1);
    if(!isOver && !isUnder){
        return false;
    }

    // 普通上下标内容（如 1→2221）保持字体；单文本的装饰标线才拉伸。
    FmaRow *row = dynamic_cast<FmaRow*>(parent);
    const bool single = row && row->isSingleText();

    // 装饰标线/关系符不依赖字体的 stretchy 标记，直接拉伸。
    if(isDecoLine(text) || isStretchRule(text)){
        return isOver || single;
    }

    if(m_type == FmaType::mo && opForm.type != FmaOperDict::mmlNofix
            && (opForm.prop & FmaOperDict::mmlStretchHorizontal)){
        return isOver || single;
    }

    return false;
}

bool FmaText::isStretchY()
{
    // 括号字符总是按内容高度拉伸，并走手绘细线。
    if(isFenceChar(text)){
        return true;
    }

    // FmaFenced 的左右括号总是按内容高度拉伸，并走手绘细线。
    if(parent && parent->type() == FmaType::mfenced){
        return true;
    }

    if(m_type != FmaType::mo){
        return false;
    }

    if(opForm.type != FmaOperDict::mmlNofix){
        return opForm.prop & FmaOperDict::mmlStretchVertical;
    }
    return false;
}

qreal FmaText::marginLeft() const
{
    if(m_type == FmaType::mo){
        if(opForm.type != FmaOperDict::mmlNofix){
            int level = fontLevel();
            return FmaTextFormats::spacing(opForm.lspace, level)*0.5;
        }
    }
    return 0;
}

qreal FmaText::marginRight() const
{
    if(m_type == FmaType::mo){
        if(opForm.type != FmaOperDict::mmlNofix){
            int level = fontLevel();
            return FmaTextFormats::spacing(opForm.rspace, level)*0.5;
        }
    }
    return 0;
}

void FmaText::buildTextIndexes(QVector<int> &indexes)
{
    if(parent){
        parent->buildIndexes(indexes);
        indexes.append(parent->getChildren().indexOf(this));
    }
    indexes.append(m_activePos);
}

QVector<int> FmaText::textIndexes()
{
    QVector<int> indexes;
    buildTextIndexes(indexes);
    return indexes;
}

QString FmaText::searchText() const
{
    QString t = FmaMmlTable::searchByText(text);
    if(t.isEmpty()){
        return text;
    }

    return t;
}


quint8 FmaText::fontMv() const
{
    if(m_fontMv == FmaTextFormats::InvalidMV){
        return FmaTextFormats::NormalMV;
    }else{
        return m_fontMv;
    }
}

int FmaText::activePos()
{
    return m_activePos;
}

void FmaText::setActivePos(int pos)
{
    m_activePos = pos;
}

bool FmaText::isAtStart()
{
    return m_activePos == 0;
}

bool FmaText::isAtEnd()
{
    return m_activePos == text.length();
}

void FmaText::moveToStart()
{
    m_activePos = 0;
}

void FmaText::moveToEnd()
{
    m_activePos = text.length();
}

QList<std::shared_ptr<FmaEditOp>> FmaText::removeSelection()
{
    QList<std::shared_ptr<FmaEditOp>> ret;
    if(cursorState == NoSelect){
        return ret;
    }

    int removeStart = partSelectStartId;
    int removeEnd = partSelectEndId;
    if(removeStart > removeEnd){
        removeStart = partSelectEndId;
        removeEnd = partSelectStartId;
    }
    if(removeStart != removeEnd){
        std::shared_ptr<FmaTextInputDelete> op
                = std::make_shared<FmaTextInputDelete>(false);
        buildIndexes(op->editItemIndex);
        op->editItemIndex.append(removeStart);
        op->text = text.mid(removeStart, removeEnd - removeStart);
        ret.append(op);

        text.remove(removeStart, removeEnd - removeStart);
        if(m_activePos > removeEnd){
            m_activePos = removeStart + m_activePos - removeEnd;
        }else{
            if(m_activePos > removeStart){
                m_activePos = removeStart;
            }
        }
    }
    partSelectEndId = 0;
    partSelectStartId = 0;
    cursorState = NoSelect;
    return ret;
}

bool FmaText::deleteActive(bool isForward)
{
    if(isForward){
        if(m_activePos>=0 && m_activePos<text.length()){
            text = text.remove(m_activePos, 1);
            if(m_activePos >=text.length()){
                m_activePos = text.length();
            }
            return true;
        }
    }else{
        int delPos = m_activePos-1;
        if(delPos>=0 && delPos<text.length()){
            text = text.remove(delPos, 1);
            m_activePos = delPos;
            return true;
        }
    }
    return false;
}

bool FmaText::deletePos(int pos, int length)
{
    if(pos>=0 && pos<text.length()){
        m_activePos = pos;
        text = text.remove(pos, length);
        return true;
    }
    return false;
}


bool FmaText::moveCursor(QTextCursor::MoveOperation op)
{
    switch(op){
    case QTextCursor::NoMove:{
        return true;
    }
    case QTextCursor::Start:
    case QTextCursor::StartOfLine:
    case QTextCursor::StartOfBlock:
    case QTextCursor::StartOfWord:{
        m_activePos = 0;
        return true;
    }
    case QTextCursor::PreviousBlock:
    case QTextCursor::PreviousCharacter:
    case QTextCursor::PreviousWord:
    case QTextCursor::Left:
    case QTextCursor::WordLeft:
    case QTextCursor::PreviousCell:{
        QString t = FmaMmlTable::searchByText(text);
        if(!t.isEmpty()){
            m_activePos = 0;
            return true;
        }

        if(m_activePos >0){
            m_activePos--;
            return true;
        }
        break;
    }
    case QTextCursor::End:
    case QTextCursor::EndOfLine:
    case QTextCursor::EndOfWord:
    case QTextCursor::EndOfBlock:{
        m_activePos = text.length();
        return true;
    }
    case QTextCursor::NextBlock:
    case QTextCursor::NextCharacter:
    case QTextCursor::NextWord:
    case QTextCursor::Right:
    case QTextCursor::WordRight:
    case QTextCursor::NextCell:{
        QString t = FmaMmlTable::searchByText(text);
        if(!t.isEmpty()){
            m_activePos = text.length();
            return true;
        }

        if(m_activePos <text.length()){
            m_activePos++;
            return true;
        }
        break;
    }
    default:{
        break;
    }
    }

    return false;
}

bool FmaText::mouseClick(const QPointF &p)
{
    m_activePos = 0;

    if(text.startsWith("&") && text.endsWith(";")){
        QString ttext = text.mid(1, text.size()-2);
        ttext = FmaMmlTable::searchValueByName(ttext);
        if(!ttext.isEmpty()){
            m_activePos = text.size();
            return true;
        }
    }

    QFontMetricsF metrics = QFontMetricsF( font() );
    QRectF t;

    qreal startOffset = metrics.averageCharWidth()*0.5;
    for(int i=1; i<=text.size(); ++i){
        t = metrics.boundingRect(text.left(i));
        t.moveLeft(rect.x() + startOffset);
        t.moveTop(rect.y());

        if(t.contains(p)){
            m_activePos = i;
            return true;
        }
    }

    if(p.x() < midX()){
        moveToStart();
    }else{
        moveToEnd();
    }
    return true;
}

int FmaText::maxIndex()
{
    return text.length();
}

