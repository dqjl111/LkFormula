#ifndef FMADOC_H
#define FMADOC_H

#include <memory>
#include <QMap>
#include <QJsonDocument>
#include "fmamath.h"
#include "fmarow.h"
#include "fmatext.h"
#include "fmafrac.h"
#include "fmasubsuperbase.h"
#include "fmaunderover.h"
#include "fmaroot.h"
#include "fmatable.h"
#include "fmaenclose.h"
#include "fmalongdiv.h"
#include "fmastack.h"
#include <QUndoStack>
#include <QDomElement>

#define _FMA_CLASSID "{25F69DC1-0B5E-4F15-90FF-352A4418967B}"

class IFmaBase;
class FmaDoc
{
public:
    FmaDoc();

    QUndoStack undoStack;

    bool saveCbor(const QString& filePath);
    bool loadCbor(const QString& filePath);
    bool loadCbor(const QByteArray& data, bool canClip = false);

    bool saveJson(const QString& filePath);
    bool loadJson(const QString& filePath);

    bool saveMml(const QString& filePath);
    bool loadMml(const QString& filePath);

    bool saveTex(const QString& filePath);


    bool saveSvg(const QString& filePath);
    const QPixmap grab();

    bool savePng(const QString& filePath);
    bool saveWmf(const QString& filePath);


    bool loadWmf(const QString& filePath);

    bool saveWmf(std::shared_ptr<QIODevice> dev,
                 bool outWmfFile = true,
                 bool withMml = true,
                 bool mapToHimetrics = false);
    bool loadWmf(std::shared_ptr<QIODevice> dev);

    static int childNumLimit(FmaType type);
    static IFmaBase* fromCbor(const QCborMap& cborMap, FmaMath* mathPtr = nullptr);


    FmaMath math;
protected:
    static bool hasInit;
    static QCborMap mmlToCbor(QDomElement* ele);
    static QCborMap mmlToCborStyle(QDomElement* ele);
    static void mmlToCborType(QDomElement* ele, QCborMap& cbor);
    static void mmlToCborTable(QDomElement* ele, QCborMap& cbor);

    static bool cborToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void generalToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc, int childNum = 1);

    static void mathToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void rowToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void moToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void miToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void mnToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void textToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void fracToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);

    static void tableToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void fencedToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void encloseToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);

    static bool styleToMml(const QCborMap& cborMap, QDomElement* ele);
    static bool typeToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);

    static void subToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void superToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);
    static void subSuperToMml(const QCborMap& cborMap, QDomElement* ele, QDomDocument* doc);


    static QString cborToTex(const QCborMap& cborMap);
    static QString typeToTex(const QCborMap& cborMap);

    static QString mathToTex(const QCborMap& cborMap);
    static QString moToTex(const QCborMap& cborMap);
    static QString miToTex(const QCborMap& cborMap);
    static QString mnToTex(const QCborMap& cborMap);
    static QString textToTex(const QCborMap& cborMap);

    static QString fracToTex(const QCborMap& cborMap);
    static QString rowToTex(const QCborMap& cborMap);
    static QString sqrtToTex(const QCborMap& cborMap);
    static QString rootToTex(const QCborMap& cborMap);

    static QString subToTex(const QCborMap& cborMap);
    static QString superToTex(const QCborMap& cborMap);
    static QString subSuperToTex(const QCborMap& cborMap);

    static QString tableToTex(const QCborMap& cborMap);
    static QString fencedToTex(const QCborMap& cborMap);
    static QString encloseToTex(const QCborMap& cborMap);

    static QString overToTex(const QCborMap& cborMap);
    static QString underToTex(const QCborMap& cborMap);
    static QString underOverToTex(const QCborMap& cborMap);

    static QString braceTex(const QString t);
};


#endif // FMADOC_H
