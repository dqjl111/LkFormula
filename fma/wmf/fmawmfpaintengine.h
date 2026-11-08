#ifndef FMAWMFPAINTENGINE_H
#define FMAWMFPAINTENGINE_H

#include <QPaintEngine>
#include <QDataStream>
#include <memory>
#include <QMap>
#include <QSet>
#include "fmawmf.h"

class FmaWmfPaintEngine : public QPaintEngine
{
public:
    explicit FmaWmfPaintEngine(PaintEngineFeatures features=PaintEngineFeatures());

    virtual bool begin(QPaintDevice *pdev) override;
    virtual bool end() override;

    virtual void updateState(const QPaintEngineState &state) override;

    void drawEllipse(const QRectF &r) override;
    void drawPath(const QPainterPath &path) override;
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
    void drawRects(const QRectF *rects, int rectCount) override;
    void drawTextItem(const QPointF &pt, const QTextItem &item) override;
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor) override;

    void drawArc(const QRectF &r, int startAngle16, int spanAngle16);

    void drawMml(const QString& mml);
    void moveTo(const QPointF& point);
    void lineTo(const QPointF& point);

    void setBackMode(const quint16 mode);
    void setBackColor(const QColor color);
    void setTextColor(const QColor color);

    void setTextAlign(const quint16 alignMode);

    void setWindowOrg(const quint16 x, const quint16 y);
    void setWindowExt(const quint16 x, const quint16 y);
    void setScaleWindowExt(const quint16 denormX, const quint16 factX,
                           const quint16 denormY, const quint16 factY);

    void setViewOrg(const quint16 x, const quint16 y);
    void setViewExt(const quint16 x, const quint16 y);
    void setScaleViewExt(const quint16 denormX, const quint16 factX,
                         const quint16 denormY, const quint16 factY);

    void setMapMode(const quint16 mode);

    virtual Type type() const override;

    enum{
        PaintWMF = QPaintEngine::User,
        PaintOLEPres
    };

    int paintType;
    QSize size;

    bool textOutGbk = true;
    bool mapToHimetrics = false;

    std::shared_ptr<QIODevice> outputDevice = nullptr;
protected:
    FmaWmfHelper helper;
    QVector<FmaWmfObjType> objTypes;

    QMap<QFont, qint16> fontMap;
    qint16 currentFontId = -1;
    QList<QPair<QPen, qint16> > penList;
    qint16 currentPenId = -1;
    QList<QPair<QBrush, qint16> > brushList;
    qint16 currentBrushId = -1;

    int dpiX =1;
    int dpiY =1;

    void setBrush(const QBrush& brush);
    void setFont(const QFont& font);
    void setPen(const QPen& pen);

    void createBrush(const QBrush& brush);
    void createFont(const QFont& font);
    void createPen(const QPen& pen);

    void selectObj(qint16 objId);
    void deleteObj(qint16 objId);
    qint16 getOrgObjId(FmaWmfObjType type);
    qint16 buildNewObjId(FmaWmfObjType type);

    void metaTextout(const QPointF &pt, const QTextItem &item);
    void metaExTextout(const QPointF &pt, const QTextItem &item);

    QPointF angleToArcPos(const QRectF &r, int angle16);
    inline long mapX(int x){
        return (mapToHimetrics? FmaWmfHelper::mapPixTohiMetrics(x, dpiX) : x);
    }
    inline long mapY(int y){
        return (mapToHimetrics? FmaWmfHelper::mapPixTohiMetrics(y, dpiY) : y);
    }
};

#endif // FMAWMFPAINTENGINE_H
