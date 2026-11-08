#ifndef FMATEXTFORMATS_H
#define FMATEXTFORMATS_H

#include <QVector>
#include <QTextCharFormat>
#include <QHash>
#include <QFontMetricsF>
#include <QRegularExpression>
class FmaTextFormats
{
public:
    FmaTextFormats();

    enum MmlSpacing{
        Nonespace = 0,
        Veryverythinmathspace=1, //1/18em
        Verythinmathspace=2,     //2/18em
        Thinmathspace=3,         //3/18em
        Mediummathspace=4,       //4/18em
        Thickmathspace=5,        //5/18em
        Verythickmathspace=6,    //6/18em
        Veryverythickmathspace=7, //7/18em
        NegativeVeryverythinmathspace=-1, //-1/18em
        NegativeVerythinmathspace=-2,     //-2/18em
        NegativeThinmathspace=-3,         //-3/18em
        NegativeMediummathspace=-4,       //-4/18em
        NegativeThickmathspace=-5,        //-5/18em
        NegativeVerythickmathspace=-6,    //-6/18em
        NegativeVeryverythickmathspace=-7, //-7/18em
    };


    enum MmlFont
    {
        NormalFont = 0,
        FrakturFont,
        SansSerifFont,
        ScriptFont,
        MonospaceFont,
        DoublestruckFont,
        ArabicInitialFont,
        ArabicTailedFont,
        ArabicLoopedFont,
        ArabicStretchedFont,
    };

    enum MathVariant
    {
        NormalMV       = NormalFont,
        FrakturMV      = FrakturFont,  //尖角字体
        SansSerifMV    = SansSerifFont,   //笔划粗细均匀字体
        ScriptMV       = ScriptFont,  //潦草字体
        MonospaceMV    = MonospaceFont,  //等宽字体
        DoubleStruckMV = DoublestruckFont,  //双线字体，两支笔并行一起写的效果
        ArabicInitialMV = ArabicInitialFont,  //mml3中阿拉伯文字特征
        ArabicTailedMV = ArabicTailedFont,
        ArabicLoopedMV = ArabicLoopedFont,
        ArabicStretchedMV = ArabicStretchedFont,
        BoldMV         = 0x40,
        ItalicMV       = 0x80,
        BoldItalicMV = BoldMV | ItalicMV,
        BoldFrakturMV = BoldMV | FrakturMV,
        BoldScript = BoldMV | ScriptMV,
        BoldSansSerifMV = BoldMV | SansSerifMV,
        SansSerifItalicMV = SansSerifMV | ItalicMV,
        SansSerifItalicBoldMV = SansSerifMV | ItalicMV | BoldMV,
        InvalidMV = 0xFF
    };


    static QString mmlMathVariantString(int value);
    static int mmlInterpretMathVariant( const QString &value, bool *ok = nullptr);

    static inline qreal em(const QFont& fn){
        return QFontMetricsF( fn ).boundingRect( 'm' ).width();
    }

    static inline qreal ex(const QFont& fn){
        return QFontMetricsF( fn ).boundingRect( 'x' ).height();
    }

    static inline qreal lineWidth(const QFont& fn){
        return qMax( 1.0, QFontMetricsF( fn ).lineWidth() );
    }

    static void setDefaultFontFamily(MmlFont type, const QString& fam);
    static QString getDefaultFontFamily(MmlFont type=NormalFont);

    static void setBaseFontPixSize(qreal size);
    static qreal getBaseFontPixSize();
    static void setMinFontPixSize(qreal size);
    static qreal getMinFontPixSize();

    static qreal getScriptFontPixSize(int scriptLevel);
    static QFont getScriptFont(quint8 mvType, int scriptLevel);

    static QString spacingString(int value);
    static int spacingType(const QString& str);
    static qreal spacing(quint8 spaceType, int scriptLevel);

    static void init();

protected:
    struct MathVariantValue
    {
        QString value;
        int mv;
    };

    static const MathVariantValue* getMvData();
};

#endif // FMATEXTFORMATS_H
