#include "fmadoc.h"
#include "cmd/fmaopscommand.h"
#include "fmammltable.h"
#include "wmf/fmawmf.h"
#include "wmf/fmawmfgenerator.h"
#include <QJsonDocument>
#include <QFile>
#include <QJsonArray>
#include <QDebug>
#include <QDomDocument>
#include <QSvgGenerator>
#include <QPainter>
#include <QTextStream>
#include <QStringConverter>
#include "util/nodeleter.h"

bool FmaDoc::hasInit = false;

FmaDoc::FmaDoc()
{
    if(!hasInit){
        FmaOperDict::init();
        FmaTextFormats::init();
        FmaOpsCommand::init();
        hasInit = true;
    }
    undoStack.setUndoLimit(400);
}

bool FmaDoc::saveCbor(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly)){
        return file.write(math.toCbor().toCborValue().toCbor()) >0;
    }
    return false;
}

bool FmaDoc::loadCbor(const QString &filePath)
{

    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly)){
        const QByteArray data = file.readAll();
        return loadCbor(data);
    }
    return false;
}

bool FmaDoc::loadCbor(const QByteArray &data, bool canClip)
{
    math.clear();
    QCborMap cbor = QCborValue::fromCbor(data).toMap();

    IFmaBase *b = fromCbor(cbor, &math);
    if(b == &math){
        math.calculateRect();
    }else{
        if(b != nullptr){
            if(canClip){
                math.doInput(b);
                math.calculateRect();
            }else{
                qWarning() << "Fma load cbor failed!";
                delete b;
            }
        }
    }
    return true;
}

bool FmaDoc::saveJson(const QString &filePath)
{
    QJsonDocument doc;
    doc.setObject(math.toCbor().toJsonObject());
    QFile f(filePath);
    if(f.open(QFile::WriteOnly)){
        return f.write(doc.toJson()) >0;
    }
    return false;
}

bool FmaDoc::loadJson(const QString &filePath)
{
    QFile f(filePath);
    if(f.open(QFile::ReadOnly)){
        QJsonParseError err;
        QByteArray d = f.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(d, &err);
        if(err.error != QJsonParseError::NoError){
            qWarning() << err.errorString();
            return false;
        }

        QJsonObject jObj = doc.object();
        QCborMap cbor = QCborMap::fromJsonObject(jObj);

        math.clear();
        IFmaBase *b = fromCbor(cbor, &math);
        if(b == &math){
            math.calculateRect();
            return true;
        }else{
            qWarning() << "Fma load json failed!";
            delete b;
        }
    }
    return false;
}

bool FmaDoc::saveMml(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly)){
        const QCborMap cborMap = math.toCbor();
        QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
        if(iter == cborMap.end()){
            return false;
        }

        QDomDocument doc;
        QDomElement root = doc.createElement(FmaNodeType::toString(FmaType::math));

        root.setAttribute("xmlns", "https://www.w3.org/1998/Math/MathML");
        if(cborToMml(cborMap, &root, &doc)){
            doc.appendChild(root);
        }
        QTextStream os(&file);
        os.setEncoding(QStringConverter::Utf8);
        os << doc.toString();

        return true;
    }
    return false;
}

QString FmaDoc::toMml(const QCborMap &cborMap) const
{
    QDomDocument doc;
    QDomElement root = doc.createElement(FmaNodeType::toString(FmaType::math));
    root.setAttribute("xmlns", "http://www.w3.org/1998/Math/MathML");
    if(cborToMml(cborMap, &root, &doc)){
        doc.appendChild(root);
    }
    return doc.toString();
}

QString FmaDoc::toTex(const QCborMap &cborMap) const
{
    return cborToTex(cborMap);
}

bool FmaDoc::loadMml(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly)){

        QDomDocument doc;
        doc.setContent(&file);
        file.close();

        QDomElement docElem = doc.documentElement();
        if(!docElem.isNull()) {
            QCborMap cbor = mmlToCbor(&docElem);

            math.clear();
            IFmaBase *b = fromCbor(cbor, &math);
            if(b == &math){
                math.calculateRect();
                return true;
            }else{
                qWarning() << "Fma load mml failed!";
                delete b;
            }
        }
    }
    return false;
}

bool FmaDoc::saveTex(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly)){
        const QCborMap cborMap = math.toCbor();
        QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
        if(iter == cborMap.end()){
            return false;
        }

        QTextStream os(&file);
        os.setEncoding(QStringConverter::Utf8);
        os << cborToTex(cborMap);

        return true;
    }
    return false;
}


bool FmaDoc::saveSvg(const QString &filePath)
{
    if (filePath.isEmpty()){
        return false;
    }

    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(QSize(math.fullWidth(), math.fullHeight()));

    QPainter painter;
    painter.begin(&generator);
    math.paintChilds(&painter);
    painter.end();

    return true;
}

const QPixmap FmaDoc::grab()
{
    QPixmap image(math.fullWidth(), math.fullHeight());
    image.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&image);
    math.paintChilds(&painter);
    painter.end();
    return image;
}



bool FmaDoc::savePng(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly)){

        QPixmap image(math.fullWidth(), math.fullHeight());
        image.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&image);
        math.paintChilds(&painter);
        painter.end();

        image.save(filePath);
        return true;
    }
    return false;
}

bool FmaDoc::saveWmf(const QString &filePath)
{
    QFile file(filePath);
    std::shared_ptr<QIODevice> dev = noDeletePtr(file);
    return saveWmf(dev, true, true, false);
}

bool FmaDoc::loadWmf(const QString &filePath)
{
    QFile file(filePath);
    std::shared_ptr<QIODevice> dev = noDeletePtr(file);
    return loadWmf(dev);
}

bool FmaDoc::saveWmf(std::shared_ptr<QIODevice> dev,
                     bool outWmfFile,
                     bool withMml,
                     bool mapToHimetrics)
{
    if(dev == nullptr){
        return false;
    }

    if(!dev->isOpen()){
        dev->open(QIODevice::WriteOnly);
    }

    if(dev->isOpen()){
        FmaWmfGenerator generator;
        generator.setIODevice(dev);
        generator.setSize(QSize(math.fullWidth(), math.fullHeight()));
        if(outWmfFile){
            generator.setPaintType(FmaWmfPaintEngine::PaintWMF);
        }else{
            generator.setPaintType(FmaWmfPaintEngine::PaintOLEPres);
        }

        generator.setMapToHimetrics(mapToHimetrics);

        QPainter painter;
        painter.begin(&generator);
        math.paintChilds(&painter);

        if(withMml){
            const QCborMap cborMap = math.toCbor();
            QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
            if(iter == cborMap.end()){
                return false;
            }

            QDomDocument doc;
            QDomElement root = doc.createElement(FmaNodeType::toString(FmaType::math));

            root.setAttribute("display", "block");
            root.setAttribute("xmlns", "http://www.w3.org/1998/Math/MathML");

            if(cborToMml(cborMap, &root, &doc)){
                doc.appendChild(root);
            }

            generator.setMml(doc.toString(-1));
        }
        painter.end();

        return true;
    }
    return false;
}

bool FmaDoc::loadWmf(std::shared_ptr<QIODevice> dev)
{
    if(dev == nullptr){
        return false;
    }

    if(!dev->isOpen()){
        dev->open(QIODevice::ReadOnly);
    }
    if(dev->isOpen()){
        QDataStream is(dev.get());

        FmaWmfHelper helper;
        if(helper.readWmf(is)){
            helper.printRecords();

            QByteArray mmlData = helper.fetchMmlData();
            if(mmlData.isEmpty()){
                qWarning() << "Fma no mml in wmf!";
            }else{
                QDomDocument doc;
                doc.setContent(mmlData);

                QDomElement docElem = doc.documentElement();
                if(!docElem.isNull()) {
                    QCborMap cbor = mmlToCbor(&docElem);

                    math.clear();
                    IFmaBase *b = fromCbor(cbor, &math);
                    if(b == &math){
                        math.calculateRect();
                        return true;
                    }else{
                        qWarning() << "Fma load mml from wmf failed!";
                        delete b;
                    }
                }
            }
        }else{
            qWarning() << "Ole read wmf failed!";
        }
    }
    return false;
}


IFmaBase *FmaDoc::fromCbor(const QCborMap &cborMap, FmaMath *mathPtr)
{
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
    if(iter == cborMap.end()){
        return nullptr;
    }
    FmaType type = (FmaType)(iter.value().toInteger());
    switch(type){
    case FmaType::math:{
        if(mathPtr){
            mathPtr->fromCborMap(cborMap);
        }
        return mathPtr;
    }
    case FmaType::mi:
    case FmaType::mn:
    case FmaType::mo:
    case FmaType::mtext:{
        FmaText* t = new FmaText(cborMap);
        return t;
    }
    case FmaType::mfrac:{
        FmaFrac* t = new FmaFrac(cborMap);
        return t;
    }
    case FmaType::mrow:{
        FmaRow* t = new FmaRow(cborMap);
        return t;
    }
    case FmaType::msqrt:{
        FmaSquareRoot* t = new FmaSquareRoot(cborMap);
        return t;
    }
    case FmaType::mroot:{
        FmaRoot* t = new FmaRoot(cborMap);
        return t;
    }
    case FmaType::msup:{
        FmaSuper* t = new FmaSuper(cborMap);
        return t;
    }
    case FmaType::msub:{
        FmaSub* t = new FmaSub(cborMap);
        return t;
    }
    case FmaType::msubsup:{
        FmaSubSuper* t = new FmaSubSuper(cborMap);
        return t;
    }
    case FmaType::mtable:{
        FmaTable* t = new FmaTable(cborMap);
        return t;
    }
    case FmaType::mfenced:{
        FmaFenced* t = new FmaFenced(cborMap);
        return t;
    }
    case FmaType::menclose:{
        FmaEnclose* t = new FmaEnclose(cborMap);
        return t;
    }
    case FmaType::mlongdiv:{
        FmaLongdiv* t = new FmaLongdiv(cborMap);
        return t;
    }
    case FmaType::mover:{
        FmaOver* t = new FmaOver(cborMap);
        return t;
    }
    case FmaType::munder:{
        FmaUnder* t = new FmaUnder(cborMap);
        return t;
    }
    case FmaType::munderover:{
        FmaUnderOver* t = new FmaUnderOver(cborMap);
        return t;
    }
    default:{
        return nullptr;
    }
    }
}

QCborMap FmaDoc::mmlToCbor(QDomElement *ele)
{
    QCborMap ret;

    FmaType type = FmaNodeType::fromString(ele->tagName());
    if(type == FmaType::mstyle){
        QDomNode tChild = ele->firstChild();
        while(!tChild.isNull()){
            QDomElement dEle = tChild.toElement();
            type = FmaNodeType::fromString(dEle.tagName());
            if(type == FmaType::mstyle){
                qWarning() << "Unexpect style!";
            }else{
                ret.insert(int(FmaCborId::Type), int(type));
                if(type == FmaType::mtable){
                    mmlToCborTable(&dEle, ret);
                }else{
                    mmlToCborType(&dEle, ret);
                }
                break;
            }
            tChild = tChild.nextSibling();
        }

        ret.insert(int(FmaCborId::Style), mmlToCborStyle(ele));
    }else if(type == FmaType::mtable){
        ret.insert(int(FmaCborId::Type), int(type));
        mmlToCborTable(ele, ret);
    }
    else{
        ret.insert(int(FmaCborId::Type), int(type));
        mmlToCborType(ele, ret);
    }
    return ret;
}

QCborMap FmaDoc::mmlToCborStyle(QDomElement *ele)
{
    QCborMap styleCbor;
    if(ele->hasAttribute("scriptlevel")){
        QString attr = ele->attribute("scriptlevel");
        if(attr.startsWith("+") || attr.startsWith("-")){
            styleCbor.insert(int(FmaProp::Scriptlevel), attr.toInt());
        }
    }

    if(ele->hasAttribute("displaystyle")){
        QString attr = ele->attribute("displaystyle");
        if(attr == "true"){
            styleCbor.insert(int(FmaProp::Displaystyle), true);
        }else{
            styleCbor.insert(int(FmaProp::Displaystyle), false);
        }
    }

    if(ele->hasAttribute("scriptsizemultiplier")){
        QString attr = ele->attribute("scriptsizemultiplier");
        styleCbor.insert(int(FmaProp::Scriptsizemultiplier), attr.toFloat());
    }

    if(ele->hasAttribute("scriptminsize")){
        QString attr = ele->attribute("scriptminsize");
        styleCbor.insert(int(FmaProp::Scriptminsize), attr);
    }

    if(ele->hasAttribute("infixlinebreakstyle")){
        QString attr = ele->attribute("infixlinebreakstyle");
        if(attr == "after"){
            styleCbor.insert(int(FmaProp::Infixlinebreakstyle), int(FmaLineBreakStyle::After));
        }else if(attr == "duplicate"){
            styleCbor.insert(int(FmaProp::Infixlinebreakstyle), int(FmaLineBreakStyle::Duplicate));
        }else{
            styleCbor.insert(int(FmaProp::Infixlinebreakstyle), int(FmaLineBreakStyle::Before));
        }
    }

    if(ele->hasAttribute("scriptminsize")){
        QString attr = ele->attribute("scriptminsize");
        styleCbor.insert(int(FmaProp::Scriptminsize), attr);
    }

    return styleCbor;
}

void FmaDoc::mmlToCborType(QDomElement *ele, QCborMap& cbor)
{
    if(ele->hasChildNodes()){
        QCborArray arr;
        QDomNode tChild = ele->firstChild();
        while(!tChild.isNull()){
            if(tChild.isText()){
                cbor.insert(int(FmaCborId::Text),tChild.toText().data());
            }else if(tChild.isElement()){
                QDomElement tEle = tChild.toElement();
                arr.append(mmlToCbor(&tEle));
            }
            tChild = tChild.nextSibling();
        }

        if(!arr.isEmpty()){
            cbor.insert(int(FmaCborId::Children), arr);
        }
    }

    if(ele->hasAttribute("notation")){
        cbor.insert(int(FmaCborId::Notation), FmaEnclose::stringToNotation(ele->attribute("notation")));
    }

    if(ele->hasAttribute("bevelled")){
        if(ele->attribute("bevelled").trimmed().toLower() == "true"){
            cbor.insert(int(FmaCborId::Bevelled), true);
        }else{
            cbor.insert(int(FmaCborId::Bevelled), false);
        }
    }

    if(ele->hasAttribute("dir")){
        if("rtl" == ele->attribute("dir").trimmed().toLower()){
            cbor.insert(int(FmaCborId::Dir), int(Qt::RightToLeft));
        }
    }

    if(ele->hasAttribute("open")){
        cbor.insert(int(FmaCborId::Open), ele->attribute("open"));
    }

    if(ele->hasAttribute("close")){
        cbor.insert(int(FmaCborId::Close), ele->attribute("close"));
    }
}

void FmaDoc::mmlToCborTable(QDomElement *ele, QCborMap &cbor)
{
    QVector<QVector<QCborMap> > items;
    if(ele->hasChildNodes()){
        QCborArray arr;
        QDomNode rChild = ele->firstChild();
        while(!rChild.isNull()){
            if(rChild.isElement()){
                QDomElement rEle = rChild.toElement();
                if(rEle.tagName() == FmaNodeType::toString(FmaType::mtr)){
                    QVector<QCborMap> rowItems;
                    QDomNode cChild = rEle.firstChild();
                    while(!cChild.isNull()){
                        if(cChild.isElement()){
                            QDomElement cEle = cChild.toElement();
                            if(cEle.tagName() == FmaNodeType::toString(FmaType::mtd)){
                                QCborMap tdCbor;

                                QCborArray arr;
                                QDomNode tChild = cEle.firstChild();
                                while(!tChild.isNull()){
                                    if(tChild.isElement()){
                                        QDomElement tEle = tChild.toElement();
                                        arr.append(mmlToCbor(&tEle));
                                    }
                                    tChild = tChild.nextSibling();
                                }

                                if(arr.size() ==1){
                                    tdCbor = arr.at(0).toMap();
                                }else{
                                    tdCbor.insert(int(FmaCborId::Type), int(FmaType::mrow));
                                    if(!arr.empty()){
                                        tdCbor.insert(int(FmaCborId::Children), arr);
                                    }
                                }

                                rowItems.append(tdCbor);
                            }
                        }
                        cChild = cChild.nextSibling();
                    }
                    items.append(rowItems);
                }
            }
            rChild = rChild.nextSibling();
        }
    }

    int rowNum = items.size();
    int colNum = 0;
    for(const QVector<QCborMap>& row : items){
        colNum = qMax(row.size(), colNum);
    }

    cbor.insert(int(FmaCborId::RowNum), rowNum);
    cbor.insert(int(FmaCborId::ColumnNum), colNum);

    QCborArray arr;
    for(int r =0; r< rowNum; ++r){
        const QVector<QCborMap>& row = items[r];
        for(int c=0; c<colNum; ++c){
            if(c>= row.size()){
                QCborMap tdCbor;
                tdCbor.insert(int(FmaCborId::Type), int(FmaType::mrow));
                arr.append(tdCbor);
            }else{
                arr.append(row.at(c));
            }
        }
    }
    cbor.insert(int(FmaCborId::Children), arr);

    if(ele->hasAttribute("rowspacing")){
        cbor.insert(int(FmaCborId::RowSpacing),
                    FmaTextFormats::spacingType(ele->attribute("rowspacing").trimmed().toLower()));
    }

    if(ele->hasAttribute("columnspacing")){
        cbor.insert(int(FmaCborId::ColumnSpacing),
                    FmaTextFormats::spacingType(ele->attribute("columnspacing").trimmed().toLower()));
    }
}

bool FmaDoc::cborToMml(const QCborMap &cborMap, QDomElement* ele, QDomDocument* doc)
{
    if(styleToMml(cborMap, ele)){
        QDomElement tEle = doc->createElement(FmaNodeType::toString(FmaType::munknown));
        if(typeToMml(cborMap, &tEle, doc)){
            ele->appendChild(tEle);
        }
        return true;
    }else{
        return typeToMml(cborMap, ele, doc);
    }
}

void FmaDoc::mathToMml(const QCborMap &cborMap, QDomElement* ele, QDomDocument* doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() == 1){
            QDomElement row = doc->createElement(FmaNodeType::toString(FmaType::munknown));
            QCborMap childCbor = cborArr[0].toMap();
            if(cborToMml(childCbor, &row, doc)){
                ele->appendChild(row);
            }
        }else if(cborArr.size() >1){
            QDomElement table = doc->createElement(FmaNodeType::toString(FmaType::mtable));

            for(int i=0; i<cborArr.size(); ++i){
                QDomElement tr = doc->createElement(FmaNodeType::toString(FmaType::mtr));
                QDomElement td = doc->createElement(FmaNodeType::toString(FmaType::mtd));

                QCborMap childCbor = cborArr[i].toMap();
                QDomElement row = doc->createElement(FmaNodeType::toString(FmaType::munknown));
                if(cborToMml(childCbor, &row, doc)){
                    td.appendChild(row);
                }
                tr.appendChild(td);
                table.appendChild(tr);
            }
            table.setAttribute("columnalign", "left");
            ele->appendChild(table);
        }
    }
}

void FmaDoc::rowToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() == 1){
            QCborMap childCbor = cborArr[0].toMap();
            cborToMml(childCbor, ele, doc);
            if(ele->tagName() == FmaNodeType::toString(FmaType::mno)){
                ele->setTagName(FmaNodeType::toString(FmaType::mrow));
            }
        }else{
            for(int i=0; i<cborArr.size(); ++i){
                QCborMap childCbor = cborArr[i].toMap();
                QDomElement eleChild = doc->createElement(FmaNodeType::toString(FmaType::munknown));
                if(cborToMml(childCbor, &eleChild, doc)){
                    if(eleChild.tagName() == FmaNodeType::toString(FmaType::mno)){

                        QDomNode tChild = eleChild.firstChild();
                        while(!tChild.isNull()){
                            ele->appendChild(tChild.cloneNode());
                            tChild = tChild.nextSibling();
                        }
                    }else{
                        ele->appendChild(eleChild);
                    }
                }
            }
        }
    }
}

void FmaDoc::moToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QDomText domText = doc->createTextNode(mapIter->toString());
        ele->appendChild(domText);
    }
}

void FmaDoc::miToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QString miText = mapIter->toString();
        QString t = FmaMmlTable::searchByText(miText);
        if(t.isEmpty()){
            if(miText.size() >1){
                ele->setTagName(FmaNodeType::toString(FmaType::mno));
                for(int i=0;i<miText.size(); ++i){
                    QDomElement mi = doc->createElement(FmaNodeType::toString(FmaType::mi));
                    QDomText domText = doc->createTextNode(QString(miText[i]));
                    mi.appendChild(domText);
                    ele->appendChild(mi);
                }
            }else{
                QDomText domText = doc->createTextNode(miText);
                ele->appendChild(domText);
            }
        }else{
             miText = t;
             QDomText domText = doc->createTextNode(miText);
             ele->appendChild(domText);
        }
    }
}

void FmaDoc::mnToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QDomText domText = doc->createTextNode(mapIter->toString());
        ele->appendChild(domText);
    }
}

void FmaDoc::textToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QDomText domText = doc->createTextNode(mapIter->toString());
        ele->appendChild(domText);
    }
}

void FmaDoc::generalToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc, int childNum)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= childNum){
            for(int i=0; i<childNum; ++i){
                QCborMap childCbor = cborArr[i].toMap();
                QDomElement eleChild = doc->createElement(FmaNodeType::toString(FmaType::munknown));
                if(cborToMml(childCbor, &eleChild, doc)){
                    ele->appendChild(eleChild);
                }
            }
        }
    }
}

void FmaDoc::fracToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    generalToMml(cborMap, ele, doc, 2);
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Bevelled));
    if(mapIter != cborMap.end()){
        if(mapIter.value().toBool()){
            ele->setAttribute("bevelled", "true");
        }
    }
}

void FmaDoc::tableToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();

        int rowNum =0, colNum = 0;

        mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::RowNum));
        if(mapIter != cborMap.end()){
            rowNum = mapIter.value().toInteger();
        }

        mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::ColumnNum));
        if(mapIter != cborMap.end()){
            colNum = mapIter.value().toInteger();
        }


        for(int r=0; r<rowNum; ++r){
            QDomElement tr = doc->createElement(FmaNodeType::toString(FmaType::mtr));

            for(int c=0; c<colNum; ++c){
                QDomElement td = doc->createElement(FmaNodeType::toString(FmaType::mtd));

                int id = c+r*colNum;
                if(id < cborArr.size()){
                    QCborMap childCbor = cborArr[id].toMap();
                    QDomElement row = doc->createElement(FmaNodeType::toString(FmaType::mrow));
                    if(cborToMml(childCbor, &row, doc)){
                        td.appendChild(row);
                    }
                }
                tr.appendChild(td);
            }
            ele->appendChild(tr);
        }
    }

    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::RowSpacing));
    if(mapIter != cborMap.end()){
        ele->setAttribute("rowspacing", FmaTextFormats::spacingString(mapIter->toInteger()));
    }

    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::ColumnSpacing));
    if(mapIter != cborMap.end()){
        ele->setAttribute("columnspacing", FmaTextFormats::spacingString(mapIter->toInteger()));
    }
}

void FmaDoc::fencedToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    generalToMml(cborMap, ele, doc, 1);
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Open));
    if(mapIter != cborMap.end()){
        ele->setAttribute("open", mapIter.value().toString());
    }

    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Close));
    if(mapIter != cborMap.end()){
        ele->setAttribute("close", mapIter.value().toString());
    }
}

void FmaDoc::encloseToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    generalToMml(cborMap, ele, doc, 1);

    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Open));
    if(mapIter != cborMap.end()){
        ele->setAttribute("notation", FmaEnclose::notationString(mapIter.value().toInteger()));
    }
}

bool FmaDoc::styleToMml(const QCborMap &cborMap, QDomElement *ele)
{
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Style));
    if(iter != cborMap.end()){
        ele->setTagName(FmaNodeType::toString(FmaType::mstyle));
        QCborMap cborStyle = iter.value().toMap();

        iter = cborStyle.constBegin();
        for(; iter != cborStyle.end(); ++iter){

            FmaProp prop = FmaProp(iter.key().toInteger());
            switch(prop){
            case FmaProp::Scriptlevel:{
                int scriptLevelOffset = iter.value().toInteger();
                if(scriptLevelOffset>=0){
                    ele->setAttribute("scriptlevel", "+" + QString::number(scriptLevelOffset));
                }else{
                    ele->setAttribute("scriptlevel", QString::number(scriptLevelOffset));
                }
                break;
            }
            case FmaProp::Displaystyle:{
                if(iter.value().toBool()){
                    ele->setAttribute("displaystyle", "true");
                }else{
                    ele->setAttribute("displaystyle", "false");
                }
                break;
            }
            //Todo no use yet
            case FmaProp::Scriptsizemultiplier:{
                ele->setAttribute("scriptsizemultiplier", iter.value().toString());
                break;
            }
            case FmaProp::Scriptminsize:{
                ele->setAttribute("scriptminsize", iter.value().toString());
                break;
            }
            case FmaProp::Infixlinebreakstyle:{
                switch(FmaLineBreakStyle(iter.value().toInteger())){
                case FmaLineBreakStyle::Before:{
                    ele->setAttribute("infixlinebreakstyle", "before");
                    break;
                }
                case FmaLineBreakStyle::After:{
                    ele->setAttribute("infixlinebreakstyle", "after");
                    break;
                }
                case FmaLineBreakStyle::Duplicate:{
                    ele->setAttribute("infixlinebreakstyle", "duplicate");
                    break;
                }
                default:{
                    ele->setAttribute("infixlinebreakstyle", "before");
                    break;
                }
                }
                break;
            }
            case FmaProp::Decimalpoint:{
                ele->setAttribute("decimalpoint", iter.value().toString());
                break;
            }
            default:{
                break;
            }
            }
        }
        return true;
    }
    return false;
}

bool FmaDoc::typeToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
    if(iter == cborMap.end()){
        return false;
    }

    FmaType type = (FmaType)(iter.value().toInteger());
    ele->setTagName(FmaNodeType::toString(type));

    switch(type){
    case FmaType::math:{
        mathToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mi:{
        miToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mn:{
        mnToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mo:{
        moToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mtext:{
        textToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mfrac:{
        fracToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mrow:{
        rowToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::msqrt:{
        generalToMml(cborMap, ele, doc, 1);
        return true;
    }
    case FmaType::mroot:{
        generalToMml(cborMap, ele, doc, 2);
        return true;
    }
    case FmaType::msup:{
        superToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::msub:{
        subToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::msubsup:{
        subSuperToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mtable:{
        tableToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mfenced:{
        fencedToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::menclose:{
        encloseToMml(cborMap, ele, doc);
        return true;
    }
    case FmaType::mlongdiv:{
        //Todo
        generalToMml(cborMap, ele, doc, 1);
        return true;
    }
    case FmaType::mover:{
        generalToMml(cborMap, ele, doc, 2);
        return true;
    }
    case FmaType::munder:{
        generalToMml(cborMap, ele, doc, 2);
        return true;
    }
    case FmaType::munderover:{
        generalToMml(cborMap, ele, doc, 3);
        return true;
    }
    default:{
        return false;
    }
    }
}

void FmaDoc::subToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }

    generalToMml(cborMap, ele, doc, 2);
    if(dir == int(Qt::RightToLeft)){
        ele->setAttribute("dir", "rtl");
    }
}

void FmaDoc::superToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }

    generalToMml(cborMap, ele, doc, 2);
    if(dir == int(Qt::RightToLeft)){
        ele->setAttribute("dir", "rtl");
    }
}

void FmaDoc::subSuperToMml(const QCborMap &cborMap, QDomElement *ele, QDomDocument *doc)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }

    generalToMml(cborMap, ele, doc, 3);
    if(dir == int(Qt::RightToLeft)){
        ele->setAttribute("dir", "rtl");
    }
}

QString FmaDoc::cborToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Style));
    if(iter != cborMap.end()){
        iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
        const QCborArray cborArr = iter.value().toArray();
        if(cborArr.size() >= 1){
            QCborMap tCbor = cborArr.at(0).toMap();
            return cborToTex(tCbor);
        }
    }else{
        return typeToTex(cborMap);
    }
    return "";
}

QString FmaDoc::typeToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Type));
    if(iter == cborMap.end()){
        return "";
    }

    FmaType type = (FmaType)(iter.value().toInteger());
    switch(type){
    case FmaType::math:{
        return mathToTex(cborMap);
    }
    case FmaType::mi:{
        return miToTex(cborMap);
    }
    case FmaType::mn:{
        return mnToTex(cborMap);
    }
    case FmaType::mo:{
        return moToTex(cborMap);
    }
    case FmaType::mtext:{
        return textToTex(cborMap);
    }
    case FmaType::mfrac:{
        return fracToTex(cborMap);
    }
    case FmaType::mrow:{
        return rowToTex(cborMap);
    }
    case FmaType::msqrt:{
        return sqrtToTex(cborMap);
    }
    case FmaType::mroot:{
        return rootToTex(cborMap);
    }
    case FmaType::msup:{
        return superToTex(cborMap);
    }
    case FmaType::msub:{
        return subToTex(cborMap);
    }
    case FmaType::msubsup:{
        return subSuperToTex(cborMap);
    }
    case FmaType::mtable:{
        return tableToTex(cborMap);
    }
    case FmaType::mfenced:{
        return fencedToTex(cborMap);
    }
    case FmaType::menclose:{
        return encloseToTex(cborMap);
    }
    case FmaType::mover:{
        return overToTex(cborMap);
    }
    case FmaType::munder:{
        return underToTex(cborMap);
    }
    case FmaType::munderover:{
        return underOverToTex(cborMap);
    }
    default:{
        return "";
    }
    }
}

QString FmaDoc::mathToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() == 1){

            QCborMap childCbor = cborArr[0].toMap();
            return "$$" + cborToTex(childCbor) + "$$";
        }else if(cborArr.size() >1){

            QString ret;
            ret += "\\begin{array}\n";
            for(int i=0; i<cborArr.size(); ++i){
                if(i!=0){
                    ret += "\\\\\n";
                }
                QCborMap childCbor = cborArr[i].toMap();
                ret += cborToTex (childCbor);
            }
            ret += "\n\\end{array}";
            return ret;
        }
    }
    return "";
}

QString FmaDoc::moToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QString mo = mapIter->toString();
        QString t= FmaMmlTable::searchByText(mapIter->toString());
        if(!t.isEmpty()){
            mo = t;
        }

        QString ret;
        for(QChar c : mo){
            ret += FmaMmlTable::letterToTex(QString(c));
        }
        return ret;
    }
    return "";
}

QString FmaDoc::miToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        QString mi = mapIter->toString();
        QString t= FmaMmlTable::searchByText(mapIter->toString());
        if(!t.isEmpty()){
            mi = t;
        }

        QString ret;
        for(QChar c : mi){
            ret += FmaMmlTable::letterToTex(QString(c));
        }
        return ret;
    }
    return "";
}

QString FmaDoc::mnToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        return mapIter->toString();
    }
    return "";
}

QString FmaDoc::textToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Text));
    if(mapIter != cborMap.end()){
        return mapIter->toString();
    }
    return "";
}


QString FmaDoc::fracToTex(const QCborMap &cborMap)
{
    bool bevelled = false;
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Bevelled));
     if(mapIter != cborMap.end()){
         bevelled = mapIter.value().toBool();
     }


    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap upCbor = cborArr[0].toMap();
            QCborMap downCbor = cborArr[1].toMap();

            if(bevelled){
                braceTex(cborToTex(upCbor)) + "/" + braceTex(cborToTex(downCbor));
            }else{
                return "\\frac " + braceTex(cborToTex(upCbor)) + braceTex(cborToTex(downCbor));
            }
        }
    }
    return "";
}

QString FmaDoc::rowToTex(const QCborMap &cborMap)
{
    QString ret;
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        for(int i=0; i<cborArr.size(); ++i){
            QCborMap childCbor = cborArr[i].toMap();
            ret += cborToTex(childCbor);
        }
    }
    return ret;
}


QString FmaDoc::sqrtToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 1){
            return "\\sqrt " + braceTex(cborToTex(cborArr[0].toMap())) + "";
        }
    }
    return "";
}

QString FmaDoc::rootToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap powerCbor = cborArr[1].toMap();

            return "\\root " + braceTex(cborToTex(baseCbor))
                    + " \\of " + braceTex(cborToTex(powerCbor));
        }
    }
    return "";
}

QString FmaDoc::subToTex(const QCborMap &cborMap)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }


    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap subCbor = cborArr[1].toMap();
            if(dir == int(Qt::LeftToRight)){
                return braceTex(cborToTex(baseCbor)) + "_" + braceTex(cborToTex(subCbor));
            }else{
                return "{}_" + braceTex(cborToTex(subCbor)) + braceTex(cborToTex(baseCbor));
            }
        }
    }
    return "";
}

QString FmaDoc::superToTex(const QCborMap &cborMap)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }

    QString ret;
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap superCbor = cborArr[1].toMap();

            if(dir == int(Qt::LeftToRight)){
                return braceTex(cborToTex(baseCbor)) +"^" + braceTex(cborToTex(superCbor));
            }else{
                return "{}^" + braceTex(cborToTex(superCbor)) + braceTex(cborToTex(baseCbor));
            }
        }
    }
    return "";
}

QString FmaDoc::subSuperToTex(const QCborMap &cborMap)
{
    int dir = int(Qt::LeftToRight);
    QCborMap::const_iterator iter = IFmaBase::findCbor(cborMap, int(FmaCborId::Dir));
    if(iter != cborMap.end()){
        if(iter.value().toInteger() == int(Qt::RightToLeft)){
            dir = int(Qt::RightToLeft);
        }
    }

    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap subCbor = cborArr[1].toMap();
            QCborMap superCbor = cborArr[2].toMap();

            if(dir == int(Qt::LeftToRight)){
                return braceTex(cborToTex(baseCbor))
                        + "_" + braceTex(cborToTex(subCbor))
                        + "^" + braceTex(cborToTex(superCbor));
            }else{
                return "{}_"  + braceTex(cborToTex(subCbor))
                        + "^" + braceTex(cborToTex(superCbor))
                        + braceTex(cborToTex(baseCbor));
            }
        }
    }
    return "";
}

QString FmaDoc::tableToTex(const QCborMap &cborMap)
{
    QString ret;
    ret += "\\begin{bmatrix}\n";
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();

        int rowNum =0, colNum = 0;

        mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::RowNum));
        if(mapIter != cborMap.end()){
            rowNum = mapIter.value().toInteger();
        }

        mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::ColumnNum));
        if(mapIter != cborMap.end()){
            colNum = mapIter.value().toInteger();
        }


        for(int r=0; r<rowNum; ++r){
            if(r!=0){
                ret += "\\\\\n";
            }

            for(int c=0; c<colNum; ++c){
                if(c!=0){
                    ret += " & ";
                }

                int id = c+r*colNum;
                if(id < cborArr.size()){
                    QCborMap childCbor = cborArr[id].toMap();
                    ret += cborToTex(childCbor);
                }
            }
        }
    }
    ret += "\n\\end{bmatrix}";
    return ret;
}

QString FmaDoc::fencedToTex(const QCborMap &cborMap)
{
    QString ret;
    QString open = "(";
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Open));
    if(mapIter != cborMap.end()){
        open = mapIter.value().toString();
    }

    if(!open.isEmpty()){
        ret += "\\left " + braceTex(open) + " ";
    }else{
        ret += "\\left. ";
    }

    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 1){
            ret += braceTex(cborToTex(cborArr[0].toMap()));
        }
    }

    QString close = ")";
    mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Close));
    if(mapIter != cborMap.end()){
        open = mapIter.value().toString();
    }

    if(!open.isEmpty()){
        ret += "\\right " + braceTex(open);
    }else{
        ret += "\\right.";
    }
    return ret;
}



QString FmaDoc::encloseToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 1){
            QCborMap baseCbor = cborArr[0].toMap();
            //55349 is code for '�', tex no enclose support
            return QChar(55349) + braceTex(cborToTex(baseCbor));
        }
    }
    return "";
}

QString FmaDoc::overToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap overCbor = cborArr[1].toMap();

            QString baseStr = cborToTex(baseCbor);
            QString overStr = cborToTex(overCbor);

            QString literal = FmaMmlTable::texOverLiteral(overStr);
            if(literal.isEmpty()){
                return braceTex(baseStr) +"\\limits^" + braceTex(overStr);
            }else{
                return literal + braceTex(baseStr);
            }
        }
    }
    return "";
}

QString FmaDoc::underToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap underCbor = cborArr[1].toMap();

            return braceTex(cborToTex(baseCbor)) +"\\limits_" + braceTex(cborToTex(underCbor));
        }
    }
    return "";
}

QString FmaDoc::underOverToTex(const QCborMap &cborMap)
{
    QCborMap::const_iterator mapIter = IFmaBase::findCbor(cborMap, int(FmaCborId::Children));
    if(mapIter != cborMap.end()){
        const QCborArray cborArr = mapIter.value().toArray();
        if(cborArr.size() >= 2){
            QCborMap baseCbor = cborArr[0].toMap();
            QCborMap underCbor = cborArr[1].toMap();
            QCborMap overCbor = cborArr[2].toMap();

            return braceTex(cborToTex(baseCbor))
                    +"\\limits_" + braceTex(cborToTex(underCbor))
                    +"^" + braceTex(cborToTex(overCbor));
        }
    }
    return "";
}

QString FmaDoc::braceTex(const QString t)
{
    if(t.length() <= 1){
        return t;
    }
    return "{" + t + "}";
}



