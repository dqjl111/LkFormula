#ifndef FMAWMFGENERATOR_H
#define FMAWMFGENERATOR_H

#include <QObject>
#include <QPaintDevice>
#include <memory>
#include "fmawmfpaintengine.h"

class FmaWmfGenerator : public QPaintDevice
{
    Q_PROPERTY(QSize size READ size WRITE setSize)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
public:
    explicit FmaWmfGenerator();

    virtual QPaintEngine *paintEngine() const override;

    QSize size() const;
    void setSize(const QSize &size);

    QString fileName() const;
    void setFileName(const QString &fileName);

    void setIODevice(std::shared_ptr<QIODevice> dev);
    void setPaintType(int type);
    void setMapToHimetrics(bool isDoMap);

    void setMml(const QString& mmlData);

    static int sysDpiX();
    static int sysDpiY();

protected:
    int metric(QPaintDevice::PaintDeviceMetric metric) const override;

    std::unique_ptr<FmaWmfPaintEngine> engine;
    QString m_fileName;
};

#endif // FMAWMFGENERATOR_H
