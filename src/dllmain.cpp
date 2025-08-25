#include "InstanceGlobal.h"
#include "ClassFactory.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        gInstance->InitInstance(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        gInstance->UninitInstance();
        break;
    }
    return TRUE;
}

extern "C" STDAPI DllCanUnloadNow(void)
{
    if (gInstance->GetRefCount() == 0)
    {
        return S_OK;
    }
    return S_FALSE;
}

extern "C" STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppv = nullptr;
    if (gInstance->IsAppClass(rclsid))
    {
        ClassFactory* pClassFactory = new(std::nothrow) ClassFactory;
        if (pClassFactory == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        HRESULT hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

extern "C" STDAPI DllRegisterServer(void)
{
    return gInstance->RegisterServer();
}

extern "C" STDAPI DllUnregisterServer(void)
{
    return gInstance->UnregisterServer();
}
