#include "fmatextformats.h"
#include <QFontDatabase>
#include <QApplication>


static QStringList fonts;
static QString s_normal_font_name = "Times New Roman";
static QString s_fraktur_font_name = "Fraktur";
static QString s_sans_serif_font_name = "Luxi Sans";
static QString s_script_font_name = "Urw Chancery L";
static QString s_monospace_font_name = "Luxi Mono";
static QString s_doublestruck_font_name = "Doublestruck";
static QString s_arabic_initial_font_name = "";
static QString s_arabic_tailed_font_name = "";
static QString s_arabic_looped_font_name = "";
static QString s_arabic_streched_font_name = "";
static qreal baseFontPixSize = 24;
static qreal g_min_font_pixel_size_calc = 8.0;
static qreal g_script_size_multiplier = 0.5;


FmaTextFormats::FmaTextFormats()
{

}

QString FmaTextFormats::mmlMathVariantString(int value)
{
    const MathVariantValue *v = getMvData();
    for ( ; !v->value.isNull(); ++v )
    {
        if ( value == v->mv )
        {
            return v->value;
        }
    }
    return "";

}

int FmaTextFormats::mmlInterpretMathVariant(const QString &value, bool *ok)
{
    const MathVariantValue *v = getMvData();
    for ( ; !v->value.isNull(); ++v )
    {
        if ( value == v->value )
        {
            if ( ok != 0 ){
                *ok = true;
            }
            return v->mv;
        }
    }

    if ( ok != 0 ){
        *ok = false;
    }

    qWarning( "interpretMathVariant(): could not parse value: \"%s\"", qPrintable( value ) );
    return NormalMV;
}

void FmaTextFormats::setDefaultFontFamily(FmaTextFormats::MmlFont type, const QString &fam)
{
    switch (type) {
    case NormalFont:{
        s_normal_font_name = fam;
        break;
    }
    case FrakturFont:{
        s_fraktur_font_name = fam;
        break;
    }
    case SansSerifFont:{
        s_sans_serif_font_name = fam;
        break;
    }
    case ScriptFont:{
        s_script_font_name = fam;
        break;
    }
    case MonospaceFont:{
        s_monospace_font_name = fam;
        break;
    }
    case DoublestruckFont:{
        s_doublestruck_font_name = fam;
        break;
    }
    case ArabicInitialFont:{
        s_arabic_initial_font_name = fam;
        break;
    }
    case ArabicTailedFont:{
        s_arabic_tailed_font_name = fam;
        break;
    }
    case ArabicLoopedFont:{
        s_arabic_looped_font_name = fam;
        break;
    }
    case ArabicStretchedFont:{
        s_arabic_streched_font_name = fam;
        break;
    }
    default:{
        break;
    }
    }

}

QString FmaTextFormats::getDefaultFontFamily(FmaTextFormats::MmlFont type)
{
    switch (type) {
    case NormalFont:{
        return s_normal_font_name;
    }
    case FrakturFont:{
        return s_fraktur_font_name;
    }
    case SansSerifFont:{
        return s_sans_serif_font_name;
    }
    case ScriptFont:{
        return s_script_font_name;
    }
    case MonospaceFont:{
        return s_monospace_font_name;
    }
    case DoublestruckFont:{
        return s_doublestruck_font_name;
    }
    case ArabicInitialFont:{
        return s_arabic_initial_font_name;
    }
    case ArabicTailedFont:{
        return s_arabic_tailed_font_name;
    }
    case ArabicLoopedFont:{
        return s_arabic_looped_font_name;
    }
    case ArabicStretchedFont:{
        return s_arabic_streched_font_name;
    }
    default:{
        return s_normal_font_name;
    }
    }
}

void FmaTextFormats::setBaseFontPixSize(qreal size)
{
    baseFontPixSize = size;
}

qreal FmaTextFormats::getBaseFontPixSize()
{
    return baseFontPixSize;
}

void FmaTextFormats::setMinFontPixSize(qreal size)
{
    g_min_font_pixel_size_calc = size;
}

qreal FmaTextFormats::getMinFontPixSize()
{
    return g_min_font_pixel_size_calc;
}

qreal FmaTextFormats::getScriptFontPixSize(int sl)
{
    qreal ps = baseFontPixSize;
    if ( sl >= 0 )
    {
        for ( int i = 0; i < sl; ++i ){
            ps = ps * g_script_size_multiplier;
        }
    }
    else
    {
        for ( int i = 0; i > sl; --i ){
            ps = ps /g_script_size_multiplier;
        }
    }
    if ( ps < g_min_font_pixel_size_calc ){
        ps = g_min_font_pixel_size_calc;
    }
    return ps;
}

QFont FmaTextFormats::getScriptFont(quint8 mvType, int sl)
{
    FmaTextFormats::MmlFont fFam = FmaTextFormats::MmlFont(mvType & 0x3F);
    QFont font(getDefaultFontFamily(fFam));

    quint8 fontSize = getScriptFontPixSize(sl);

    if ( mvType & BoldMV ){
        font.setBold( true );
    }

    if ( mvType & ItalicMV ){
        font.setItalic( true );
    }

    font.setPixelSize(fontSize);
    return font;
}

const FmaTextFormats::MathVariantValue *FmaTextFormats::getMvData()
{
    static const MathVariantValue g_mv_data[] =
    {
        { "normal",                 NormalMV },
        { "bold",                   BoldMV },
        { "italic",                 ItalicMV },
        { "bold-italic",            BoldMV | ItalicMV },
        { "double-struck",          DoubleStruckMV },
        { "bold-fraktur",           BoldMV | FrakturMV },
        { "script",                 ScriptMV },
        { "bold-script",            BoldMV | ScriptMV },
        { "fraktur",                FrakturMV },
        { "sans-serif",             SansSerifMV },
        { "bold-sans-serif",        BoldMV | SansSerifMV },
        { "sans-serif-italic",      SansSerifMV | ItalicMV },
        { "sans-serif-bold-italic", SansSerifMV | ItalicMV | BoldMV },
        { "monospace",              MonospaceMV },
        { "initial",                ArabicInitialMV },
        { "tailed",                 ArabicTailedMV },
        { "looped",                 ArabicLoopedMV },
        { "stretched",              ArabicStretchedMV },
        { QString(),               0 }
    };

    const MathVariantValue *v = g_mv_data;
    return v;
}


QString FmaTextFormats::spacingString(int value)
{

    switch (value) {
    case Nonespace:{
        return "";
    }
    case Veryverythinmathspace:{
        return "veryverythinmathspace";
    }
    case Verythinmathspace:{
        return "verythinmathspace";
    }
    case Thinmathspace:{
        return "thinmathspace";
    }
    case Mediummathspace:{
        return "mediummathspace";
    }
    case Thickmathspace:{
        return "thickmathspace";
    }
    case Verythickmathspace:{
        return "verythickmathspace";
    }
    case Veryverythickmathspace:{
        return "veryverythickmathspace";
    }
    case NegativeVeryverythinmathspace:{
        return "negativeveryverythinmathspace";
    }
    case NegativeVerythinmathspace:{
        return "negativeverythinmathspace";
    }
    case NegativeThinmathspace:{
        return "negativethinmathspace";
    }
    case NegativeMediummathspace:{
        return "negativemediummathspace";
    }
    case NegativeThickmathspace:{
        return "negativethickmathspace";
    }
    case NegativeVerythickmathspace:{
        return "negativeverythickmathspace";
    }
    case NegativeVeryverythickmathspace:{
        return "negativeveryverythickmathspace";
    }
    default:{
        return "";
    }
    }
}

int FmaTextFormats::spacingType(const QString &str)
{
    for(int i=Nonespace; i<=Veryverythickmathspace; ++i ){
        if(str == spacingString(i)){
            return i;
        }
    }
    return Nonespace;
}

qreal FmaTextFormats::spacing(quint8 spaceType, int scriptLevel)
{
    switch (spaceType) {
    case Veryverythinmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.0555556;
    }
    case Verythinmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.111111;
    }
    case Thinmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.166667;
    }
    case Mediummathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.222222;
    }
    case Thickmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.277778;
    }
    case Verythickmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.333333;
    }
    case Veryverythickmathspace:{
        return getScriptFontPixSize(scriptLevel) * 0.388889;
    }
    case NegativeVeryverythinmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.0555556;
    }
    case NegativeVerythinmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.111111;
    }
    case NegativeThinmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.166667;
    }
    case NegativeMediummathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.222222;
    }
    case NegativeThickmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.277778;
    }
    case NegativeVerythickmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.333333;
    }
    case NegativeVeryverythickmathspace:{
        return -getScriptFontPixSize(scriptLevel) * 0.388889;
    }
    default:{
        return 0;
    }
    }
}

void FmaTextFormats::init()
{
    if ( QFontDatabase().hasFamily( "Cambria" ) ){
        s_normal_font_name = "Cambria";
    }else if ( QFontDatabase().hasFamily( "Times New Roman" ) ){
        s_normal_font_name = "Times New Roman";
    }
}
