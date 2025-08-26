#include "CtxMenuAction.h"

CtxMenuAction::CtxMenuAction(const std::map<std::string, std::wstring>& params)
    : _params(params)
{
}

CtxMenuAction::~CtxMenuAction()
{
}

CtxMenuAction CtxMenuAction::parseFromXmlElement(tinyxml2::XMLElement* element, TargetType targetType, const std::vector<std::wstring>& selections)
{
    if (!element)
    {
        return CtxMenuAction();
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
    return CtxMenuAction(params);
}

CtxMenuAction::ActionType CtxMenuAction::actionType() const
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
    return Unknown;
}

std::wstring CtxMenuAction::iconPattern() const
{
    auto iter = _params.find("icon");
    if (iter != _params.end() && !iter->second.empty())
    {
        return iter->second;
    }
    return std::wstring();
}

bool CtxMenuAction::execute() const
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
    return false;
}

bool CtxMenuAction::_execOpen(const std::wstring& file, const std::wstring& parameter, int show) const
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

bool CtxMenuAction::_execRunas(const std::wstring& file, const std::wstring& parameter, int show) const
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

bool CtxMenuAction::_execExplore(const std::wstring& file) const
{
    auto buf = gInstance->GetPathBuffer(true);
    auto bufSize = gInstance->GetPathBufferSize();
    StringCchPrintfW(buf, bufSize, L"/select,\"%s\"", file.c_str());
    return _execOpen(L"explorer.exe", buf, SW_SHOWNORMAL);
}

void CtxMenuAction::_replaceVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections)
{
    _replaceEnvironmentVariables(str);
    _replaceCtxMenuVariables(str, targetType, selections);
}

void CtxMenuAction::_replaceEnvironmentVariables(std::wstring& str)
{
    auto envMap = gInstance->EnvironmentVariables();
    for (const auto& [key, value] : envMap)
    {
        gInstance->StringReplace(str, L"%" + key + L"%", value);
    }
}

void CtxMenuAction::_replaceCtxMenuVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections)
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
}
