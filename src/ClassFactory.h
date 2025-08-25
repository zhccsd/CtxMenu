#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H
#include "InstanceGlobal.h"

class ClassFactory : public IClassFactory
{
public:
    ClassFactory();

private:
    ~ClassFactory();

public:
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IClassFactory methods
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    STDMETHODIMP LockServer(BOOL fLock) override;

private:
    ULONG _ref;
};

#endif
