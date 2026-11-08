#ifdef _WIN32

#include "fmaaggregated.h"
#include <qt_windows.h>
#include <QDebug>
#include "fmamain.h"
#include "util/nodeleter.h"
#include <wingdi.h>
#include "fmabinder.h"
#include "fma/fmamime.h"

//#define _LOG_TEST_ON

//ReleaseInterface calls 'Release' and NULLs the pointer
#define ReleaseInterface(p)\
{\
    if (NULL!=p)\
{\
    p->Release();\
    p=NULL;\
    }\
    }


FmaAggregated::FmaAggregated()
{
    FmaMime::init();
}

FmaAggregated::~FmaAggregated()
{
    if(_pIOleAdviseHolder){
        _pIOleAdviseHolder->SendOnClose();
    }

    ReleaseInterface(_spClientSite);
    ReleaseInterface(_pIDataAdviseHolder);
    ReleaseInterface(_pIOleAdviseHolder);

    FmaBinder* b = binder();
    if(b){
        b->aggergated = nullptr;
    }
}

long FmaAggregated::queryInterface(const QUuid &iid, void **iface)
{
    if (!iface){
        return E_POINTER;
    }
    *iface = nullptr;

    if (iid == IID_IOleObject){
        *iface = static_cast<IOleObject*>(this);
    }
    else if (iid == IID_IDataObject){
        *iface = static_cast<IDataObject*>(this);
    }
    else if (iid == IID_IPersist){
        *iface = static_cast<IPersist *>(static_cast<IPersistStorage *>(this));
    }
    else if (iid == IID_IPersistStorage){
        *iface = static_cast<IPersistStorage*>(this);
    }

    if (!*iface){
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

// IOleObject
STDMETHODIMP FmaAggregated::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:Advise";
#endif

    if (NULL== _pIOleAdviseHolder)
    {
        HRESULT     hr;
        hr=CreateOleAdviseHolder(&_pIOleAdviseHolder);
        if (FAILED(hr)){
            return hr;
        }
    }
    return _pIOleAdviseHolder->Advise(pAdvSink, pdwConnection);
}

STDMETHODIMP FmaAggregated::Close(DWORD dwSaveOption)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:Close";
#endif
    if(_pIDataAdviseHolder){
        _pIDataAdviseHolder->SendOnDataChange(this, 0, 0);
    }

    if(_spClientSite){
        if (dwSaveOption != OLECLOSE_NOSAVE){
            _isDirty = true;
            _spClientSite->SaveObject();
            if(_pIOleAdviseHolder){
                _pIOleAdviseHolder->SendOnSave();
            }
        }
        _spClientSite->OnShowWindow(false);
    }
    return S_OK;
}

STDMETHODIMP FmaAggregated::DoVerb(LONG iVerb,
                                   LPMSG lpmsg,
                                   IOleClientSite *pActiveSite,
                                   LONG lindex,
                                   HWND hwndParent,
                                   LPCRECT lprcPosRect)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:DoVerb" << iVerb;
#endif
    switch(iVerb){
    case OLEIVERB_PRIMARY:
    case OLEIVERB_SHOW:
    case OLEIVERB_OPEN:{
        binder()->setVisible(true, true);
        if(pActiveSite){
            pActiveSite->ShowObject();
            pActiveSite->OnShowWindow(true);
        }else{
            qWarning() << "No pActiveSite";
        }
        return S_OK;
    }
    case OLEIVERB_HIDE:{
        binder()->setVisible(false);
        if(pActiveSite){
            pActiveSite->OnShowWindow(false);
        }else{
            qWarning() << "No pActiveSite";
        }
        return S_OK;
    }
    default:{
        return E_NOTIMPL;
    }
    }
}

STDMETHODIMP FmaAggregated::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:EnumAdvise";
#endif
    if (NULL!= _pIOleAdviseHolder){
        return _pIOleAdviseHolder->EnumAdvise(ppenumAdvise);
    }
    return ResultFromScode(E_FAIL);
}

STDMETHODIMP FmaAggregated::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:EnumVerbs";
#endif
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::GetClientSite(IOleClientSite **ppClientSite)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetClientSite";
#endif
    if (!ppClientSite){
        return E_POINTER;
    }
    *ppClientSite = _spClientSite;
    if (*ppClientSite){
        (*ppClientSite)->AddRef();
    }
    return S_OK;
}

STDMETHODIMP FmaAggregated::GetClipboardData(DWORD dwReserved, IDataObject **ppDataObject)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetClipboardData";
#endif
    if(ppDataObject){
        IDataObject *iface = dynamic_cast<IDataObject*>(this);
        if(*ppDataObject == NULL){
            *ppDataObject = iface;
            iface->AddRef();
            return S_OK;
        }else if(*ppDataObject == iface){
            return S_OK;
        }else{
            return E_INVALIDARG;
        }
    }
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetExtent";
#endif
    if(dwDrawAspect != DVASPECT_CONTENT ){
        return E_INVALIDARG;
    }

    if ( !psizel ){
        return E_INVALIDARG;
    }


    std::shared_ptr<FmaDoc>  doc = binder()->doc();
    FmaMime::getMetaSize(doc.get(), &(psizel->cx), &(psizel->cy));
    return S_OK;
}

STDMETHODIMP FmaAggregated::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetMiscStatus";
#endif
    IOleObject *iface = dynamic_cast<IOleObject *>(controllingUnknown());
    return iface->GetMiscStatus(dwAspect, pdwStatus);
}

STDMETHODIMP FmaAggregated::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    //qWarning() << "IOleObject:GetMoniker";

    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::GetUserClassID(CLSID *clsid)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetUserClassID";
#endif
    if(clsid){
        *clsid = FmaMime::fmaClassId;
        return S_OK;
    }
    return E_POINTER;
}

STDMETHODIMP FmaAggregated::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:GetUserType";
#endif
    IOleObject *iface = dynamic_cast<IOleObject *>(controllingUnknown());
    return iface->GetUserType(dwFormOfType, pszUserType);
}

STDMETHODIMP FmaAggregated::InitFromData(IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:InitFromData";
#endif
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::IsUpToDate()
{
#ifdef _LOG_TEST_ON
    //qWarning() << "IOleObject:IsUpToDate";
#endif
    //return OLE_E_UNAVAILABLE;
    return S_OK;
}

STDMETHODIMP FmaAggregated::SetClientSite(IOleClientSite *pClientSite)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:SetClientSite";
#endif
    // release all client site interfaces
    if (_spClientSite){
        _spClientSite->Release();
    }

    _spClientSite = pClientSite;
    if (_spClientSite) {
        _spClientSite->AddRef();
    }

    return S_OK;
}

STDMETHODIMP FmaAggregated::SetColorScheme(LOGPALETTE *pLogPal)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:SetColorScheme";
#endif
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:SetExtent";
#endif
    if (dwDrawAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;
    if (!psizel){
        return E_POINTER;
    }
    return S_OK;
}

STDMETHODIMP FmaAggregated::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:SetHostNames";
#endif
    return S_OK;
}

STDMETHODIMP FmaAggregated::SetMoniker(DWORD dwWhichMoniker, IMoniker *ppmk)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:SetMoniker";
#endif
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::Unadvise(DWORD dwConnection)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:Unadvise";
#endif
    if (NULL!=_pIOleAdviseHolder){
        return _pIOleAdviseHolder->Unadvise(dwConnection);
    }
    return ResultFromScode(E_FAIL);
}

STDMETHODIMP FmaAggregated::Update()
{
#ifdef _LOG_TEST_ON
    qWarning() << "IOleObject:Update";
#endif
    return S_OK;
}


// IDataObject
STDMETHODIMP FmaAggregated::GetData(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:GetData";
#endif
    if ( !pformatetc ){
        //qWarning() << "Invalid: pformatetcIn";
        return DV_E_FORMATETC;
    }

    if ( !pmedium ){
        //qWarning() << "Invalid: pmedium";
        return STG_E_MEDIUMFULL;
    }

    if ( pformatetc->dwAspect == DVASPECT_THUMBNAIL
         || pformatetc->dwAspect == DVASPECT_ICON
         || pformatetc->dwAspect == DVASPECT_DOCPRINT ){
        //qWarning() << "Invalid: dwAspect"<< pformatetc->dwAspect;
        return DV_E_DVASPECT;
    }

    if ( pformatetc->cfFormat == CF_METAFILEPICT )
    {
        std::shared_ptr<FmaDoc>  doc = binder()->doc();
        return FmaMime::getWmfData(doc.get(), pformatetc, pmedium);
    }
#ifdef _WITH_EMF
    else if ( pformatetc->cfFormat == CF_ENHMETAFILE )
    {
        std::shared_ptr<FmaDoc>  doc = binder()->doc();
        return FmaMime::getEmfData(doc.get(), pformatetc, pmedium);
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

            if(pmedium->pstg != NULL){
                return STG_E_INVALIDPARAMETER;
            }

            std::shared_ptr<FmaDoc>  doc = binder()->doc();
            if(doc){
                QByteArray cborData = doc->math.toCbor().toCborValue().toCbor();
                return FmaMime::getEmbed(cborData, pmedium, pformatetc->cfFormat, FmaMime::EmbedCreate);
            }else{
                E_OUTOFMEMORY;
            }
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP FmaAggregated::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:GetDataHere";
#endif

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

        std::shared_ptr<FmaDoc>  doc = binder()->doc();
        if(doc){
            QByteArray cborData = doc->math.toCbor().toCborValue().toCbor();
            quint8 mode = FmaMime::EmbedDirect;
            if(pformatetc->cfFormat == FmaMime::cf_fmaEmbedSource){
                mode = FmaMime::EmbedCreateCopy;
            }
            return FmaMime::getEmbed(cborData, pmedium, pformatetc->cfFormat, mode);
        }else{
            E_OUTOFMEMORY;
        }
    }
    return DV_E_FORMATETC;
}

STDMETHODIMP FmaAggregated::QueryGetData(FORMATETC *pformatetc)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:QueryGetData";
#endif
    return FmaMime::queryGetData(pformatetc);
}

STDMETHODIMP FmaAggregated::GetCanonicalFormatEtc(FORMATETC *pformatetc, FORMATETC *pformatetcOut)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:GetCanonicalFormatEtc";
#endif
    return FmaMime::getCanonicalFormatEtc(pformatetc, pformatetcOut);
}

STDMETHODIMP FmaAggregated::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    //qWarning() << "IDataObject:SetData";
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:EnumFormatEtc";
#endif
    if ( dwDirection == DATADIR_GET ){
        return OLE_S_USEREG;
    }
    return E_NOTIMPL;
}

STDMETHODIMP FmaAggregated::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:DAdvise";
#endif
    if (NULL== _pIDataAdviseHolder)
    {
        HRESULT hr=CreateDataAdviseHolder(&_pIDataAdviseHolder);
        if (FAILED(hr)){
            return ResultFromScode(E_OUTOFMEMORY);
        }
    }

    return _pIDataAdviseHolder->Advise(this, pformatetc, advf, pAdvSink, pdwConnection);
}

STDMETHODIMP FmaAggregated::DUnadvise(DWORD dwConnection)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:DUnadvise";
#endif
    if (NULL==_pIDataAdviseHolder){
        return ResultFromScode(E_FAIL);
    }
    return _pIDataAdviseHolder->Unadvise(dwConnection);
}

STDMETHODIMP FmaAggregated::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IDataObject:EnumDAdvise";
#endif
    if (NULL==_pIDataAdviseHolder){
        return ResultFromScode(E_FAIL);
    }
    return _pIDataAdviseHolder->EnumAdvise(ppenumAdvise);
}


FmaMain *FmaAggregated::window() const
{
    FmaBinder * binder = dynamic_cast<FmaBinder *>(object());
    return binder->window();
}

FmaBinder *FmaAggregated::binder() const
{
    return dynamic_cast<FmaBinder *>(object());
}

//**** IPersist
STDMETHODIMP FmaAggregated::GetClassID(GUID*clsid)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersist:GetClassID";
#endif
    if(clsid){
        *clsid = FmaMime::fmaClassId;
        return S_OK;
    }
    return E_POINTER;
}

//**** IPersistStorage
STDMETHODIMP FmaAggregated::IsDirty()
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:IsDirty" << _isDirty;
#endif
    return _isDirty ? S_OK : S_FALSE;
}

STDMETHODIMP FmaAggregated::InitNew(IStorage *pStg)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:InitNew";
#endif
    IPersistStorage *iface = dynamic_cast<IPersistStorage *>(controllingUnknown());
    iface->InitNew(pStg);
    return S_OK;
}

STDMETHODIMP FmaAggregated::Load(IStorage *pStg)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:Load";
#endif
    std::shared_ptr<FmaDoc>  doc = binder()->doc();
    if(doc){
        if (InitNew(pStg) != S_OK){
            return CO_E_ALREADYINITIALIZED;
        }

        QByteArray embedData;
        FmaMime::loadEmbed(embedData, pStg);
        doc->loadCbor(embedData, true);
        return S_OK;
    }else{
        return E_FAIL;
    }
}

STDMETHODIMP FmaAggregated::Save(IStorage *pStg, BOOL fSameAsLoad)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:Save";
#endif
    std::shared_ptr<FmaDoc>  doc = binder()->doc();
    if(doc){
        QByteArray embedData = doc->math.toCbor().toCborValue().toCbor();
        HRESULT ret = FmaMime::saveEmbed(embedData, pStg, fSameAsLoad);
        if(ret == S_OK){
            _isDirty = false;
        }
        return ret;
    }else{
        return E_FAIL;
    }
}

STDMETHODIMP FmaAggregated::SaveCompleted(IStorage *pStg)
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:SaveCompleted";
#endif
    IPersistStorage *iface = dynamic_cast<IPersistStorage *>(controllingUnknown());
    iface->SaveCompleted(pStg);
    return S_OK;
}

STDMETHODIMP FmaAggregated::HandsOffStorage()
{
#ifdef _LOG_TEST_ON
    qWarning() << "IPersistStorage:HandsOffStorage";
#endif
    IPersistStorage *iface = dynamic_cast<IPersistStorage *>(controllingUnknown());
    iface->HandsOffStorage();
    return S_OK;
}

#endif
