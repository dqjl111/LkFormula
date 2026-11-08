#ifndef FMAMIME_H
#define FMAMIME_H

#ifdef _WIN32

#include <QtCore/qt_windows.h>
#include <objidl.h>
#include <ole2.h>
#include <QWindowsMimeConverter>
#include <QVariant>
#include <QList>
#include <QMetaType>

class FmaDoc;
class FmaMime : public QWindowsMimeConverter
{
public:
    FmaMime();
    ~FmaMime() override;

    // for converting from Qt
    virtual bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const override;
    virtual bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const override;
    virtual QList<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const override;

    // for converting to Qt
    virtual bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const override;
    virtual QVariant convertToMime(const QString &mimeType, IDataObject *pDataObj, QMetaType preferredType) const override;
    virtual QString mimeForFormat(const FORMATETC &formatetc) const override;

    static bool _canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData);
    static bool _convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium);
    static QList<FORMATETC> _formatsForMime(const QString &mimeType, const QMimeData *mimeData);

    // for converting to Qt
    static bool _canConvertToMime(const QString &mimeType, IDataObject *pDataObj);
    static QVariant _convertToMime(const QString &mimeType, IDataObject *pDataObj, QMetaType preferredType);
    static QString _mimeForFormat(const FORMATETC &formatetc);

    static HRESULT queryGetData(FORMATETC *pformatetc);
    static HRESULT getCanonicalFormatEtc(FORMATETC* pformatetc,FORMATETC* pformatetcOut);

    static void getMetaSize(const FmaDoc* doc,long* x, long* y);
    static HRESULT loadEmbed(QByteArray& embedData, IStorage *pStg);
    static HRESULT saveEmbed(const QByteArray& embedData, IStorage *pStg, BOOL fSameAsLoad);

    static HRESULT getEmbed(const QByteArray& embedData, IStorage** ppStg, int cf);
    /**
     * @brief getEmbed
     * @param embedData
     * @param pmedium
     * @param mode 0 create doc and copy 1 create doc without copy 2 no create doc without copy
     * @return
     */
    enum EmbedGetMode{
        EmbedCreateCopy,
        EmbedCreate,
        EmbedDirect
    };
    static HRESULT getEmbed(const QByteArray& embedData,
                            STGMEDIUM *pmedium,
                            int cf,
                            quint8 mode = EmbedCreate);

    static HRESULT getWmfData(FmaDoc* doc, const FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    static HRESULT getEmfData(FmaDoc* doc, const FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

    static void init();

    const static QString fmaCbor;
    const static QString fmaMml;
    const static QString fmaTex;
    const static QString fmaEmbedSource;
    const static QString fmaEmbedObject;
    const static QString fmaObjectDescriptor;
    const static QUuid fmaClassId;

    const static QString fmaEmbedStream;

    static int cf_fmaCbor;
    static int cf_fmaMml;
    static int cf_fmaTex;
    static int cf_fmaEmbedSource;
    static int cf_fmaEmbedObject;
    static int cf_fmaObjectDescriptor;

private:
    static bool hasInit;
 //    static QString embedName;
};

#endif
#endif // FMAMIME_H
