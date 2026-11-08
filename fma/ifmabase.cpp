#include "ifmabase.h"
#include "ifmacontainer.h"
#include "fmatextformats.h"
#include <QPainter>

QString FmaNodeType::toString(quint16 type)
{
    QMetaEnum metaEnm = QMetaEnum::fromType<FmaType>();
    return  metaEnm.valueToKey(type);
}

FmaNodeType::Type FmaNodeType::fromString(const QString &key)
{
    bool isOk = false;
    QMetaEnum metaEnm = QMetaEnum::fromType<FmaNodeType::Type>();
    int type = metaEnm.keyToValue(key.toLower().toStdString().data(), &isOk);
    if(isOk){
        return FmaType(type);
    }
    return FmaType::munknown;
}
//////////////////
IFmaBase IFmaBase::startBase;

int IFmaBase::compareIndexes(const QVector<int> &i1, const QVector<int>& i2)
{
    int len = qMin(i1.size(), i2.size());
    for(int i=0; i<len; ++i){
        if(i1[i] != i2[i]){
            return i1[i] - i2[i];
        }
    }
    return -(i1.size() - i2.size());
}

IFmaBase::IFmaBase()
{

}

IFmaBase::IFmaBase(const QCborMap &cborMap)
{
    parseBase(cborMap);
}

IFmaBase::~IFmaBase()
{
}

FmaType IFmaBase::type()
{
    return FmaType::munknown;
}

QCborMap IFmaBase::toCbor(bool)
{
    QCborMap cborMap;
    cborMap.insert(int(FmaCborId::Type), int(type()));
    if(!mstyle.isEmpty()){
        QCborMap cborStyle;
        QMap<FmaProp, QVariant>::iterator iter =  mstyle.begin();
        for(; iter != mstyle.end(); ++iter){
            FmaProp prop = iter.key();
            switch(prop){
            case FmaProp::Scriptlevel:{
                cborStyle.insert(int(iter.key()), iter.value().toInt());
                break;
            }
            case FmaProp::Displaystyle:{
                cborStyle.insert(int(iter.key()), iter.value().toBool());
                break;
            }
            //Todo no use yet
            case FmaProp::Scriptsizemultiplier:{
                cborStyle.insert(int(iter.key()), iter.value().toFloat());
                break;
            }
            case FmaProp::Scriptminsize:{
                cborStyle.insert(int(iter.key()), iter.value().toString());
                break;
            }
            case FmaProp::Infixlinebreakstyle:{
                cborStyle.insert(int(iter.key()), iter.value().toInt());
                break;
            }
            case FmaProp::Decimalpoint:{
                cborStyle.insert(int(iter.key()), iter.value().toString());
                break;
            }
            default:{
                cborStyle.insert(int(iter.key()), iter.value().toString());
                break;
            }
            }
        }
        cborMap.insert(int(FmaCborId::Style), cborStyle);
    }

    return cborMap;
}

void IFmaBase::parseBase(const QCborMap &cborMap)
{
    QCborMap::const_iterator iter = findCbor(cborMap, int(FmaCborId::Style));
    if(iter != cborMap.end()){
        parseStyle(iter.value().toMap());
    }
}

void IFmaBase::parseStyle(const QCborMap &styleMap)
{
    QCborMap::const_iterator iter = styleMap.begin();
    for(; iter != styleMap.end(); ++iter){
        mstyle[FmaProp(iter.key().toInteger())] = iter.value().toVariant();
    }
}


bool IFmaBase::isText()
{
    return false;
}

bool IFmaBase::isContainer()
{
    return false;
}

bool IFmaBase::isRow()
{
    return false;
}


void IFmaBase::calculateSize()
{

}

void IFmaBase::markSelectStart()
{
    if((cursorState & PartialSelect) == NoSelect){
        cursorState = PartialSelect;
        partSelectStartId = activePos();
    }
}

void IFmaBase::selectFrom(const QVector<int> &indexes, int level, quint8 type)
{
    cursorState = PartialSelect;
    partSelectStartId = indexes[level];

    int min = minIndex();
    int max = maxIndex();
    switch (type) {
    case SelectToLeft:{
        partSelectEndId = min;
        break;
    }
    case SelectToRight:{
        partSelectEndId = max;
        break;
    }
    default:{
        partSelectEndId = partSelectStartId;
        break;
    }
    }
}

void IFmaBase::selectTo(const QVector<int> &indexes, int level, quint8 type)
{
    cursorState = PartialSelect;
    partSelectEndId = indexes[level];

    int min = minIndex();
    int max = maxIndex();
    switch (type) {
    case SelectToLeft:{
        partSelectStartId = max;
        break;
    }
    case SelectToRight:{
        partSelectStartId = min;
        break;
    }
    default:{
        partSelectStartId = partSelectEndId;
        break;
    }
    }
}

void IFmaBase::selectRange(const QVector<int> &start, const QVector<int> &end, int level, quint8 type)
{
    cursorState = PartialSelect;
    int min = minIndex();
    int max = maxIndex();

    switch (type) {
    case SelectToLeft:{
        if(level >=0){
            if(level<start.size()){
                partSelectStartId = start[level];
            }else{
                partSelectStartId = max;
            }

            if(level<end.size()){
                partSelectEndId = end[level];
            }else{
                partSelectEndId = min;
            }
        }else{
            partSelectStartId = max;
            partSelectEndId = min;
        }
        break;
    }
    default:{
        if(level >=0){
            if(level<start.size()){
                partSelectStartId = start[level];
            }else{
                partSelectStartId = min;
            }

            if(level<end.size()){
                partSelectEndId = end[level];
            }else{
                partSelectEndId = max;
            }
        }else{
            partSelectStartId = min;
            partSelectEndId = max;
        }
        break;
    }
    }
}

bool IFmaBase::isStretchX()
{
    return false;
}

bool IFmaBase::isStretchY()
{
    return false;
}

bool IFmaBase::isDisplayStyle() const
{
    if(mstyle.isEmpty()){
        return false;
    }
    return mstyle[FmaProp::Displaystyle].toBool();
}

int IFmaBase::stylecSriptlevel() const
{
    if(mstyle.isEmpty()){
        return 0;
    }
    return mstyle[FmaProp::Scriptlevel].toInt();
}

void IFmaBase::setDisplayStyle(bool isDisplay)
{
    mstyle[FmaProp::Displaystyle] = isDisplay;
}

void IFmaBase::setStyleScriptLevel(int level)
{
    mstyle[FmaProp::Scriptlevel] = level;
}

void IFmaBase::setStyleValue(FmaProp propId, QVariant var)
{
    mstyle[propId] = var;
}

QVariant IFmaBase::styleValue(FmaProp propId) const
{
    if(mstyle.isEmpty()){
        return QVariant();
    }
    return mstyle[propId];
}

bool IFmaBase::isValid(IFmaBase *base)
{
    return base != nullptr && base != &startBase;
}

bool IFmaBase::isStart(IFmaBase *base)
{
    return base == &startBase;
}

void IFmaBase::paint(QPainter *)
{

}

void IFmaBase::paintSelect(QPainter *p)
{
    if(NoSelect){
        return;
    }

    QRectF f = bounds();
    p->save();
    p->setBrushOrigin(f.topLeft());
    p->fillRect(f, p->background());
    p->restore();
}

void IFmaBase::clear()
{

}

bool IFmaBase::isAtStart()
{
    return true;
}

bool IFmaBase::isAtEnd()
{
    return true;
}


qreal IFmaBase::fullWidth() const
{
    if(rect.width()){
        return baseFullWidth();
    }
    return 0;
}

qreal IFmaBase::fullHeight() const
{
    if(rect.height()){
        return baseFullHeight();
    }
    return 0;
}

int IFmaBase::scriptLevel() const
{
    if(parent){
        return ((IFmaBase*)parent)->scriptLevel() + levelOffset + stylecSriptlevel();
    }
    return levelOffset;
}

void IFmaBase::stretchSize()
{
    if(parent != nullptr){
        if(parent->canStrechChild(this)){
            bool stretchX = isStretchX();
            bool stretchY = isStretchY();
            if(stretchX || stretchY){
                QRectF t = rect;
                QRectF pRect = parent->rect;
                if(stretchY){
                    if(pRect.height() > baseFullHeight()){
                        t.setHeight(pRect.height() - marginTop() - marginBottom());
                    }
                }

                if(stretchX){
                    if(pRect.width() > baseFullWidth()){
                        t.setWidth(pRect.width() - marginLeft() - marginRight());
                    }
                }

                rect = t;
            }
        }
    }
}

qreal IFmaBase::upHeight()
{
    return fullHeight()*0.5;
}

qreal IFmaBase::downHeight()
{
    return fullHeight() - upHeight();
}

qreal IFmaBase::midX()
{
    return rect.x() + rect.width()/2;
}

qreal IFmaBase::midY()
{
    return rect.y() + rect.height()/2;
}

QRectF IFmaBase::bounds()
{
    QRectF t;
    t.setRect(rect.x() - marginLeft(), rect.y()-marginTop(), fullWidth(), fullHeight());
    return t;
}

bool IFmaBase::hasSelect()
{
    if(cursorState == NoSelect){
        return false;
    }
    return true;
}

bool IFmaBase::isAllSelect()
{
    if(cursorState == NoSelect){
        return false;
    }

    int min = minIndex();
    int max = maxIndex();

    if((partSelectStartId == min)
            && (partSelectEndId == max)){
        return true;
    }

    if((partSelectStartId == max)
                 && (partSelectEndId == min)){
        return true;
    }
    return false;
}


void IFmaBase::deSelect()
{
    cursorState = NoSelect;
}

void IFmaBase::selectAll(bool isLeftToRight)
{
    cursorState = PartialSelect;
    if(isLeftToRight){
        partSelectStartId = minIndex();
        partSelectEndId = maxIndex();
    }else{
        partSelectStartId = maxIndex();
        partSelectEndId = minIndex();
    }
}

QList<std::shared_ptr<FmaEditOp>> IFmaBase::removeSelection()
{
    QList<std::shared_ptr<FmaEditOp>> ret;
    if(cursorState == NoSelect){
        return ret;
    }

    partSelectEndId = 0;
    partSelectStartId = 0;
    cursorState = NoSelect;
    return ret;
}


bool IFmaBase::mouseClick(const QPointF &p)
{
    if(p.x() < midX()){
        moveToStart();
    }else{
        moveToEnd();
    }
    return true;
}

void IFmaBase::fillBackground(QPainter *p, const QRectF &rect)
{
    p->save();
    p->setBrushOrigin(rect.topLeft());
    p->fillRect(rect, p->background());
    p->restore();
}

qreal IFmaBase::lineThickness(qreal factor) const
{
    QFont font = FmaTextFormats::getScriptFont(FmaTextFormats::NormalMV, scriptLevel());
    qreal ret = factor *qMax(1.0, QFontMetricsF(font).lineWidth());
    if(ret >=fmaCon::maxThick){
        return fmaCon::maxThick;
    }
    return ret;
}

void IFmaBase::buildIndexes(QVector<int>& indexes)
{
    if(parent){
        parent->buildIndexes(indexes);
        indexes.append(parent->getChildren().indexOf(this));
        if(isContainer()){
            if(isAtStart()){
                indexes.append(-1);
            }
        }
    }
}

QVector<int> IFmaBase::fullIndexes()
{
    QVector<int> indexes;
    buildIndexes(indexes);
    return indexes;
}

QString IFmaBase::indexesString(const QVector<int> &indexes)
{
    QStringList tList;
    for(int index : indexes){
        tList.append(QString::number(index));
    }
    return tList.join(".");
}

FmaType IFmaBase::parseTextType(const QString& t, FmaOperDict::Form& form)
{
    if(t.isEmpty()){
        return FmaType::mtext;
    }else{
        bool isOk =false;
        t.toDouble(&isOk);
        if(isOk){
            return FmaType::mn;
        }

        if(FmaOperDict::findForm(t, form) != FmaOperDict::mmlNofix){
            return FmaType::mo;
        }

        bool isAllLetterOrNumber = true;
        QString::const_iterator iter = t.begin();
        for(; iter != t.end(); ++iter){
            if(iter->isSpace() || iter->isNonCharacter()){
                return FmaType::mtext;
            }

            if(!iter->isLetterOrNumber()){
                isAllLetterOrNumber = false;
                break;
            }
        }

        if(isAllLetterOrNumber){
            return FmaType::mi;
        }

        return FmaType::mtext;
    }
}


QCborMap::const_iterator IFmaBase::findCbor(const QCborMap &cborMap, int key)
{
    QCborMap::const_iterator iter = cborMap.find(key);
    if(iter != cborMap.end()){
        return iter;
    }

    iter = cborMap.find(QString::number(key));
    return iter;
}

FmaMath *IFmaBase::getMath()
{
    if(this->type() == FmaType::math){
        return (FmaMath*)this;
    }
    if(parent){
        return parent->getMath();
    }
    return nullptr;
}



