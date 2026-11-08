#ifdef _WIN32

#include "fmatransdataobject.h"
#include "fma/fmamime.h"
#include <QDebug>
#include <QUuid>
#include <shlobj.h>

FmaTransDataObject::FmaTransDataObject(QMimeData *mimeData):
    data(mimeData),
    CF_PERFORMEDDROPEFFECT(RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT)),
    ref(1)
{
}

FmaTransDataObject::~FmaTransDataObject()
{
    releaseData();
}

void FmaTransDataObject::resetData(QMimeData *mimeData)
{
    if(data != mimeData){
        releaseData();
        data = mimeData;
    }
}

void FmaTransDataObject::releaseData()
{
    if(data){
        delete data;
        data = nullptr;
    }
}

QMimeData *FmaTransDataObject::mimeData() const
{
    return data.data();
}

DWORD FmaTransDataObject::reportedPerformedEffect() const
{
    return performedEffect;
}

HRESULT FmaTransDataObject::QueryInterface(const IID &iid, LPVOID *iface)
{
    if (!iface){
        return E_POINTER;
    }
    *iface = nullptr;

    if (iid == IID_IUnknown){
        *iface = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_IDataObject){
        *iface = static_cast<IDataObject*>(this);
    }

    if (!*iface){
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG FmaTransDataObject::AddRef()
{
    return InterlockedIncrement(&ref);
}

ULONG FmaTransDataObject::Release()
{
    LONG refCount = InterlockedDecrement(&ref);
    if (!refCount){
        delete this;
    }
    return refCount;
}

STDMETHODIMP FmaTransDataObject::GetData(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    if ( !pformatetc ){
        return DV_E_FORMATETC;
    }

    if ( !pmedium ){
        return STG_E_MEDIUMFULL;
    }

    pmedium->pUnkForRelease = nullptr;

    if ( pformatetc->dwAspect == DVASPECT_THUMBNAIL
         || pformatetc->dwAspect == DVASPECT_ICON
         || pformatetc->dwAspect == DVASPECT_DOCPRINT ){
        //qWarning() << "Invalid: dwAspect"<< pformatetc->dwAspect;
        return DV_E_DVASPECT;
    }

    HRESULT hr = ResultFromScode(DATA_E_FORMATETC);
    if (data) {
        if(FmaMime::_convertFromMime(*pformatetc, data, pmedium)){
            hr = ResultFromScode(S_OK);
        }
    }
    return hr;
}

STDMETHODIMP FmaTransDataObject::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    if ( !pformatetc ){
        return DV_E_FORMATETC;
    }

    if ( !pmedium ){
        return STG_E_MEDIUMFULL;
    }

    if ( pformatetc->cfFormat == FmaMime::cf_fmaEmbedObject
         || pformatetc->cfFormat == FmaMime::cf_fmaEmbedSource )
    {
        if ( !( pformatetc->tymed & TYMED_ISTORAGE ) ){
            return DV_E_TYMED;
        }

        if(pmedium->pstg == NULL){
            return STG_E_MEDIUMFULL;
        }

        if(data){
            QByteArray cborData = data->data(FmaMime::fmaCbor);
            quint8 mode = FmaMime::EmbedDirect;
            if(pformatetc->cfFormat == FmaMime::cf_fmaEmbedSource){
                mode = FmaMime::EmbedCreateCopy;
            }
            return FmaMime::getEmbed(cborData, pmedium, pformatetc->cfFormat, mode);
        }
    }
    return DV_E_FORMATETC;
}

STDMETHODIMP FmaTransDataObject::QueryGetData(FORMATETC *pformatetc)
{
    return FmaMime::queryGetData(pformatetc);
}

STDMETHODIMP FmaTransDataObject::GetCanonicalFormatEtc(FORMATETC *pformatetc, FORMATETC *pformatetcOut)
{
    return FmaMime::getCanonicalFormatEtc(pformatetc, pformatetcOut);
}

STDMETHODIMP FmaTransDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    HRESULT hr = ResultFromScode(E_NOTIMPL);

    if (pformatetc->cfFormat == CF_PERFORMEDDROPEFFECT && pmedium->tymed == TYMED_HGLOBAL) {
        DWORD * val = (DWORD*)GlobalLock(pmedium->hGlobal);
        performedEffect = *val;
        GlobalUnlock(pmedium->hGlobal);
        if (fRelease){
            ReleaseStgMedium(pmedium);
        }
        hr = ResultFromScode(S_OK);
    }
    return hr;
}

STDMETHODIMP FmaTransDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    if(!ppenumFormatEtc){
        return E_INVALIDARG;
    }

    if (!data){
        return ResultFromScode(DATA_E_FORMATETC);
    }

    QVector<FORMATETC> fmtetcs;
    if (dwDirection == DATADIR_GET) {
        fmtetcs = FmaMime::_formatsForMime(FmaMime::fmaCbor, data);
    } else {
        FORMATETC formatetc;
        formatetc.cfFormat = CLIPFORMAT(CF_PERFORMEDDROPEFFECT);
        formatetc.dwAspect = DVASPECT_CONTENT;
        formatetc.lindex = -1;
        formatetc.ptd = nullptr;
        formatetc.tymed = TYMED_HGLOBAL;
        fmtetcs.append(formatetc);
    }

    *ppenumFormatEtc = new FmaTransEnumFmtEtc(fmtetcs);
    return S_OK;
}

STDMETHODIMP FmaTransDataObject::DAdvise(FORMATETC FAR*, DWORD, LPADVISESINK, DWORD FAR*)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


STDMETHODIMP FmaTransDataObject::DUnadvise(DWORD)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP FmaTransDataObject::EnumDAdvise(LPENUMSTATDATA FAR*)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
///////////////////////////
FmaTransEnumFmtEtc::FmaTransEnumFmtEtc(const QVector<FORMATETC> &fmtetcs):
    m_nIndex(0),
    m_fmtetcs(fmtetcs),
    m_ref(1)
{
}

FmaTransEnumFmtEtc::~FmaTransEnumFmtEtc()
{
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) == NOERROR) {
        for (int idx = 0; idx < m_fmtetcs.count(); ++idx) {
            FORMATETC tmpetc = m_fmtetcs.at(idx);
            if (tmpetc.ptd){
                pmalloc->Free(tmpetc.ptd);
            }
        }
        pmalloc->Release();
    }
    m_fmtetcs.clear();
}


HRESULT FmaTransEnumFmtEtc::QueryInterface(const IID &iid, LPVOID *iface)
{
    if (!iface){
        return E_POINTER;
    }
    *iface = nullptr;

    if (iid == IID_IUnknown){
        *iface = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_IEnumFORMATETC){
        *iface = static_cast<IEnumFORMATETC*>(this);
    }

    if (!*iface){
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG FmaTransEnumFmtEtc::AddRef()
{
    return InterlockedIncrement(&m_ref);
}

ULONG FmaTransEnumFmtEtc::Release()
{
    LONG refCount = InterlockedDecrement(&m_ref);
    if (!refCount){
        delete this;
    }
    return refCount;
}


STDMETHODIMP FmaTransEnumFmtEtc::Next(ULONG celt, LPFORMATETC rgelt, ULONG *pceltFetched)
{
    ULONG i=0;
    ULONG nOffset;

    if (rgelt == nullptr){
        return ResultFromScode(E_INVALIDARG);
    }

    while (i < celt) {
        nOffset = m_nIndex + i;

        if (nOffset < ULONG(m_fmtetcs.count())) {
            copyFormatEtc(rgelt + i, &(m_fmtetcs.at(int(nOffset))));
            i++;
        } else {
            break;
        }
    }

    m_nIndex += i;

    if (pceltFetched != nullptr){
        *pceltFetched = i;
    }

    if (i != celt){
        return ResultFromScode(S_FALSE);
    }

    return NOERROR;
}

STDMETHODIMP FmaTransEnumFmtEtc::Skip(ULONG celt)
{
    ULONG i=0;
    ULONG nOffset;

    while (i < celt) {
        nOffset = m_nIndex + i;

        if (nOffset < ULONG(m_fmtetcs.count())) {
            i++;
        } else {
            break;
        }
    }

    m_nIndex += i;

    if (i != celt){
        return ResultFromScode(S_FALSE);
    }
    return NOERROR;
}

STDMETHODIMP FmaTransEnumFmtEtc::Reset()
{
    m_nIndex = 0;
    return NOERROR;
}

STDMETHODIMP FmaTransEnumFmtEtc::Clone(LPENUMFORMATETC *newEnum)
{
    if (newEnum == nullptr){
        return ResultFromScode(E_INVALIDARG);
    }

    FmaTransEnumFmtEtc *result = new FmaTransEnumFmtEtc(m_fmtetcs);
    result->m_nIndex = m_nIndex;
    *newEnum = result;
    return NOERROR;
}

bool FmaTransEnumFmtEtc::copyFormatEtc(LPFORMATETC dest, const FORMATETC *src) const
{
    if (dest == nullptr || src == nullptr){
        return false;
    }

    *dest = *src;

    if (src->ptd) {
        LPMALLOC pmalloc;

        if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR){
            return false;
        }

        dest->ptd = static_cast<DVTARGETDEVICE*>(pmalloc->Alloc(src->ptd->tdSize));
        if (!dest->ptd){
            pmalloc->Release();
            return false;
        }
        memcpy(dest->ptd, src->ptd, size_t(src->ptd->tdSize));

        pmalloc->Release();
    }

    return true;
}

#endif

