#include "fmatext.h"
#include "fmatextformats.h"
#include "ifmacontainer.h"
#include "fmaoperdict.h"
#include <QPainter>
#include <QDebug>
#include "fmammltable.h"


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

void FmaText::paint(QPainter *painter)
{
    QString ttext = searchText();
    if(!ttext.isEmpty()){
        painter->save();
        QFont tFont = font();

        QFontMetricsF metrics = QFontMetricsF( tFont);
        if(m_type == FmaType::mo){
            if(opForm.type != FmaOperDict::mmlNofix){
                if((opForm.prop & FmaOperDict::mmlStretchHorizontal)
                        || (opForm.prop & FmaOperDict::mmlStretchVertical)){
    //                if(ttext == QChar(0x2017)){
    //                    ttext = "=";
    //                }
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

                        tFont.setPixelSize(QVariant(tFont.pixelSize()*hRatio).toInt());
                        int t = QVariant(wRatio*100.0/hRatio).toInt();
                        if(t>4000){
                            t =4000;
                        }
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
    if(parent){
        if(parent->parent){
            FmaType pType = parent->parent->type();
            if(pType == FmaType::mover
               ||pType == FmaType::munder
               ||pType == FmaType::munderover
               ||pType == FmaType::menclose){
                FmaBuddyBase* buddyBase = (FmaBuddyBase*)(parent->parent);
                if((IFmaContainer*)(buddyBase->getBuddy()) != parent){
                    if(m_type == FmaType::mo){
                        if(opForm.type != FmaOperDict::mmlNofix){
                            return opForm.prop & FmaOperDict::mmlStretchHorizontal;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool FmaText::isStretchY()
{
    if(m_type == FmaType::mo){
        if(opForm.type != FmaOperDict::mmlNofix){
            return opForm.prop & FmaOperDict::mmlStretchVertical;
        }
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

