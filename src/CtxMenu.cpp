#include "CtxMenu.h"

#define INSERT_MENU_INDEX(HMENU) GetMenuItemCount(HMENU)

CtxMenu::CtxMenu()
{
    _iconManager = std::make_unique<IconManager>();
}

CtxMenu::~CtxMenu()
{
}

bool CtxMenu::initialize(TargetType targetType, const std::vector<std::wstring>& selections)
{
    _targetType = targetType;
    _selections = selections;
    return true;
}

UINT CtxMenu::buildContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast)
{
    ElapsedTimer timer;
    struct MenuInfo
    {
        HMENU hMenu;
        std::wstring menuName;
        std::wstring iconPattern;
    };
    std::list<MenuInfo> menuList;
    std::list<std::pair<std::wstring, std::wstring>> configPaths = _getAllConfigPaths();
    WORD curCmd = 0;
    for (const auto& [configPath, configName] : configPaths)
    {
        std::wstring iconPattern;
        HMENU hXmlMenu = _buildMenuFromXml(configPath, idCmdFirst, idCmdLast, curCmd, iconPattern);
        if (hXmlMenu)
        {
            menuList.push_back({ hXmlMenu, configName, iconPattern });
        }
    }
    HMENU hCtxMenu = _buildCtxMenu(idCmdFirst, idCmdLast, curCmd);
    if (hCtxMenu)
    {
        StringCchPrintfW(gInstance->GetPathBuffer(), gInstance->GetPathBufferSize(), L"exeIcon:%s@-101", gInstance->GetDllPath().c_str());
        menuList.push_back({ hCtxMenu, L"CtxMenu", std::wstring(gInstance->GetPathBuffer()) });
    }
    if (menuList.empty())
    {
        return 0;
    }
    _insertSeparator(hMenu, indexMenu++);
    for (const auto& menu: menuList)
    {
        _insertMenu(hMenu, indexMenu++, menu.menuName, menu.iconPattern, const_cast<HMENU*>(&menu.hMenu));
    }
    _insertSeparator(hMenu, indexMenu++);
    Logger::log("{} finished, used {:.4f} ms", __FUNCTION__, timer.elapsed());
    return curCmd;
}

bool CtxMenu::invokeIdCmd(WORD idCmd)
{
    auto iter = _actions.find(idCmd);
    if (iter != _actions.end())
    {
        return iter->second.execute();
    }
    return false;
}

std::wstring CtxMenu::getHelpTextW(WORD idCmd, UINT cchMax) const
{
    UNREFERENCED_PARAMETER(idCmd);
    UNREFERENCED_PARAMETER(cchMax);
    return std::wstring();
}

std::wstring CtxMenu::getVerbW(WORD idCmd, UINT cchMax) const
{
    UNREFERENCED_PARAMETER(idCmd);
    UNREFERENCED_PARAMETER(cchMax);
    return std::wstring();
}

std::list<std::pair<std::wstring, std::wstring>> CtxMenu::_getAllConfigPaths() const
{
    std::list<std::pair<std::wstring, std::wstring>> result;
    std::wstring searchFolder = gInstance->GetConfigDirPath();
    std::wstring searchPath = searchFolder + L"\\*.xml";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::wstring fileBaseName = findData.cFileName;
                size_t pos = fileBaseName.find_last_of(L'.');
                if (pos != std::wstring::npos)
                {
                    fileBaseName = fileBaseName.substr(0, pos);
                }
                result.push_back(std::make_pair(searchFolder + L"\\" + findData.cFileName, fileBaseName));
                if (result.size() >= WINDOWS_CONTEXT_MENU_ENTRIES_MAX)
                {
                    break;
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
    return result;
}

HMENU CtxMenu::_buildMenuFromXml(const std::wstring& xmlPath, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd, std::wstring& iconPattern)
{
    tinyxml2::XMLDocument doc;
    FILE* file = nullptr;
    _wfopen_s(&file, xmlPath.c_str(), L"rb");
    if (!file)
    {
        return nullptr;
    }
    auto err = doc.LoadFile(file);
    fclose(file);
    if (err != tinyxml2::XML_SUCCESS)
    {
        return nullptr;
    }
    tinyxml2::XMLElement* rootElement = doc.RootElement();
    if (!rootElement)
    {
        return nullptr;
    }
    CtxMenuItem rootItem = CtxMenuItem::parseFromXmlElement(rootElement, _targetType, _selections);
    iconPattern = rootItem.iconPattern();
    tinyxml2::XMLElement* varElement = rootElement->FirstChildElement("var");
    while (varElement)
    {
        const char* nameAttr = varElement->Attribute("name");
        std::wstring name = nameAttr ? gInstance->Utf8ToUtf16(nameAttr) : L"";
        const char* valueAttr = varElement->Attribute("value");
        std::wstring value = valueAttr ? gInstance->Utf8ToUtf16(valueAttr) : L"";
        CtxMenuItem::registerUserVariable(name, value);
        varElement = varElement->NextSiblingElement("var");
    }
    tinyxml2::XMLElement* menuElement = nullptr;
    switch (_targetType)
    {
    case TargetType::DirectoryBackground:
        menuElement = rootElement->FirstChildElement("background");
        break;
    default:
        break;
    }
    if (!menuElement)
    {
        return nullptr;
    }
    return _buildMenuFromElement(menuElement, idCmdFirst, idCmdLast, curCmd);
}

HMENU CtxMenu::_buildCtxMenu(UINT idCmdFirst, UINT idCmdLast, WORD& curCmd)
{
    if (_targetType != TargetType::DirectoryBackground)
    {
        return nullptr;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* menuElement = doc.NewElement("ctxMenu");

    auto action = menuElement->InsertNewChildElement("action");
    action->SetAttribute("text", "Restart Explorer");
    action->SetAttribute("type", "open");
    action->SetAttribute("file", "cmd.exe");
    action->SetAttribute("parameter", "/c taskkill /f /im explorer.exe && start explorer");

    action = menuElement->InsertNewChildElement("action");
    action->SetAttribute("text", "CtxMenu Home");
    action->SetAttribute("type", "open");
    action->SetAttribute("file", "${CTXMENU_HOME_PATH}");

    action = menuElement->InsertNewChildElement("action");
    action->SetAttribute("text", "CtxMenu Config");
    action->SetAttribute("type", "open");
    action->SetAttribute("file", "${CTXMENU_CONFIG_PATH}");

    action = menuElement->InsertNewChildElement("action");
    action->SetAttribute("text", "About CtxMenu");
    action->SetAttribute("type", "open");
    action->SetAttribute("file", "https://github.com/zhccsd/CtxMenu");

    return _buildMenuFromElement(menuElement, idCmdFirst, idCmdLast, curCmd);
}

HMENU CtxMenu::_buildMenuFromElement(tinyxml2::XMLElement* menuElement, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd)
{
    HMENU hMenu = CreatePopupMenu();
    if (hMenu == nullptr)
    {
        return nullptr;
    }
    UINT itemCount = _parseMenu(hMenu, idCmdFirst, idCmdLast, curCmd, menuElement);
    if (itemCount == 0)
    {
        DestroyMenu(hMenu);
        return nullptr;
    }
    return hMenu;
}

UINT CtxMenu::_parseMenu(HMENU hRootMenu, UINT idCmdFirst, UINT idCmdLast, WORD& curCmd, tinyxml2::XMLElement* menuElement)
{
    UINT ret = 0;
    if (!menuElement)
    {
        return ret;
    }
    if (curCmd >= (idCmdLast - idCmdFirst + 1))
    {
        return ret;
    }
    for (auto element = menuElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
    {
        std::string name = element->Name();
        if (name == "separator")
        {
            _insertSeparator(hRootMenu, INSERT_MENU_INDEX(hRootMenu));
            ++ret;
        }
        else if (name == "menu")
        {
            CtxMenuItem menuItem = CtxMenuItem::parseFromXmlElement(element, _targetType, _selections);
            const char* textAttr = element->Attribute("text");
            std::wstring text = textAttr ? gInstance->Utf8ToUtf16(textAttr) : L"";
            HMENU hSubMenu = nullptr;
            if (_insertMenu(hRootMenu, INSERT_MENU_INDEX(hRootMenu), text, menuItem.iconPattern(), &hSubMenu))
            {
                ret += _parseMenu(hSubMenu, idCmdFirst, idCmdLast, curCmd, element);
            }
        }
        else if (name == "action")
        {
            CtxMenuItem actionItem = CtxMenuItem::parseFromXmlElement(element, _targetType, _selections);
            if (actionItem.actionType() == CtxMenuItem::ActionType::Unknown)
            {
                continue;
            }
            const char* textAttr = element->Attribute("text");
            std::wstring text = textAttr ? gInstance->Utf8ToUtf16(textAttr) : L"";
            if (text.empty())
            {
                continue;
            }
            if (!_insertAction(hRootMenu, INSERT_MENU_INDEX(hRootMenu), text, idCmdFirst, curCmd, actionItem))
            {
            }
            else
            {
                curCmd++;
                ret++;
            }
        }
    }
    return ret;
}

void CtxMenu::_insertSeparator(HMENU hMenu, UINT indexMenu)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItemW(hMenu, indexMenu, TRUE, &mii);
}

bool CtxMenu::_insertMenu(HMENU hMenu, UINT indexMenu, const std::wstring& text, const std::wstring& iconPattern, HMENU* subMenu)
{
    if (!(*subMenu))
    {
        *subMenu = CreatePopupMenu();
        if (!(*subMenu))
        {
            return false;
        }
    }
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;
    mii.dwTypeData = const_cast<wchar_t*>(text.c_str());
    mii.cch = static_cast<UINT>(text.size());
    mii.hSubMenu = *subMenu;
    mii.hbmpItem = _iconManager->getHBitmapFromPattern(iconPattern);
    if (mii.hbmpItem)
    {
        mii.fMask |= MIIM_BITMAP;
    }
    if (!InsertMenuItemW(hMenu, indexMenu, TRUE, &mii))
    {
        DestroyMenu(*subMenu);
        return false;
    }
    return true;
}

bool CtxMenu::_insertAction(HMENU hMenu, UINT indexMenu, const std::wstring& text, UINT idCmdFirst, WORD idCmd, const CtxMenuItem& actionItem)
{
    MENUITEMINFOW mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
    mii.wID = idCmdFirst + idCmd;
    mii.fState = MFS_ENABLED;
    StringCchPrintfW(gInstance->GetPathBuffer(), gInstance->GetPathBufferSize(), text.c_str());
    mii.dwTypeData = gInstance->GetPathBuffer();
    mii.hbmpItem = _iconManager->getHBitmapFromPattern(actionItem.iconPattern());
    if (mii.hbmpItem)
    {
        mii.fMask |= MIIM_BITMAP;
    }
    if (!InsertMenuItemW(hMenu, indexMenu, TRUE, &mii))
    {
        return false;
    }
    _actions.emplace(idCmd, actionItem);
    return true;
}
