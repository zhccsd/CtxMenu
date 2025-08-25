#include "ClassFactory.h"
#include "ContextMenuHandler.h"

ClassFactory::ClassFactory()
    : _ref(1)
{
    gInstance->AddRef();
}

ClassFactory::~ClassFactory()
{
    gInstance->SubRef();
}

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(ClassFactory, IClassFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObject);
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&_ref);
}

STDMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&_ref);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppvObject = nullptr;
    if (pUnkOuter != nullptr)
    {
        return CLASS_E_NOAGGREGATION;
    }
    ContextMenuHandler* pHandler = new(std::nothrow) ContextMenuHandler;
    if (pHandler == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    HRESULT hr = pHandler->QueryInterface(riid, ppvObject);
    pHandler->Release();
    return hr;
}

STDMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        gInstance->AddRef();
    }
    else
    {
        gInstance->SubRef();
    }
    return S_OK;
}
