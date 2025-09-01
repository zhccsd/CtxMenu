#include "ContextMenuHandler.h"
#include <shellapi.h>
#include <shlwapi.h>

ContextMenuHandler::ContextMenuHandler()
    : _ref(1)
{
    try
    {
        _ctxMenu = std::make_unique<CtxMenu>();
    }
    catch (const std::bad_alloc&)
    {
        _ctxMenu.reset();
    }
    gInstance->AddRef();
}

ContextMenuHandler::~ContextMenuHandler()
{
    gInstance->SubRef();
}

STDMETHODIMP ContextMenuHandler::QueryInterface(REFIID riid, void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(ContextMenuHandler, IContextMenu),
        QITABENT(ContextMenuHandler, IShellExtInit),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObject);
}

STDMETHODIMP_(ULONG) ContextMenuHandler::AddRef()
{
    return InterlockedIncrement(&_ref);
}

STDMETHODIMP_(ULONG) ContextMenuHandler::Release()
{
    ULONG cRef = InterlockedDecrement(&_ref);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP ContextMenuHandler::Initialize(
    PCIDLIST_ABSOLUTE pidlFolder,
    IDataObject* pdtobj,
    HKEY hkeyProgID)
{
    UNREFERENCED_PARAMETER(hkeyProgID);
    if (!_ctxMenu)
    {
        return E_OUTOFMEMORY;
    }
    wchar_t* pathBuffer = gInstance->GetPathBuffer(true);
    size_t pathBufferSize = gInstance->GetPathBufferSize();
    if (pidlFolder != nullptr)
    {
        SHGetPathFromIDListW(pidlFolder, pathBuffer);
        _ctxMenu->initialize(TargetType::DirectoryBackground, { std::wstring(pathBuffer) });
        return S_OK;
    }
    if (pdtobj != nullptr)
    {
        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stg = { TYMED_HGLOBAL };
        HRESULT hr = pdtobj->GetData(&fmt, &stg);
        std::vector<std::wstring> selectedFiles;
        if (SUCCEEDED(hr))
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
            if (hDrop != nullptr)
            {
                UINT uNumFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
                for (UINT i = 0; i < uNumFiles; i++)
                {
                    if (DragQueryFileW(hDrop, i, pathBuffer, (DWORD)pathBufferSize))
                    {
                        selectedFiles.emplace_back(pathBuffer);
                    }
                }
                GlobalUnlock(stg.hGlobal);
            }
            ReleaseStgMedium(&stg);
            if (selectedFiles.size() != 1)
            {
                return E_FAIL;
            }
            _ctxMenu->initialize(TargetType::SingleFile, selectedFiles);
        }
        return hr;
    }
    return E_FAIL;
}

STDMETHODIMP ContextMenuHandler::QueryContextMenu(
    HMENU hMenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags)
{
    UNREFERENCED_PARAMETER(idCmdLast);
    if (uFlags & CMF_DEFAULTONLY)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
    }
    UINT ret = 0;
    if (_ctxMenu)
    {
        ret = _ctxMenu->buildContextMenu(hMenu, indexMenu, idCmdFirst, idCmdLast);
    }
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, ret);
}

STDMETHODIMP ContextMenuHandler::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
    if (pici == nullptr)
    {
        return E_INVALIDARG;
    }
    if (HIWORD(pici->lpVerb) != 0)
    {
        return E_FAIL;
    }
    WORD idCmd = LOWORD(pici->lpVerb);
    bool ret = false;
    if (_ctxMenu)
    {
        ret = _ctxMenu->invokeIdCmd(idCmd);
    }
    if (ret)
    {
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP ContextMenuHandler::GetCommandString(
    UINT_PTR idCmd,
    UINT uType,
    UINT* pReserved,
    LPSTR pszName,
    UINT cchMax)
{
    UNREFERENCED_PARAMETER(pReserved);
    if (pszName == nullptr || cchMax == 0)
    {
        return E_INVALIDARG;
    }
    if (!_ctxMenu)
    {
        return E_FAIL;
    }
    if (uType == GCS_HELPTEXTW)
    {
        std::wstring wstr = _ctxMenu->getHelpTextW(LOWORD(idCmd), cchMax);
        if (!wstr.empty())
        {
            StringCchCopyW((LPWSTR)pszName, cchMax, wstr.c_str());
        }
        return S_OK;
    }
    else if (uType == GCS_VERBW)
    {
        std::wstring wstr = _ctxMenu->getVerbW(LOWORD(idCmd), cchMax);
        if (!wstr.empty())
        {
            StringCchCopyW((LPWSTR)pszName, cchMax, wstr.c_str());
        }
        return S_OK;
    }
    return E_FAIL;
}
