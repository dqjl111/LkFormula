#include "fmawmfpaintengine.h"
#include <QVariant>
#include <QtMath>
#include <QIODevice>
#include <QFile>
#include <QPainterPath>
#include <QTextLayout>
#include <QTextLine>
#include <QFontMetricsF>
#include "util/lkutil.h"
#include <codecvt>
#include <locale>
#include <QTextCodec>


FmaWmfPaintEngine::FmaWmfPaintEngine(PaintEngineFeatures features)
    : QPaintEngine(features)
{
    paintType = PaintWMF;
}

bool FmaWmfPaintEngine::begin(QPaintDevice *e)
{
    if (!outputDevice) {
         qWarning("FmaWmfPaintEngine::begin(), no output device");
         return false;
     }

     if (!outputDevice->isOpen()) {
         if (!outputDevice->open(QIODevice::WriteOnly)) {
             qWarning("FmaWmfPaintEngine::begin(), could not open output device: '%s'",
                      qPrintable(outputDevice->errorString()));
             return false;
         }
     } else if (!outputDevice->isWritable()) {
         qWarning("FmaWmfPaintEngine::begin(), could not write to read-only output device: '%s'",
                  qPrintable(outputDevice->errorString()));
         return false;
     }

     setTextAlign(quint16(FmaWmfTextAlignMode::TA_BASELINE));
     if(!mapToHimetrics){
         setMapMode(quint16(FmaWmfMapMode::MM_ANISOTROPIC));
     }

     if(paintType == PaintWMF){
         dpiX = e->logicalDpiX();
         dpiY = e->logicalDpiY();
     }else{
         dpiX = e->physicalDpiX();
         dpiY = e->physicalDpiY();

         setTextColor(Qt::black);
         setBackMode(0x01);
         setBackColor(Qt::white);

         setWindowOrg(0, 0);
         setWindowExt(size.width(), size.height());
     }
     return true;
}

bool FmaWmfPaintEngine::end()
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 3;
    record->meta = FmaWmfRecord::METAEOF;
    helper.records.append(record);

    if(outputDevice){
        if(outputDevice->isOpen()){
            QSize t(mapX(size.width()), mapY(size.height()));
            helper.buildHeaderAsRecord(t, objTypes.size(), qMax(helper.records.size(), 232), dpiX);

            QDataStream s(outputDevice.get());
            helper.saveWmf(s, (paintType == PaintWMF));
        }
        outputDevice->close();
    }
    return true;
}

void FmaWmfPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    // always stream full gstate, which is not required, but...
    flags |= QPaintEngine::AllDirty;


    if (flags & QPaintEngine::DirtyBrush) {
        setBrush(state.brush());
    }

    if (flags & QPaintEngine::DirtyPen) {
        setPen(state.pen());
    }

    if (flags & QPaintEngine::DirtyTransform) {
        //Todo no support
    }

    if (flags & QPaintEngine::DirtyFont) {
        setFont(state.font());
    }

    if (flags & QPaintEngine::DirtyOpacity) {
        //Todo no support
    }
}

void FmaWmfPaintEngine::drawEllipse(const QRectF &r)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 7;
    record->meta = FmaWmfRecord::RECTANGLE;

    record->parameters.resize(8);
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t;
    t = mapY(QVariant(r.bottom()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(r.right()).toUInt()) & 0xFFFF;
    os << t;

    t = mapY(QVariant(r.top()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(r.left()).toUInt()) & 0xFFFF;
    os << t;

    helper.records.append(record);
}

void FmaWmfPaintEngine::drawPath(const QPainterPath &path)
{
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:{
            moveTo(QPointF(e.x, e.y));
            break;
        }
        case QPainterPath::LineToElement:{
            lineTo(QPointF(e.x, e.y));
            break;
        }
        default:{
            //No support
            break;
        }
        }
    }
}

void FmaWmfPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    drawImage(r, pm.toImage(), sr);
}

void FmaWmfPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if((points != nullptr) && (pointCount>0)){
        if(mode == OddEvenMode || mode == WindingMode){
            std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
            record->size = 3 + 1 + pointCount*2;
            record->meta = FmaWmfRecord::POLYGON;

            record->parameters.resize(1 + pointCount*2);
            QDataStream os(&record->parameters, QIODevice::WriteOnly);
            os.setByteOrder(QDataStream::LittleEndian);

            quint16 t;
            for(int i=0; i<pointCount; ++i){
                t = mapX(QVariant(points[i].x()).toUInt()) & 0xFFFF;
                os << t;

                t = mapY(QVariant(points[i].y()).toUInt()) & 0xFFFF;
                os << t;
            }
            helper.records.append(record);
        }else{
            if(pointCount >2){
                std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
                record->size = 3 + 1 + (pointCount+1)*2;
                record->meta = FmaWmfRecord::POLYLINE;

                record->parameters.resize(1 + (pointCount+1)*2);
                QDataStream os(&record->parameters, QIODevice::WriteOnly);
                os.setByteOrder(QDataStream::LittleEndian);

                quint16 t;
                for(int i=0; i<pointCount; ++i){
                    t = mapX(QVariant(points[i].x()).toUInt()) & 0xFFFF;
                    os << t;

                    t = mapY(QVariant(points[i].y()).toUInt()) & 0xFFFF;
                    os << t;
                }

                t = mapX(QVariant(points[0].x()).toUInt()) & 0xFFFF;
                os << t;

                t = mapY(QVariant(points[0].y()).toUInt()) & 0xFFFF;
                os << t;

                helper.records.append(record);
            }else if(pointCount == 2){
                moveTo(points[0]);
                lineTo(points[1]);
            }else if(pointCount == 1){
                moveTo(points[0]);
            }
        }
    }
}

void FmaWmfPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    for (int i=0; i < rectCount; ++i) {
        const QRectF &rect = rects[i].normalized();
        std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
        record->size = 7;
        record->meta = FmaWmfRecord::RECTANGLE;

        record->parameters.resize(8);
        QDataStream os(&record->parameters, QIODevice::WriteOnly);
        os.setByteOrder(QDataStream::LittleEndian);

        quint16 t;
        t = mapY(QVariant(rect.bottom()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(rect.right()).toUInt()) & 0xFFFF;
        os << t;

        t = mapY(QVariant(rect.top()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(rect.left()).toUInt()) & 0xFFFF;
        os << t;

        helper.records.append(record);
    }
}

void FmaWmfPaintEngine::drawTextItem(const QPointF &pt, const QTextItem &item)
{
    setFont(item.font());
    metaExTextout(pt, item);
}

void FmaWmfPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
    //No draw for fma
}

void FmaWmfPaintEngine::drawArc(const QRectF &r, int startAngle16, int spanAngle16)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::ARC;
    record->size = 11;
    record->parameters.resize(16);
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    QPointF startP = angleToArcPos(r, startAngle16);
    QPointF endP = angleToArcPos(r, startAngle16 + spanAngle16);

    quint16 t;

    if(spanAngle16 >=0){
        t = mapY(QVariant(endP.y()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(endP.x()).toUInt()) & 0xFFFF;
        os << t;

        t = mapY(QVariant(startP.y()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(startP.x()).toUInt()) & 0xFFFF;
        os << t;
    }else{
        t = mapY(QVariant(startP.y()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(startP.x()).toUInt()) & 0xFFFF;
        os << t;

        t = mapY(QVariant(endP.y()).toUInt()) & 0xFFFF;
        os << t;

        t = mapX(QVariant(endP.x()).toUInt()) & 0xFFFF;
        os << t;
    }

    t = mapY(QVariant(r.bottom()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(r.right()).toUInt()) & 0xFFFF;
    os << t;

    t = mapY(QVariant(r.top()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(r.left()).toUInt()) & 0xFFFF;
    os << t;


    helper.records.append(record);
}

void FmaWmfPaintEngine::drawMml(const QString &mml)
{
    QList<std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> > mmlRecords =
            FmaWmfEscapeEnhancedMetaFileRecord::buildFromMml(mml);

    for(std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> mmlRecord : mmlRecords){
        std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
        record->meta = FmaWmfRecord::ESCAPE;

        mmlRecord->buildFromMml(mml);
        record->parameters = mmlRecord->toParameters();
        record->size = 3 + record->parameters.size()/2;
        helper.records.append(record);
    }
}

void FmaWmfPaintEngine::moveTo(const QPointF &point)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 5;
    record->meta = FmaWmfRecord::MOVETO;

    record->parameters.resize(4);
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t;
    t = mapY(QVariant(point.y()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(point.x()).toUInt()) & 0xFFFF;
    os << t;

    helper.records.append(record);
}

void FmaWmfPaintEngine::lineTo(const QPointF &point)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 5;
    record->meta = FmaWmfRecord::LINETO;

    record->parameters.resize(4);
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t;
    t = mapY(QVariant(point.y()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(point.x()).toUInt()) & 0xFFFF;
    os << t;

    helper.records.append(record);
}

void FmaWmfPaintEngine::setBackMode(const quint16 mode)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETBKMODE;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << mode << quint16(0);

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setBackColor(const QColor color)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETBKCOLOR;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    os << quint8(color.red());
    os << quint8(color.green());
    os << quint8(color.blue());
    os << quint8(color.alpha());

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setTextColor(const QColor color)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETTEXTCOLOR;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << quint8(color.red());
    os << quint8(color.green());
    os << quint8(color.blue());
    os << quint8(color.alpha());

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setTextAlign(const quint16 alignMode)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETTEXTALIGN;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << alignMode << quint16(0);

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setWindowOrg(const quint16 x, const quint16 y)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETWINDOWORG;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    quint16 t = mapY(y) & 0xFFFF;
    os << t;
    t = mapX(x) & 0xFFFF;
    os << t;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setWindowExt(const quint16 x, const quint16 y)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETWINDOWEXT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t = mapY(y) & 0xFFFF;
    os << t;
    t = mapX(x) & 0xFFFF;
    os << t;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setScaleWindowExt(const quint16 denormX, const quint16 factX,
                                          const quint16 denormY, const quint16 factY)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SCALEWINDOWEXT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << denormY << factY << denormX << factX;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setViewOrg(const quint16 x, const quint16 y)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETVIEWPORTORG;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    quint16 t = mapY(y) & 0xFFFF;
    os << t;
    t = mapX(x) & 0xFFFF;
    os << t;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setViewExt(const quint16 x, const quint16 y)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETVIEWPORTEXT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    quint16 t = mapY(y) & 0xFFFF;
    os << t;
    t = mapX(x) & 0xFFFF;
    os << t;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setScaleViewExt(const quint16 denormX, const quint16 factX, const quint16 denormY, const quint16 factY)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SCALEVIEWPORTEXT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << denormY << factY << denormX << factX;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::setMapMode(const quint16 mode)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::SETMAPMODE;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << mode;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

QPaintEngine::Type FmaWmfPaintEngine::type() const
{
    return QPaintEngine::Type(paintType);
}

void FmaWmfPaintEngine::setBrush(const QBrush &brush)
{
    qint16 brushId = -1;
    QList<QPair<QBrush, qint16> >::iterator iter = brushList.begin();
    for(; iter != brushList.end(); ++iter){
        if(iter->first == brush){
            brushId = iter->second;
            break;
        }
    }

    if(brushId >=0){
        if(brushId != currentBrushId){
            selectObj(brushId);
            currentBrushId = brushId;
        }
    }else{
        //Create Object
        createBrush(brush);
        qint16 newObjId = buildNewObjId(FmaWmfObjType::Brush);
        brushList.append(QPair<QBrush, qint16>(brush, newObjId));
        //Select Object
        selectObj(newObjId);
        currentBrushId = newObjId;
    }
}

void FmaWmfPaintEngine::setFont(const QFont &font)
{
    QMap<QFont, qint16>::iterator iter = fontMap.find(font);
    if(iter != fontMap.end()){
        if(iter.value() != currentFontId){
            selectObj(iter.value());
            currentFontId = iter.value();
        }
    }
    else{
        createFont(font);
        qint16 newObjId = buildNewObjId(FmaWmfObjType::Font);
        fontMap[font] = newObjId;
        selectObj(newObjId);
        currentFontId = newObjId;
    }
}


void FmaWmfPaintEngine::setPen(const QPen &pen)
{
    qint16 penId = -1;
    QList<QPair<QPen, qint16> >::iterator iter = penList.begin();
    for(; iter != penList.end(); ++iter){
        if(iter->first == pen){
            penId = iter->second;
            break;
        }
    }

    if(penId >=0){
        if(penId != currentPenId){
            selectObj(penId);
            currentPenId = penId;
        }
    }else{
        //Create Object
        createPen(pen);
        qint16 newObjId = buildNewObjId(FmaWmfObjType::Pen);
        penList.append(QPair<QPen, qint16>(pen, newObjId));
        //Select Object
        selectObj(newObjId);
        currentPenId = newObjId;
    }
}

void FmaWmfPaintEngine::createBrush(const QBrush &brush)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::CREATEBRUSHINDIRECT;
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    FmaWmfHelper::WmfBrush wmfBrush;
    wmfBrush.fromQBrush(brush);

    os << wmfBrush.brushStyle;
    os << wmfBrush.colorRef.red;
    os << wmfBrush.colorRef.green;
    os << wmfBrush.colorRef.blue;
    os << wmfBrush.colorRef.reserved;
    os << wmfBrush.brushHatch;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::createFont(const QFont &font)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::CREATEFONTINDIRECT;
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    FmaWmfHelper::WmfFont wmfFont;
    wmfFont.fromQFont(font);

    wmfFont.height = mapY(wmfFont.height) & 0xFFFF;
    wmfFont.width = mapX(wmfFont.width) & 0xFFFF;

    os << wmfFont.height;
    os << wmfFont.width;
    os << wmfFont.escapement;
    os << wmfFont.orientation;
    os << wmfFont.weight;
    os << wmfFont.italic;
    os << wmfFont.underline;
    os << wmfFont.strikeout;
    os << wmfFont.charset;
    os << wmfFont.outPrecision;
    os << wmfFont.clipPrecisition;
    os << wmfFont.quality;
    os << wmfFont.pitchAndFamily;

    os.writeRawData(wmfFont.facename, strlen(wmfFont.facename));
    if(record->parameters.size()%2){
        os << quint8(0x00);
    }
    //os.writeRawData(wmfFont.facename, 32);

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}


void FmaWmfPaintEngine::createPen(const QPen &pen)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::CREATEPENINDIRECT;
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    FmaWmfHelper::WmfPen wmfPen;
    wmfPen.fromQPen(pen);

    os << wmfPen.penStyle;
    os << wmfPen.width;
    os << wmfPen.colorRef.red;
    os << wmfPen.colorRef.green;
    os << wmfPen.colorRef.blue;
    os << wmfPen.colorRef.reserved;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}


void FmaWmfPaintEngine::selectObj(qint16 objId)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 4;
    record->meta = FmaWmfRecord::SELECTOBJECT;
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << objId;
    helper.records.append(record);
}

void FmaWmfPaintEngine::deleteObj(qint16 objId)
{
    if(objId == currentFontId){
        currentFontId = -1;
    }else if(objId == currentPenId){
        currentPenId = -1;
    }else if(objId == currentBrushId){
        currentBrushId = -1;
    }

    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->size = 4;
    record->meta = FmaWmfRecord::DELETEOBJECT;
    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);
    os << objId;
    helper.records.append(record);
}

qint16 FmaWmfPaintEngine::getOrgObjId(FmaWmfObjType type)
{
    qint16 orgObjId = -1;
    for(qint16 i=0; i< objTypes.size(); ++i){
        if(objTypes[i] == type){
            orgObjId = i;
            break;
        }
    }
    return orgObjId;
}

qint16 FmaWmfPaintEngine::buildNewObjId(FmaWmfObjType type)
{
    qint16 newObjId = -1;
    if(type != FmaWmfObjType::NoType){
        for(qint16 i=0; i< objTypes.size(); ++i){
            if(objTypes[i] == FmaWmfObjType::NoType){
                newObjId = i;
                objTypes[i] = type;
                break;
            }
        }

        if(newObjId == -1){
            newObjId = objTypes.size();
            objTypes.append(type);
        }
    }
    return newObjId;
}

void FmaWmfPaintEngine::metaTextout(const QPointF &pt, const QTextItem &item)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::TEXTOUT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t;

    QByteArray text = item.text().toUtf8(); //windows system should enable utf-8
    if(textOutGbk){
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QTextCodec *gbk = QTextCodec::codecForName("GBK");
        text=gbk->fromUnicode(utf8->toUnicode(text));
    }
    t = text.length();
    if(t%2){
        text.append(char(0));
    }
    os << quint16(t & 0xFFFF);

    os.writeRawData(text.data(), text.length());
    t = mapY(QVariant(pt.y()).toUInt()) & 0xFFFF;
    os << t;

    t = mapX(QVariant(pt.x()).toUInt()) & 0xFFFF;
    os << t;

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

void FmaWmfPaintEngine::metaExTextout(const QPointF &pt, const QTextItem &item)
{
    std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
    record->meta = FmaWmfRecord::EXTTEXTOUT;

    QDataStream os(&record->parameters, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    quint16 t = mapY(QVariant(pt.y()).toUInt()) & 0xFFFF;
    os << t;
    t = mapX(QVariant(pt.x()).toUInt()) & 0xFFFF;
    os << t;
    QByteArray text = item.text().toUtf8(); //windows system should enable utf-8
    if(textOutGbk){
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QTextCodec *gbk = QTextCodec::codecForName("GBK");
        text=gbk->fromUnicode(utf8->toUnicode(text));
    }
    t = text.length();
    if(t%2){
        text.append(char(0));
    }
    os << quint16(t & 0xFFFF);

    if(item.renderFlags() & QTextItem::RightToLeft){
        os << quint16(FmaWmfTextOpt::ETO_RTLREADING);
    }else{
        os << quint16(0x0000);
    }
    os.writeRawData(text.data(), text.length());

    // 附加每个字符的横向间距数组（META_EXTTEXTOUT 标准可选字段），
    // 让 GDI 按与 Qt 相同的 advance 排版。
    // 用前缀绝对位置的差值，避免逐字符舍入误差累积导致尾部留白。
    const QString str = item.text();
    if(str.size() > 1 && text.length() == str.size()){
        // dx 数组：GDI 按“字符位置 = 起点 + 前序 dx 累计”排版。
        // 布局（FmaText::textSize）用含 letterSpacing 的 horizontalAdvance，
        // 绘制管线的度量不含 letterSpacing，这里显式补上，保证元素总宽一致。
        QFontMetricsF fm(item.font());
        const qreal ls = (item.font().letterSpacingType() == QFont::AbsoluteSpacing)
                ? item.font().letterSpacing() : 0.0;
        long prevPos = 0;
        for(int i = 1; i <= str.size(); ++i){
            const long pos = mapX(qRound(fm.horizontalAdvance(str.left(i)) + i * ls));
            os << qint16(qBound(-32768, int(pos - prevPos), 32767));
            prevPos = pos;
        }
    }

    record->size = 3 + record->parameters.size()/2;
    helper.records.append(record);
}

QPointF FmaWmfPaintEngine::angleToArcPos(const QRectF &r, int angle16)
{
    qreal a = (r.right() - r.left())/2;
    qreal b = (r.bottom() - r.top())/2;

    angle16 = angle16%(16*360);

    QPointF c = r.center();
    QPointF pos;
    if(angle16 == 0){
        pos.setX(c.x() + a);
        pos.setY(c.y());
    }else if(abs(angle16) == (16*180)){
        pos.setX(c.x() - a);
        pos.setY(c.y());
    }else if((angle16 == (16*270)) || (angle16 == -(16*90))){
        pos.setX(c.x());
        pos.setY(c.y()+b);
    }else if((angle16 == -(16*270)) || (angle16 == (16*90))){
        pos.setX(c.x());
        pos.setY(c.y()-b);
    }else{
        qreal tanAlpha = tan(qDegreesToRadians(abs(angle16)/16.0));
        qreal dx = a*b/sqrt(b*b+a*a*tanAlpha*tanAlpha);
        qreal dy = dx*tanAlpha;

        if(angle16 >0){
            if(angle16 < 90*16){
                pos.setX(c.x() + dx);
                pos.setY(c.y() - dy);
            }else if(angle16 < 180*16){
                pos.setX(c.x() - dx);
                pos.setY(c.y() - dy);
            }else if(angle16 < 270*16){
                pos.setX(c.x() - dx);
                pos.setY(c.y() + dy);
            }else{
                pos.setX(c.x() + dx);
                pos.setY(c.y() + dy);
            }
        }else{
            if(angle16 > -90*16){
                pos.setX(c.x() + dx);
                pos.setY(c.y() + dy);
            }else if(angle16 > -180*16){
                pos.setX(c.x() - dx);
                pos.setY(c.y() + dy);
            }else if(angle16 > -270*16){
                pos.setX(c.x() - dx);
                pos.setY(c.y() - dy);
            }else{
                pos.setX(c.x() + dx);
                pos.setY(c.y() - dy);
            }
        }
    }

    return pos;
}



