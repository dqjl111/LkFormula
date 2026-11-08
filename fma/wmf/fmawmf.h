#ifndef FMAWMFH_H
#define FMAWMFH_H

#include <QDataStream>
#include <QVector>
#include <memory>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QFontMetrics>

class FmaWmfRecord
{
    Q_GADGET
public:
    FmaWmfRecord(){}

    enum Meta{
        METAEOF = 0x0000,
        REALIZEPALETTE = 0x0035,
        SETPALENTRIES = 0x0037,
        SETBKMODE = 0x0102,
        SETMAPMODE = 0x0103,
        SETROP2 = 0x0104,
        SETRELABS = 0x0105,
        SETPOLYFILLMODE = 0x0106,
        SETSTRETCHBLTMODE = 0x0107,
        SETTEXTCHAREXTRA = 0x0108,
        RESTOREDC = 0x0127,
        RESIZEPALETTE = 0x0139,
        DIBCREATEPATTERNBRUSH = 0x0142,
        SETLAYOUT = 0x0149,
        SETBKCOLOR = 0x0201,
        SETTEXTCOLOR = 0x0209,
        OFFSETVIEWPORTORG = 0x0211,
        LINETO = 0x0213,
        MOVETO = 0x0214,
        OFFSETCLIPRGN = 0x0220,
        FILLREGION = 0x0228,
        SETMAPPERFLAGS = 0x0231,
        SELECTPALETTE = 0x0234,
        POLYGON = 0x0324,
        POLYLINE = 0x0325,
        SETTEXTJUSTIFICATION = 0x020A,
        SETWINDOWORG = 0x020B,
        SETWINDOWEXT = 0x020C,
        SETVIEWPORTORG = 0x020D,
        SETVIEWPORTEXT = 0x020E,
        OFFSETWINDOWORG = 0x020F,
        SCALEWINDOWEXT = 0x0410,
        SCALEVIEWPORTEXT = 0x0412,
        EXCLUDECLIPRECT = 0x0415,
        INTERSECTCLIPRECT = 0x0416,
        ELLIPSE = 0x0418,
        FLOODFILL = 0x0419,
        FRAMEREGION = 0x0429,
        ANIMATEPALETTE = 0x0436,
        TEXTOUT = 0x0521,
        POLYPOLYGON = 0x0538,
        EXTFLOODFILL = 0x0548,
        RECTANGLE = 0x041B,
        SETPIXEL = 0x041F,
        ROUNDRECT = 0x061C,
        PATBLT = 0x061D,
        SAVEDC = 0x001E,
        PIE = 0x081A,
        STRETCHBLT = 0x0B23,
        ESCAPE = 0x0626,
        INVERTREGION = 0x012A,
        PAINTREGION = 0x012B,
        SELECTCLIPREGION = 0x012C,
        SELECTOBJECT = 0x012D,
        SETTEXTALIGN = 0x012E,
        ARC = 0x0817,
        CHORD = 0x0830,
        BITBLT = 0x0922,
        EXTTEXTOUT = 0x0a32,
        SETDIBTODEV = 0x0d33,
        DIBBITBLT = 0x0940,
        DIBSTRETCHBLT = 0x0b41,
        STRETCHDIB = 0x0f43,
        DELETEOBJECT = 0x01f0,
        CREATEPALETTE = 0x00f7,
        CREATEPATTERNBRUSH = 0x01F9,
        CREATEPENINDIRECT = 0x02FA,
        CREATEFONTINDIRECT = 0x02FB,
        CREATEBRUSHINDIRECT = 0x02FC,
        CREATEREGION = 0x06FF
    };
    Q_ENUM(Meta);

    static QString metaToString(quint16 meta);

    quint32 size =0;
    qint16 meta =0;
    QByteArray parameters;

    quint16 paramU16(quint32 index);

};

class FmaWmfExtTextout{
public:
    FmaWmfExtTextout(){}

    void fromRecord(std::shared_ptr<FmaWmfRecord> record);

    quint16 x=0;
    quint16 y=0;
    quint16 boxInfo=0;
    quint16 left=0;
    quint16 top=0;
    quint16 right=0;
    quint16 bottom=0;
    QString text;
};

class FmaWmfEscape{
    Q_GADGET
public:
    FmaWmfEscape(){}
    enum Type{
        NEWFRAME = 0x0001,
        ABORTDOC = 0x0002,
        NEXTBAND = 0x0003,
        SETCOLORTABLE = 0x0004,
        GETCOLORTABLE = 0x0005,
        FLUSHOUT = 0x0006,
        DRAFTMODE = 0x0007,
        QUERYESCSUPPORT = 0x0008,
        SETABORTPROC = 0x0009,
        STARTDOC = 0x000A,
        ENDDOC = 0x000B,
        GETPHYSPAGESIZE = 0x000C,
        GETPRINTINGOFFSET = 0x000D,
        GETSCALINGFACTOR = 0x000E,
        META_ESCAPE_ENHANCED_METAFILE = 0x000F,
        SETPENWIDTH = 0x0010,
        SETCOPYCOUNT = 0x0011,
        SETPAPERSOURCE = 0x0012,
        PASSTHROUGH = 0x0013,
        GETTECHNOLOGY = 0x0014,
        SETLINECAP = 0x0015,
        SETLINEJOIN = 0x0016,
        SETMITERLIMIT = 0x0017,
        BANDINFO = 0x0018,
        DRAWPATTERNRECT = 0x0019,
        GETVECTORPENSIZE = 0x001A,
        GETVECTORBRUSHSIZE = 0x001B,
        ENABLEDUPLEX = 0x001C,
        GETSETPAPERBINS = 0x001D,
        GETSETPRINTORIENT = 0x001E,
        ENUMPAPERBINS = 0x001F,
        SETDIBSCALING = 0x0020,
        EPSPRINTING = 0x0021,
        ENUMPAPERMETRICS = 0x0022,
        GETSETPAPERMETRICS = 0x0023,
        POSTSCRIPT_DATA = 0x0025,
        POSTSCRIPT_IGNORE = 0x0026,
        GETDEVICEUNITS = 0x002A,
        GETEXTENDEDTEXTMETRICS = 0x0100,
        GETPAIRKERNTABLE = 0x0102,
        EXTTEXTOUT = 0x0200,
        GETFACENAME = 0x0201,
        DOWNLOADFACE = 0x0202,
        METAFILE_DRIVER = 0x0801,
        QUERYDIBSUPPORT = 0x0C01,
        BEGIN_PATH = 0x1000,
        CLIP_TO_PATH = 0x1001,
        END_PATH = 0x1002,
        OPENCHANNEL = 0x100E,
        DOWNLOADHEADER = 0x100F,
        CLOSECHANNEL = 0x1010,
        POSTSCRIPT_PASSTHROUGH = 0x1013,
        ENCAPSULATED_POSTSCRIPT = 0x1014,
        POSTSCRIPT_IDENTIFY = 0x1015,
        POSTSCRIPT_INJECTION = 0x1016,
        CHECKJPEGFORMAT = 0x1017,
        CHECKPNGFORMAT = 0x1018,
        GET_PS_FEATURESETTING = 0x1019,
        MXDC_ESCAPE = 0x101A,
        SPCLPASSTHROUGH2 = 0x11D8
    };
    Q_ENUM(Type);

    static QString typeToString(quint16 type);

    void fromRecord(std::shared_ptr<FmaWmfRecord> record);

    qint16 type =0;
    quint16 byteCount =0;
};

class FmaWmfEscapeEnhancedMetaFileRecord
{
public:
    FmaWmfEscapeEnhancedMetaFileRecord(){};
    void fromRecord(std::shared_ptr<FmaWmfRecord> record);

    struct EmfHead{
        quint32 commentIdentifier;  //should be 0x43464D57
        quint32 commentType;
        quint32 version;
        quint16 checksum;
        quint32 flags;
        quint32 commentRecordCount;
        quint32 currentRecordSize;
        quint32 remainingBytes;
        quint32 enhancedMetafileDataSize;

        static int streamSize(){
            return 34;
        }
    };

    struct AppsMFCCHead{
        char id[8]; //should be "AppsMFCC"
        quint16 version;
        quint32 totalBytes;
        quint32 dataBytes;

        static int streamSize(){
            return 18;
        }
    };

    struct MTEFHead{
        char id[8]; //should be "MathType"
        quint16 magic;
        quint16 dataBytes;

        static int streamSize(){
            return 12;
        }
    };

    const static quint32 emfEscapeKey = 0x43464D57;

    enum HeadType{
        UNKNOWN_HEAD, EMF_HEAD, APPSMFCC_HEAD, MTEF_HEAD
    };

    HeadType headType=UNKNOWN_HEAD;
    QByteArray headData;

    QByteArray appsMfccSignature;
    QByteArray metafileData;

    EmfHead* emfHead();
    AppsMFCCHead* appsMfccHead();
    MTEFHead* mtefHead();

    static QList<std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> > buildFromMml(QString mmlData);
    QByteArray toParameters();
};

enum class FmaWmfCharset{
    ANSI = 0x00,
    DEFAULT = 0x01,
    SYMBOL = 0x02,
    MAC = 0x4D,
    SHIFTJIS = 0x80,
    HANGUL = 0x81,
    JOHAB = 0x82,
    GB2312 = 0x86,
    CHINESEBIG5 = 0x88,
    GREEK = 0xA1,
    TURKISH = 0xA2,
    VIETNAMESE = 0xA3,
    HEBREW = 0xB1,
    ARABIC = 0xB2,
    BALTIC = 0xBA,
    RUSSIAN = 0xCC,
    THAI = 0xDE,
    EASTEUROPE = 0xEE,
    OEM = 0xFF
};

enum class FmaWmfTextOpt{
    ETO_OPAQUE = 0x0002,
    ETO_CLIPPED = 0x0004,
    ETO_GLYPH_INDEX = 0x0010,
    ETO_RTLREADING = 0x0080,
    ETO_NUMERICSLOCAL = 0x0400,
    ETO_NUMERICSLATIN = 0x0800,
    ETO_PDY = 0x2000
};

enum class FmaWmfObjType{
    NoType,
    Brush,
    Font,
    Palette,
    Pen,
    Region,
    Other
};

enum class FmaWmfBrushStyle{
   BS_SOLID = 0x0000,
   BS_NULL = 0x0001,
   BS_HATCHED = 0x0002,
   BS_PATTERN = 0x0003,
   BS_INDEXED = 0x0004,
   BS_DIBPATTERN = 0x0005,
   BS_DIBPATTERNPT = 0x0006,
   BS_PATTERN8X8 = 0x0007,
   BS_DIBPATTERN8X8 = 0x0008,
   BS_MONOPATTERN = 0x0009
};

enum class FmaWmfPenStyle{
  PS_COSMETIC = 0x0000,
  PS_ENDCAP_ROUND = 0x0000,
  PS_JOIN_ROUND = 0x0000,
  PS_SOLID = 0x0000,
  PS_DASH = 0x0001,
  PS_DOT = 0x0002,
  PS_DASHDOT = 0x0003,
  PS_DASHDOTDOT = 0x0004,
  PS_NULL = 0x0005,
  PS_INSIDEFRAME = 0x0006,
  PS_USERSTYLE = 0x0007,
  PS_ALTERNATE = 0x0008,
  PS_ENDCAP_SQUARE = 0x0100,
  PS_ENDCAP_FLAT = 0x0200,
  PS_JOIN_BEVEL = 0x1000,
  PS_JOIN_MITER = 0x2000
};

enum class FmaWmfTextAlignMode{
    TA_NONE = 0x0000,
    TA_UPDATECP = 0x0001,
    TA_RIGHT = 0x0002,
    TA_CENTER = 0x0006,
    TA_BOTTOM = 0x0008,
    TA_BASELINE = 0x0018,
    TA_RTLREADING = 0x0100
};

enum class FmaWmfMapMode{
    MM_TEXT = 0x0001,
    MM_LOMETRIC = 0x0002,
    MM_HIMETRIC = 0x0003,
    MM_LOENGLISH = 0x0004,
    MM_HIENGLISH = 0x0005,
    MM_TWIPS = 0x0006,
    MM_ISOTROPIC = 0x0007,
    MM_ANISOTROPIC = 0x0008
};

class FmaWmfHelper
{
public:
    FmaWmfHelper();

    bool readWmf(QDataStream& wmfStream);
    bool saveWmf(QDataStream& wmfStream, bool withPlaceHeader = true);

    QByteArray fetchMmlData();

    void printRecords();

    struct PlaceableMetaHeader
    {
        quint32 key;      /* Magic number (always 9AC6CDD7h) */
        quint16 handle;   /* Metafile HANDLE number (always 0) */
        qint16 left;     /* Left coordinate in metafile units */
        qint16 top;      /* Top coordinate in metafile units */
        qint16 right;    /* Right coordinate in metafile units */
        qint16 bottom;   /* Bottom coordinate in metafile units */
        qint16 inch;     /* Number of metafile units per inch */
        quint32 reserved; /* Reserved (always 0) */
        quint16 checksum; /* Checksum value for previous 10 U16s */
    };

    struct WmfHead
    {
        quint16 fileType;      /* Type of metafile (1=memory, 2=disk) */
        quint16 headerSize;    /* Size of header in U16S (always 9) */
        quint16 version;       /* Version of Microsoft Windows used */
        quint32 fileSize;      /* Total size of the metafile in U16s */
        quint16 numOfObjects;  /* Number of objects in the file */
        quint32 maxRecordSize; /* The size of largest record in U16s */
        quint16 numOfParams;   /* Not Used (always 0) */
    };

    struct WmfFont
    {
        quint16 height;
        quint16 width;
        quint16 escapement;
        quint16 orientation;
        quint16 weight;
        quint8 italic;
        quint8 underline;
        quint8 strikeout;
        quint8 charset;
        quint8 outPrecision;
        quint8 clipPrecisition;
        quint8 quality;
        quint8 pitchAndFamily;
        char facename[32];

        void fromQFont(const QFont& font){
            QFontMetrics metrics = QFontMetrics( font);

            // GDI lfHeight 负值表示 em 高度，与 Qt pixelSize 一致；
            // 正值是含内部行距的字符单元高度，会让字体变小。
            height = -qAbs(font.pixelSize());
            // lfWidth 为 0 时 GDI 使用字体的自然宽度，
            // 否则比例字体被强制为统一平均宽度，字距异常。
            width = 0;
            weight = font.weight();
            escapement = 0;
            orientation = 0;

            if(font.italic()){
                italic = 0x01;
            }else{
                italic = 0x00;
            }

            if(font.underline()){
                underline = 0x01;
            }else{
                underline = 0x00;
            }

            if(font.strikeOut()){
                strikeout = 0x01;
            }else{
                strikeout = 0x00;
            }

            charset = quint8(FmaWmfCharset::DEFAULT);
            outPrecision = 0x00;
            clipPrecisition = 0x00;
            quality = 0x00;
            pitchAndFamily = 0x00;
            QByteArray fam =font.family().toLatin1();
            memset(facename, 0, 32);
            memcpy(facename, fam.data(), qMin(fam.size(), 32));
        }
    };

    struct WmfColor
    {
        quint8 red;
        quint8 green;
        quint8 blue;
        quint8 reserved;
    };

    struct WmfBrush
    {
        quint16 brushStyle;
        WmfColor colorRef;
        quint16 brushHatch;

        void fromQBrush(const QBrush& brush){
            switch(brush.style()){
            case Qt::NoBrush:{
                brushStyle = quint16(FmaWmfBrushStyle::BS_NULL);
                break;
            }
            default:{
                brushStyle = quint16(FmaWmfBrushStyle::BS_SOLID);
                break;
            }
            }

            QColor c = brush.color();
            colorRef.red = c.red();
            colorRef.green = c.green();
            colorRef.blue = c.blue();
            colorRef.reserved = c.alpha();
            brushHatch = 0x00;
        }
    };

    struct WmfPen{
        quint16 penStyle;
        quint32 width;
        WmfColor colorRef;

        void fromQPen(const QPen& pen){
            switch(pen.style()){
            case Qt::NoPen:{
                penStyle = quint16(FmaWmfPenStyle::PS_NULL);
                break;
            }
            case Qt::SolidLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_SOLID);
                break;
            }
            case Qt::DashLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_DASH);
                break;
            }
            case Qt::DotLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_DOT);
                break;
            }
            case Qt::DashDotLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_DASHDOT);
                break;
            }
            case Qt::DashDotDotLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_DASHDOTDOT);
                break;
            }
            case Qt::CustomDashLine:{
                penStyle = quint16(FmaWmfPenStyle::PS_USERSTYLE);
            }
            default:{
                penStyle = quint16(FmaWmfPenStyle::PS_SOLID);
                break;
            }
            }

            width = pen.width();
            QColor c = pen.color();
            colorRef.red = c.red();
            colorRef.green = c.green();
            colorRef.blue = c.blue();
            colorRef.reserved = c.alpha();
        }
    };


    QVector<std::shared_ptr<FmaWmfRecord> > records;

    PlaceableMetaHeader placeHead;
    WmfHead wmfHead;

    const static quint32 wmfKey = 0x9ac6cdd7;
    void buildHeaderAsRecord(QSize size, quint16 objNum, quint32 maxRecord, qint16 dpi);

    static inline long mapPixTohiMetrics(int x, qreal logicalDpi)
    {
        return qRound((qreal(x) * 2540) / logicalDpi);
    }
protected:
    bool readHead(QDataStream& wmfStream);
    void readRecords(QDataStream& wmfStream);

    bool writeHead(QDataStream& wmfStream, bool withPlaceHeader);
    void writeRecords(QDataStream& wmfStream);

    quint32 metaSize();
};


#endif // FMAWMFH_H
