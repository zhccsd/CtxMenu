#include "CtxMenuItem.h"

std::map<std::wstring, std::wstring> CtxMenuItem::_userVariables;

CtxMenuItem::CtxMenuItem(const std::map<std::string, std::wstring>& params)
    : _params(params)
{
}

CtxMenuItem::~CtxMenuItem()
{
}

CtxMenuItem CtxMenuItem::parseFromXmlElement(tinyxml2::XMLElement* element, TargetType targetType, const std::vector<std::wstring>& selections)
{
    if (!element)
    {
        return CtxMenuItem();
    }
    std::map<std::string, std::wstring> params;
    for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next())
    {
        const char* paramName = attr->Name();
        const char* paramValue = attr->Value();
        if (paramName && strlen(paramName) > 0 && paramValue && strlen(paramValue) > 0)
        {
            std::wstring value = InstanceGlobal::GetInstance()->Utf8ToUtf16(paramValue);
            _replaceVariables(value, targetType, selections);
            params[paramName] = value;
        }
    }
    return CtxMenuItem(params);
}

CtxMenuItem::ActionType CtxMenuItem::actionType() const
{
    auto iter = _params.find("type");
    if (iter == _params.end())
    {
        return Unknown;
    }
    if (iter->second == L"open")
    {
        return Open;
    }
    else if (iter->second == L"runas")
    {
        return Runas;
    }
    else if (iter->second == L"explore")
    {
        return Explore;
    }
    else if (iter->second == L"copy")
    {
        return Copy;
    }
    return Unknown;
}

std::wstring CtxMenuItem::iconPattern() const
{
    auto iter = _params.find("icon");
    if (iter != _params.end() && !iter->second.empty())
    {
        return iter->second;
    }
    return std::wstring();
}

bool CtxMenuItem::execute() const
{
    ActionType actionType = this->actionType();
    if (actionType == Unknown)
    {
        return false;
    }
    else if (actionType == Open)
    {
        auto iterFile = _params.find(ACTION_PARAM_FILE);
        if (iterFile == _params.end() || iterFile->second.empty())
        {
            return false;
        }
        auto iterParam = _params.find(ACTION_PARAM_PARAMETER);
        auto iterShow = _params.find(ACTION_PARAM_SHOW);
        int show = SW_SHOWNORMAL;
        if (iterShow != _params.end() && !iterShow->second.empty())
        {
            show = _wtoi(iterShow->second.c_str());
            if (show == 0)
            {
                show = SW_SHOWNORMAL;
            }
        }
        return _execOpen(iterFile->second, iterParam != _params.end() ? iterParam->second : L"", show);
    }
    else if (actionType == Runas)
    {
        auto iterFile = _params.find(ACTION_PARAM_FILE);
        if (iterFile == _params.end() || iterFile->second.empty())
        {
            return false;
        }
        auto iterParam = _params.find(ACTION_PARAM_PARAMETER);
        auto iterShow = _params.find(ACTION_PARAM_SHOW);
        int show = SW_SHOWNORMAL;
        if (iterShow != _params.end() && !iterShow->second.empty())
        {
            show = _wtoi(iterShow->second.c_str());
            if (show == 0)
            {
                show = SW_SHOWNORMAL;
            }
        }
        return _execRunas(iterFile->second, iterParam != _params.end() ? iterParam->second : L"", show);
    }
    else if (actionType == Explore)
    {
        auto iterFile = _params.find(ACTION_PARAM_FILE);
        if (iterFile == _params.end() || iterFile->second.empty())
        {
            return false;
        }
        return _execExplore(iterFile->second);
    }
    else if (actionType == Copy)
    {
        auto iterParam = _params.find(ACTION_PARAM_PARAMETER);
        if (iterParam == _params.end() || iterParam->second.empty())
        {
            return false;
        }
        return _execCopy(iterParam->second);
    }
    return false;
}

bool CtxMenuItem::registerUserVariable(const std::wstring& name, const std::wstring& value)
{
    if (name.empty() || value.empty())
    {
        return false;
    }
    if (name.find_first_of(L"%${}") != std::wstring::npos)
    {
        return false;
    }
    _userVariables[name] = value;
    return true;
}

bool CtxMenuItem::_execOpen(const std::wstring& file, const std::wstring& parameter, int show) const
{
    SHELLEXECUTEINFOW sei = { 0 };
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI;
    sei.hwnd = NULL;
    sei.lpVerb = L"open";
    sei.lpFile = file.c_str();
    sei.lpParameters = parameter.empty() ? NULL : parameter.c_str();
    sei.nShow = show;
    if (ShellExecuteExW(&sei))
    {
        return true;
    }
    return false;
}

bool CtxMenuItem::_execRunas(const std::wstring& file, const std::wstring& parameter, int show) const
{
    SHELLEXECUTEINFOW sei = { 0 };
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI;
    sei.hwnd = NULL;
    sei.lpVerb = L"runas";
    sei.lpFile = file.c_str();
    sei.lpParameters = parameter.empty() ? NULL : parameter.c_str();
    sei.nShow = show;
    if (ShellExecuteExW(&sei))
    {
        return true;
    }
    return false;
}

bool CtxMenuItem::_execExplore(const std::wstring& file) const
{
    auto buf = gInstance->GetPathBuffer(true);
    auto bufSize = gInstance->GetPathBufferSize();
    StringCchPrintfW(buf, bufSize, L"/select,\"%s\"", file.c_str());
    return _execOpen(L"explorer.exe", buf, SW_SHOWNORMAL);
}

bool CtxMenuItem::_execCopy(const std::wstring& parameter) const
{
    return gInstance->CopyTextToClipboard(parameter);
}

void CtxMenuItem::_replaceVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections)
{
    _replaceUserVariables(str);
    _replaceEnvironmentVariables(str);
    _replaceCtxMenuVariables(str, targetType, selections);
}

void CtxMenuItem::_replaceUserVariables(std::wstring& str)
{
#if ENABLE_NESTED_USER_VARIABLES
    std::set<std::wstring> usedVars;
    bool replaced;
    do {
        replaced = false;
        for (const auto& [key, value] : _userVariables)
        {
            std::wstring varPattern = L"$${" + key + L"}";
            if (str.find(varPattern) != std::wstring::npos)
            {
                if (usedVars.count(varPattern))
                {
                    continue;
                }
            }
            if (gInstance->StringReplace(str, varPattern, value) > 0)
            {
                replaced = true;
                usedVars.insert(varPattern);
            }
        }
    } while (replaced);
#else
    for (const auto& [key, value] : _userVariables)
    {
        gInstance->StringReplace(str, L"$${" + key + L"}", value);
    }
#endif
}

void CtxMenuItem::_replaceEnvironmentVariables(std::wstring& str)
{
    auto envMap = gInstance->EnvironmentVariables();
    for (const auto& [key, value] : envMap)
    {
        gInstance->StringReplace(str, L"%" + key + L"%", value);
    }
}

void CtxMenuItem::_replaceCtxMenuVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections)
{
    gInstance->StringReplace(str, L"${CTXMENU_HOME_PATH}", gInstance->GetDllDirPath());
    gInstance->StringReplace(str, L"${CTXMENU_CONFIG_PATH}", gInstance->GetConfigDirPath());
    if (targetType == TargetType::DirectoryBackground || targetType == TargetType::SingleDirectory)
    {
        if (!selections.empty())
        {
            gInstance->StringReplace(str, L"${CTXMENU_PWD}", selections[0]);
        }
    }
    gInstance->StringReplace(str, L"${CTXMENU_DLL_PATH}", gInstance->GetDllPath());
    for (int i = 0; i < selections.size(); ++i)
    {
        gInstance->StringReplace(str, L"${CTXMENU_FILE_" + std::to_wstring(i) + L"}", selections[i]);
    }
}
