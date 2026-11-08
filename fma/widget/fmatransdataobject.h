#ifndef FMATRANSDATAOBJECT_H
#define FMATRANSDATAOBJECT_H

#ifdef _WIN32

#include <ocidl.h>
#include <QMimeData>
#include <QPointer>
#include <QVector>

class FmaTransDataObject : public IDataObject
{
public:
    FmaTransDataObject(QMimeData *mimeData);
    virtual ~FmaTransDataObject();

    void resetData(QMimeData *mimeData);

    void releaseData();
    QMimeData *mimeData() const;
    DWORD reportedPerformedEffect() const;
// IUnknown
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface);
    ULONG WINAPI AddRef();
    ULONG WINAPI Release();

// IDataObject
    STDMETHOD(GetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHOD(QueryGetData)(FORMATETC* pformatetc);
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pformatetc,FORMATETC* pformatetcOut);
    STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

private:
    QPointer<QMimeData> data;
    const int CF_PERFORMEDDROPEFFECT;
    DWORD performedEffect = DROPEFFECT_NONE;
    LONG ref=1;
};

class FmaTransEnumFmtEtc : public IEnumFORMATETC
{
public:
    explicit FmaTransEnumFmtEtc(const QVector<FORMATETC> &fmtetcs);
    virtual ~FmaTransEnumFmtEtc();

// IUnknown
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface);
    ULONG WINAPI AddRef();
    ULONG WINAPI Release();

// IEnumFORMATETC methods
    STDMETHOD(Next)(ULONG celt, LPFORMATETC rgelt, ULONG FAR* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(LPENUMFORMATETC FAR* newEnum);

private:
    bool copyFormatEtc(LPFORMATETC dest, const FORMATETC *src) const;

    ULONG m_nIndex = 0;
    QVector<FORMATETC> m_fmtetcs;
    LONG m_ref=1;
};

#endif

#endif // FMATRANSDATAOBJECT_H
