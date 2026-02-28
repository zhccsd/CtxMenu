#ifndef GLOBALDEF_H
#define GLOBALDEF_H

// Max context menu entries at windows explorer context menu
// Not including CtxMenu and two separators
#define WINDOWS_CONTEXT_MENU_ENTRIES_MAX 10

#define ENABLE_NESTED_USER_VARIABLES true
#define DISABLE_IF_NOT_EXECUTABLE true

enum class TargetType
{
    Unknown,
    DirectoryBackground,
    SingleFile,
    SingleDirectory,
    MultiFile,
    MultiDirectory,
    MultiMixed,
};

#endif
