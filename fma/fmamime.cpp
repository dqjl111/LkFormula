#ifdef _WIN32

#include "wmf/fmawmfgenerator.h"
#include "fmamime.h"
#include "fma/fmadoc.h"
#include "util/nodeleter.h"
#include <QMimeData>
#include <QApplication>
#include <QDebug>

#define CF_EMBEDSOURCE      "Embed Source"
#define CF_EMBEDDEDOBJECT   "Embedded Object"
#define CF_OBJECTDESCRIPTOR "Object Descriptor"

const QString FmaMime::fmaCbor(QStringLiteral("FMA Cbor"));
const QString FmaMime::fmaMml(QStringLiteral("MathML Presentation"));
const QString FmaMime::fmaTex(QStringLiteral("text/plain"));
const QString FmaMime::fmaEmbedSource(QStringLiteral(CF_EMBEDSOURCE));
const QString FmaMime::fmaEmbedObject(QStringLiteral(CF_EMBEDDEDOBJECT));
const QString FmaMime::fmaObjectDescriptor(QStringLiteral(CF_OBJECTDESCRIPTOR));
const QString FmaMime::fmaEmbedStream(QStringLiteral("fmaEmbedStream"));

const QUuid FmaMime::fmaClassId(_FMA_CLASSID);

bool FmaMime::hasInit = false;

int FmaMime::cf_fmaCbor =0;
int FmaMime::cf_fmaMml =0;
int FmaMime::cf_fmaTex =0;
int FmaMime::cf_fmaEmbedSource =0;
int FmaMime::cf_fmaEmbedObject =0;
int FmaMime::cf_fmaObjectDescriptor =0;
//QString FmaMime::embedName =0;


FmaMime::FmaMime()
{
    init();
}

FmaMime::~FmaMime()
{
}

void FmaMime::init()
{
    if(!hasInit){
        hasInit = true;

        cf_fmaCbor = FmaMime::registerMimeType(FmaMime::fmaCbor);
        cf_fmaMml = FmaMime::registerMimeType(FmaMime::fmaMml);
        cf_fmaTex = FmaMime::registerMimeType(FmaMime::fmaTex);
        cf_fmaEmbedSource = FmaMime::registerMimeType(FmaMime::fmaEmbedSource);
        cf_fmaEmbedObject = FmaMime::registerMimeType(FmaMime::fmaEmbedObject);
        cf_fmaObjectDescriptor = FmaMime::registerMimeType(FmaMime::fmaObjectDescriptor);
    }
}

bool FmaMime::canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const
{
    return _canConvertFromMime(formatetc, mimeData);
}

bool FmaMime::convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const
{
    return _convertFromMime(formatetc, mimeData, pmedium);
}

QList<FORMATETC> FmaMime::formatsForMime(const QString &mimeType, const QMimeData *mimeData) const
{
    return _formatsForMime(mimeType, mimeData);
}

bool FmaMime::canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const
{
    return _canConvertToMime(mimeType, pDataObj);
}

QVariant FmaMime::convertToMime(const QString &mimeType, IDataObject *pDataObj, QMetaType preferredType) const
{
    return _convertToMime(mimeType, pDataObj, preferredType);
}

QString FmaMime::mimeForFormat(const FORMATETC &formatetc) const
{
    return _mimeForFormat(formatetc);
}

bool FmaMime::_canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData)
{
    if(mimeData){
        if(mimeData->hasFormat(fmaCbor)){
            if (formatetc.cfFormat == cf_fmaCbor
                    || formatetc.cfFormat == cf_fmaEmbedSource
                    || formatetc.cfFormat == cf_fmaEmbedObject
                    || formatetc.cfFormat == CF_METAFILEPICT
                    || formatetc.cfFormat == CF_ENHMETAFILE
                    ) {
                return true;
            }
        }
    }
    return false;
}

bool FmaMime::_convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium)
{
    if (_canConvertFromMime(formatetc, mimeData)) {
         if (formatetc.cfFormat == cf_fmaCbor) {
            QByteArray cborData = mimeData->data(fmaCbor);

            HGLOBAL hData = GlobalAlloc(0, SIZE_T(cborData.size()));
            if (!hData){
                return false;
            }
            void *out = GlobalLock(hData);
            memcpy(out, cborData.data(), size_t(cborData.size()));
            GlobalUnlock(hData);
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hData;
            pmedium->pUnkForRelease = nullptr;
            return true;
         }
         else if(formatetc.cfFormat == FmaMime::cf_fmaObjectDescriptor){
             //qWarning() << "Get descriptor";

             HGLOBAL hData = GlobalAlloc(0, sizeof(OBJECTDESCRIPTOR));
             if (!hData){
                 return E_OUTOFMEMORY;
             }
             LPOBJECTDESCRIPTOR desc = (LPOBJECTDESCRIPTOR)GlobalLock(hData);
             //memcpy(out, cborData.data(), size_t(cborData.size()));
             desc->cbSize = sizeof(OBJECTDESCRIPTOR);
             desc->clsid = FmaMime::fmaClassId;
             desc->dwDrawAspect = DVASPECT_CONTENT;

             QByteArray cborData = mimeData->data(fmaCbor);
             FmaDoc doc;
             doc.loadCbor(cborData, true);

             getMetaSize(&doc, &(desc->sizel.cx), &(desc->sizel.cy));

             desc->pointl.x = 0;
             desc->pointl.y = 0;
             desc->dwStatus = 0;
             desc->dwFullUserTypeName = sizeof(OBJECTDESCRIPTOR);
             desc->dwSrcOfCopy = sizeof(OBJECTDESCRIPTOR);

             GlobalUnlock(hData);
             pmedium->tymed = TYMED_HGLOBAL;
             pmedium->hGlobal = hData;
             pmedium->pUnkForRelease = nullptr;
             return true;
         }
         else if(formatetc.cfFormat == cf_fmaEmbedSource
                 || formatetc.cfFormat == cf_fmaEmbedObject
                 ){
             if ( !( formatetc.tymed & TYMED_ISTORAGE ) ){
                 return false;
             }

             if(pmedium->pstg != NULL){
                 return false;
             }

             QByteArray cborData = mimeData->data(fmaCbor);
             return getEmbed(cborData, pmedium, formatetc.cfFormat, FmaMime::EmbedCreate);
         }
         else if (formatetc.cfFormat == CF_METAFILEPICT) {
             QByteArray cborData = mimeData->data(fmaCbor);
             FmaDoc doc;
             doc.loadCbor(cborData, true);

             return getWmfData(&doc, &formatetc, pmedium) == S_OK;
         }
         else if (formatetc.cfFormat == CF_ENHMETAFILE) {
             QByteArray cborData = mimeData->data(fmaCbor);
             FmaDoc doc;
             doc.loadCbor(cborData, true);

             return getEmfData(&doc, &formatetc, pmedium) == S_OK;
          }
    }
    return false;
}

QList<FORMATETC> FmaMime::_formatsForMime(const QString &mimeType, const QMimeData *mimeData)
{
    QList<FORMATETC> ret;
    if (mimeType.startsWith(fmaCbor)) {
        {
            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

            formatetc.tymed = TYMED_HGLOBAL;
            formatetc.cfFormat = cf_fmaCbor;
            ret.append(formatetc);
        }
        {
            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

            formatetc.tymed = TYMED_HGLOBAL;
            formatetc.cfFormat = cf_fmaObjectDescriptor;
            ret.append(formatetc);
        }
        {
            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

            formatetc.tymed = TYMED_ISTORAGE;
            formatetc.cfFormat = cf_fmaEmbedSource;
            ret.append(formatetc);
        }
        {
            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

            formatetc.tymed = TYMED_ISTORAGE;
            formatetc.cfFormat = cf_fmaEmbedObject;
            ret.append(formatetc);
        }
    }
    return ret;
}

bool FmaMime::_canConvertToMime(const QString &mimeType, IDataObject *pDataObj)
{
    if(pDataObj){
        if(mimeType.startsWith(fmaCbor)){
            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

            CLIPFORMAT cf_embObj = static_cast<CLIPFORMAT>(RegisterClipboardFormatW( L"Embedded Object" ));
            formatetc.tymed = TYMED_ISTORAGE;
            formatetc.cfFormat = cf_embObj;
            if (pDataObj->QueryGetData(&formatetc) == S_OK){
                 return true;
            }

            CLIPFORMAT cf_embSource = static_cast<CLIPFORMAT>(RegisterClipboardFormatW( L"Embed Source" ));
            formatetc.tymed = TYMED_ISTORAGE;
            formatetc.cfFormat = cf_embSource;
            if (pDataObj->QueryGetData(&formatetc) == S_OK){
                 return true;
            }
        }
    }
    return false;
}

QVariant FmaMime::_convertToMime(const QString &mimeType, IDataObject *pDataObj, QMetaType preferredType)
{
    if(pDataObj){
        if(mimeType.startsWith(fmaCbor)){
            STGMEDIUM s;
            memset((char*)(&s), 0, sizeof(STGMEDIUM));

            FORMATETC formatetc;
            formatetc.dwAspect = DVASPECT_CONTENT;
            formatetc.lindex = -1;
            formatetc.ptd = nullptr;

        }
    }
    return QVariant();
}

QString FmaMime::_mimeForFormat(const FORMATETC &formatetc)
{
    if((formatetc.cfFormat == CF_METAFILEPICT)
            || (formatetc.cfFormat == cf_fmaEmbedSource)
            || (formatetc.cfFormat == cf_fmaEmbedObject)
            || (formatetc.cfFormat == cf_fmaCbor)){
        return fmaCbor;
    }
    return QString();
}

HRESULT FmaMime::queryGetData(FORMATETC *pformatetc)
{
    if ( pformatetc )
    {
        if ( pformatetc->dwAspect == DVASPECT_THUMBNAIL
             || pformatetc->dwAspect == DVASPECT_ICON
             || pformatetc->dwAspect == DVASPECT_DOCPRINT ){
            return DV_E_DVASPECT;
        }

        if(pformatetc->cfFormat == cf_fmaCbor){
            if ( !( pformatetc->tymed & TYMED_HGLOBAL ) ){
                return DV_E_TYMED;
            }
            return S_OK;
        }

        if(pformatetc->cfFormat == cf_fmaObjectDescriptor){
            if ( !( pformatetc->tymed & TYMED_HGLOBAL ) ){
                return DV_E_TYMED;
            }
            return S_OK;
        }

        if ( pformatetc->cfFormat == CF_METAFILEPICT )
        {
            if ( !( pformatetc->tymed & TYMED_MFPICT ) ){
                return DV_E_TYMED;
            }
            return S_OK;
        }
#ifdef _WITH_EMF
        else if ( pformatetc->cfFormat == CF_ENHMETAFILE )
        {
            if ( !( pformatetc->tymed & TYMED_ENHMF ) ){
                return DV_E_TYMED;
            }
            return S_OK;
        }
#endif
        else
        {
            if ( pformatetc->cfFormat == FmaMime::cf_fmaEmbedObject
                 || pformatetc->cfFormat == FmaMime::cf_fmaEmbedSource )
            {
                if ( !( pformatetc->tymed & TYMED_ISTORAGE ) ){
                    return DV_E_TYMED;
                }
                return S_OK;
            }
        }
    }
    return DV_E_FORMATETC;
}

HRESULT FmaMime::getCanonicalFormatEtc(FORMATETC *pformatetc, FORMATETC *pformatetcOut)
{
    //qWarning() << "IDataObject:GetCanonicalFormatEtc";
    if ( !pformatetc || !pformatetcOut )
        return DV_E_FORMATETC;

    pformatetcOut->ptd = nullptr;
    pformatetcOut->cfFormat = pformatetc->cfFormat;
    pformatetcOut->dwAspect = DVASPECT_CONTENT;

    if(pformatetc->cfFormat == cf_fmaCbor){
        pformatetcOut->tymed = TYMED_HGLOBAL;
        return S_OK;
    }

    if(pformatetc->cfFormat == cf_fmaObjectDescriptor){
        pformatetcOut->tymed = TYMED_HGLOBAL;
        return S_OK;
    }

    if ( pformatetc->cfFormat == CF_METAFILEPICT )
    {
        pformatetcOut->tymed = TYMED_MFPICT;
        return S_OK;
    }
#ifdef _WITH_EMF
    else if ( pformatetc->cfFormat == CF_ENHMETAFILE )
    {
        pformatetc->tymed = TYMED_ENHMF;
        return S_OK;
    }
#endif
    else
    {
        if ( pformatetc->cfFormat == FmaMime::cf_fmaEmbedObject
             || pformatetc->cfFormat == FmaMime::cf_fmaEmbedSource )
        {
            pformatetcOut->tymed = TYMED_ISTORAGE;
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

void FmaMime::getMetaSize(const FmaDoc *doc, long *x, long *y)
{
    if(doc){
        int w = doc->math.fullWidth();
        int h = doc->math.fullHeight();

        if(w>0 && h >0){
            *x = FmaWmfHelper::mapPixTohiMetrics(w+2, FmaWmfGenerator::sysDpiX());
            *y = FmaWmfHelper::mapPixTohiMetrics(h+2,FmaWmfGenerator::sysDpiY());
            return;
        }
    }
    QFont font = FmaTextFormats::getScriptFont(FmaTextFormats::NormalMV, 0);
    QFontMetricsF metrics = QFontMetricsF( font);
    QRectF rect = metrics.boundingRect(QStringLiteral("XXXX"));
    *x = FmaWmfHelper::mapPixTohiMetrics(rect.width()+2, FmaWmfGenerator::sysDpiX());
    *y = FmaWmfHelper::mapPixTohiMetrics(rect.height()+2, FmaWmfGenerator::sysDpiY());
}

HRESULT FmaMime::loadEmbed(QByteArray& embedData, IStorage *pStg)
{
    IStream *spStream = 0;

    pStg->OpenStream(reinterpret_cast<const wchar_t *>(FmaMime::fmaEmbedStream.utf16()), 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spStream);
    if (!spStream){
        return E_FAIL;
    }

    STATSTG stat;
    HRESULT hres = spStream->Stat(&stat, STATFLAG_DEFAULT);

    if (hres == S_OK) {
        QString streamName = QString::fromWCharArray(stat.pwcsName);
        CoTaskMemFree(stat.pwcsName);
        if (stat.cbSize.HighPart){ // more than 4GB - too large!
            return S_FALSE;
        }

        embedData.resize(int(stat.cbSize.LowPart));
        ULONG read;
        spStream->Read(embedData.data(), stat.cbSize.LowPart, &read);
    }
    spStream->Release();
    return S_OK;
}

HRESULT FmaMime::saveEmbed(const QByteArray& embedData, IStorage *pStg, BOOL fSameAsLoad)
{
    if(!embedData.isEmpty()){
        IStream *spStream = 0;

        pStg->CreateStream(reinterpret_cast<const wchar_t *>(FmaMime::fmaEmbedStream.utf16()), STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStream);
        if (!spStream){
            return E_FAIL;
        }

        ULONG written = 0;
        const char *data = embedData.constData();
        ULARGE_INTEGER newsize;
        newsize.HighPart = 0;
        newsize.LowPart = DWORD(embedData.size());
        spStream->SetSize(newsize);
        spStream->Write(data, ULONG(embedData.size()), &written);
        spStream->Commit(STGC_ONLYIFCURRENT);

        spStream->Release();
    }
    return S_OK;
}


HRESULT FmaMime::getEmbed(const QByteArray &embedData, IStorage **ppStg, int cf)
{
    LPLOCKBYTES lpLockBytes = NULL;
    HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, (LPLOCKBYTES *)&lpLockBytes);
    if (hr != NOERROR){
        return hr;
    }

    IStorage *pStg = NULL;
    hr = StgCreateDocfileOnILockBytes(
        lpLockBytes,
        STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
        0,	// reserved
        &pStg
    );

    lpLockBytes->Release();
    if (hr != NOERROR){
        return hr;
    }

    if(pStg){
        hr = WriteClassStg(pStg, fmaClassId);

        if (hr == NOERROR){
            hr = saveEmbed( embedData, pStg, false);
            if ( FAILED( hr ) ){
                pStg->Release();
                return hr;
            }
            pStg->Commit(STGC_ONLYIFCURRENT);
            *ppStg = pStg;
            return S_OK;
        }
        return hr;
    }else{
        return STG_E_MEDIUMFULL;
    }

}

HRESULT FmaMime::getEmbed(const QByteArray &embedData, STGMEDIUM *pmedium, int cf, quint8 mode)
{
    if(mode == EmbedDirect){
        IStorage *pStg = pmedium->pstg;
        if(pStg){
            HRESULT hr;
            hr = WriteClassStg(pStg, fmaClassId);
            if (hr == NOERROR){
                hr = saveEmbed( embedData, pStg, false);
                if ( FAILED( hr ) ){
                    return hr;
                }
                pStg->Commit(STGC_ONLYIFCURRENT);
                return S_OK;
            }
            return hr;
        }else{
            return STG_E_INVALIDPARAMETER;
        }
    }else{
        IStorage *pStg = NULL;
        HRESULT hr = getEmbed(embedData, &pStg, cf);
        if(hr == S_OK){
            if(mode == EmbedCreateCopy){
                hr = pStg->CopyTo(0, NULL, NULL, pmedium->pstg);
                if (hr == NOERROR){
                    pmedium->tymed=TYMED_ISTORAGE;
                    pmedium->pstg->Commit(STGC_DEFAULT);
                    pmedium->pUnkForRelease=NULL;
                }
            }else{
                pmedium->tymed = TYMED_ISTORAGE;
                pmedium->pstg = pStg;
                pmedium->pstg->AddRef();
                pmedium->pUnkForRelease = pStg;
            }
            pStg->Release();
        }
        return hr;
    }
}

HRESULT FmaMime::getWmfData(FmaDoc *doc, const FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    if(!doc){
        return E_OUTOFMEMORY;
    }

    if (!pmedium){
        return E_POINTER;
    }
    if ((pformatetcIn->tymed & TYMED_MFPICT) == 0){
        return DATA_E_FORMATETC;
    }

    QByteArray data;
    QBuffer buffer(&data);
    std::shared_ptr<QIODevice> dev = noDeletePtr(buffer);
    doc->saveWmf(dev, false, false, false);
    buffer.close();

    HMETAFILE hMF = SetMetaFileBitsEx(data.size(), (const BYTE*)data.data());
    if (!hMF){
        return E_UNEXPECTED;
    }

    HGLOBAL hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(METAFILEPICT));
    if (!hMem) {
        DeleteMetaFile(hMF);
        return ResultFromScode(STG_E_MEDIUMFULL);
    }

    LPMETAFILEPICT pMF = static_cast<LPMETAFILEPICT>(GlobalLock(hMem));
    pMF->hMF = hMF;
    pMF->mm = MM_ANISOTROPIC;

    SIZEL size;
    getMetaSize(doc, &size.cx, &size.cy);
    pMF->xExt = size.cx;
    pMF->yExt = size.cy;
    GlobalUnlock(hMem);

    memset(pmedium, 0, sizeof(STGMEDIUM));
    pmedium->tymed = TYMED_MFPICT;
    pmedium->hMetaFilePict = hMem;
    pmedium->pUnkForRelease = 0;
    return S_OK;
}

HRESULT FmaMime::getEmfData(FmaDoc *doc, const FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    if(!doc){
        return E_OUTOFMEMORY;
    }

    if (!pmedium){
        return E_POINTER;
    }
    if ((pformatetcIn->tymed & TYMED_MFPICT) == 0){
        return DATA_E_FORMATETC;
    }

    QByteArray data;
    QBuffer buffer(&data);
    std::shared_ptr<QIODevice> dev = noDeletePtr(buffer);
    doc->saveWmf(dev, false, false, false);
    buffer.close();

    METAFILEPICT pict;
    pict.mm = MM_ANISOTROPIC;
    SIZEL size;
    getMetaSize(doc, &size.cx, &size.cy);
    pict.xExt = size.cx;
    pict.yExt = size.cy;
    pict.hMF = NULL;

    HENHMETAFILE hEMF = SetWinMetaFileBits(data.size(), (const BYTE*)data.data(), NULL, &pict);
    if(!hEMF){
        return E_UNEXPECTED;
    }

    memset(pmedium, 0, sizeof(STGMEDIUM));
    pmedium->tymed = TYMED_ENHMF;
    pmedium->hEnhMetaFile = hEMF;
    pmedium->pUnkForRelease = 0;
    return S_OK;
}

#endif
