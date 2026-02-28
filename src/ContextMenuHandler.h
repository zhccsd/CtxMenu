#ifndef CONTEXTMENUHANDLER_H
#define CONTEXTMENUHANDLER_H
#include "InstanceGlobal.h"
#include "CtxMenu.h"

class ContextMenuHandler : public IContextMenu, public IShellExtInit
{
public:
    ContextMenuHandler();

private:
    ~ContextMenuHandler();

public:
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IShellExtInit methods
    STDMETHODIMP Initialize(
        PCIDLIST_ABSOLUTE pidlFolder,
        IDataObject* pdtobj,
        HKEY hkeyProgID) override;

    // IContextMenu methods
    STDMETHODIMP QueryContextMenu(
        HMENU hMenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags) override;
    STDMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* pici) override;
    STDMETHODIMP GetCommandString(
        UINT_PTR idCmd,
        UINT uType,
        UINT* pReserved,
        LPSTR pszName,
        UINT cchMax) override;

private:
    TargetType _getTypeFromSelectedFiles(const std::vector<std::wstring>& selected);

private:
    ULONG _ref;
    std::unique_ptr<CtxMenu> _ctxMenu;
};

#endif
