#ifndef CTXMENU_H
#define CTXMENU_H
#include "CtxMenuItem.h"
#include "IconManager.h"

class CtxMenu
{
public:
    CtxMenu();
    ~CtxMenu();

    bool initialize(TargetType targetType, const std::vector<std::wstring>& selections);
    UINT buildContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast);
    bool invokeIdCmd(WORD idCmd);
    std::wstring getHelpTextW(WORD idCmd, UINT cchMax) const;
    std::wstring getVerbW(WORD idCmd, UINT cchMax) const;

private:
    std::list<std::pair<std::wstring, std::wstring>> _getAllConfigPaths() const;
    HMENU _buildMenuFromXml(const std::wstring& xmlPath, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd);
    HMENU _buildCtxMenu(UINT idCmdFirst, UINT idCmdLast, WORD& curCmd);
    HMENU _buildMenuFromElement(tinyxml2::XMLElement* menuElement, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd);

private:
    UINT _parseMenu(HMENU hRootMenu, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd, tinyxml2::XMLElement* menuElement);
    void _insertSeparator(HMENU hMenu, UINT indexMenu);
    bool _insertMenu(HMENU hMenu, UINT indexMenu, const std::wstring& text, const std::wstring& iconPattern, HMENU* subMenu);
    bool _insertAction(HMENU hMenu, UINT indexMenu, const std::wstring& text, UINT idCmdFirst, WORD idCmd, const CtxMenuItem& actionItem);

private:
    TargetType _targetType = TargetType::Unknown;
    std::vector<std::wstring> _selections;
    std::map<WORD, CtxMenuItem> _actions;
    std::unique_ptr<IconManager> _iconManager;
};

#endif
