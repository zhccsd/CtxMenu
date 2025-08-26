#ifndef CTXMENUACTION_H
#define CTXMENUACTION_H
#include "InstanceGlobal.h"
#include <tinyxml2/11.0.0/tinyxml2.h>

#define ACTION_PARAM_FILE      "file"
#define ACTION_PARAM_PARAMETER "parameter"
#define ACTION_PARAM_SHOW      "show"

class CtxMenuAction
{
public:
    enum ActionType
    {
        Unknown,
        Open, // require: [ACTION_PARAM_FILE], optional: [ACTION_PARAM_PARAMETER, ACTION_PARAM_SHOW]
        Runas, // require: [ACTION_PARAM_FILE], optional: [ACTION_PARAM_PARAMETER, ACTION_PARAM_SHOW]
        Explore, // require: [ACTION_PARAM_FILE]
    };

    CtxMenuAction(const std::map<std::string, std::wstring>& params = std::map<std::string, std::wstring>());
    ~CtxMenuAction();
    static CtxMenuAction parseFromXmlElement(tinyxml2::XMLElement* element, TargetType targetType, const std::vector<std::wstring>& selections);
    ActionType actionType() const;
    std::wstring iconPattern() const;
    bool execute() const;

private:
    bool _execOpen(const std::wstring& file, const std::wstring& parameter = L"", int show = SW_SHOWNORMAL) const;
    bool _execRunas(const std::wstring& file, const std::wstring& parameter = L"", int show = SW_SHOWNORMAL) const;
    bool _execExplore(const std::wstring& file) const;

private:
    static void _replaceVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections);
    static void _replaceEnvironmentVariables(std::wstring& str);
    static void _replaceCtxMenuVariables(std::wstring& str, TargetType targetType, const std::vector<std::wstring>& selections);

private:
    std::map<std::string, std::wstring> _params;
};

#endif
