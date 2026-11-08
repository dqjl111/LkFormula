#ifndef FMAAGGREGATED_H
#define FMAAGGREGATED_H

#ifdef _WIN32

#include <QAxAggregated>
#include "fmamain.h"
#include <ocidl.h>

class FmaBinder;
class FmaAggregated : public QAxAggregated
        , public IOleObject
        , public IDataObject
        , public IPersistStorage
{
public:
    FmaAggregated();
    ~FmaAggregated();

    virtual long queryInterface(const QUuid &iid, void **iface) override;
// IUnknown
    QAXAGG_IUNKNOWN;

// IOleObject
    STDMETHOD(Advise)(IAdviseSink* pAdvSink, DWORD* pdwConnection);
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA** ppenumAdvise);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB** ppEnumOleVerb);
    STDMETHOD(GetClientSite)(IOleClientSite** ppClientSite);
    STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject** ppDataObject);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL* psizel);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHOD(GetUserClassID)(CLSID* pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(InitFromData)(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved);
    STDMETHOD(IsUpToDate)();
    STDMETHOD(SetClientSite)(IOleClientSite* pClientSite);
    STDMETHOD(SetColorScheme)(LOGPALETTE* pLogPal);
    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL* psizel);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker* ppmk);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(Update)();

// IDataObject
    STDMETHOD(GetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHOD(QueryGetData)(FORMATETC* pformatetc);
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pformatect,FORMATETC* pformatetcOut);
    STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid);

// IPersistStorage
    STDMETHOD(IsDirty)();
    STDMETHOD(InitNew)(IStorage *pStg);
    STDMETHOD(Load)(IStorage *pStg);
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage *pStgNew);
    STDMETHOD(HandsOffStorage)();
protected:
    FmaMain *window() const;
    FmaBinder *binder() const;

    IOleClientSite *getClientSite();
    IOleClientSite *_spClientSite = NULL;
    IDataAdviseHolder *  _pIDataAdviseHolder = NULL;
    IOleAdviseHolder *  _pIOleAdviseHolder = NULL;
    bool _isDirty = false;
    int _cf_fmtCbor;
};

#endif
#endif // FMAAGGREGATED_H
