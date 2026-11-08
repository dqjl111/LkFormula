#include "fmawmfgenerator.h"
#include "fmawmfpaintengine.h"
#include <QFile>

#include <qt_windows.h>

FmaWmfGenerator::FmaWmfGenerator()
    :engine(new FmaWmfPaintEngine)
{

}

static int _sysDpiX=-1, _sysDpiY=-1;
int FmaWmfGenerator::sysDpiX()
{
    if (_sysDpiX < 0) {
#ifdef _WIN32
        const HDC displayDC = GetDC(0);
        if ( NULL != displayDC ) {
            _sysDpiX = GetDeviceCaps(displayDC, LOGPIXELSX);
            _sysDpiY = GetDeviceCaps(displayDC, LOGPIXELSY);
            ReleaseDC(0, displayDC);
        }else{
            _sysDpiX = 72;
            _sysDpiY = 72;
        }
#else
        _sysDpiX = 72;
        _sysDpiY = 72;
#endif
    }
    return _sysDpiX;
}

int FmaWmfGenerator::sysDpiY()
{
    if (_sysDpiY < 0) {
#ifdef _WIN32
        const HDC displayDC = GetDC(0);
        if ( NULL != displayDC ) {
            _sysDpiX = GetDeviceCaps(displayDC, LOGPIXELSX);
            _sysDpiY = GetDeviceCaps(displayDC, LOGPIXELSY);
            ReleaseDC(0, displayDC);
        }else{
            _sysDpiX = 72;
            _sysDpiY = 72;
        }
#else
        _sysDpiX = 72;
        _sysDpiY = 72;
#endif
    }
    return _sysDpiY;
}

QPaintEngine *FmaWmfGenerator::paintEngine() const
{
    return engine.get();
}

QSize FmaWmfGenerator::size() const
{
    return engine->size;
}

void FmaWmfGenerator::setSize(const QSize &size)
{
    if (engine->isActive()) {
        qWarning("FmaWmfGenerator::setSize(), cannot set size while WMF is being generated");
        return;
    }
    engine->size = size;
}

QString FmaWmfGenerator::fileName() const
{
    return m_fileName;
}

void FmaWmfGenerator::setFileName(const QString &fileName)
{
    if (engine->isActive()) {
        qWarning("FmaWmfGenerator::setFileName(), cannot set file name while WMF is being generated");
        return;
    }

    if (engine->outputDevice){
        engine->outputDevice = nullptr;
    }

    m_fileName = fileName;
    engine->outputDevice = std::make_shared<QFile>(m_fileName);
}

void FmaWmfGenerator::setIODevice(std::shared_ptr<QIODevice> dev)
{
    if (engine->isActive()) {
        qWarning("FmaWmfGenerator::setIODevice(), cannot set output device while WMF is being generated");
        return;
    }
    engine->outputDevice = dev;
}

void FmaWmfGenerator::setPaintType(int type)
{
    engine->paintType = type;
}

void FmaWmfGenerator::setMapToHimetrics(bool isDoMap)
{
    engine->mapToHimetrics = isDoMap;
}

void FmaWmfGenerator::setMml(const QString &mmlData)
{
    engine->drawMml(mmlData);
}

int FmaWmfGenerator::metric(PaintDeviceMetric metric) const
{
    switch (metric) {
    case QPaintDevice::PdmDepth:
        return 32;
    case QPaintDevice::PdmWidth:
        return engine->size.width();
    case QPaintDevice::PdmHeight:
        return engine->size.height();
    case QPaintDevice::PdmDpiX:
        return 72;
    case QPaintDevice::PdmDpiY:
        return 72;
    case QPaintDevice::PdmHeightMM:
        return qRound(engine->size.height() * 25.4 / sysDpiX());
    case QPaintDevice::PdmWidthMM:
        return qRound(engine->size.width() * 25.4 / sysDpiY());
    case QPaintDevice::PdmNumColors:
        return 0xffffffff;
    case QPaintDevice::PdmPhysicalDpiX:
        return sysDpiX();
    case QPaintDevice::PdmPhysicalDpiY:
        return sysDpiY();
    case QPaintDevice::PdmDevicePixelRatio:
        return 1;
    case QPaintDevice::PdmDevicePixelRatioScaled:
        return 1 * QPaintDevice::devicePixelRatioFScale();
    default:
        qWarning("QSvgGenerator::metric(), unhandled metric %d\n", metric);
        break;
    }
    return 0;
}
