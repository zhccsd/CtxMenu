#ifndef ICONMANAGER_H
#define ICONMANAGER_H
#include "InstanceGlobal.h"

class IconManager
{
public:
    IconManager();
    ~IconManager();
    HBITMAP getHBitmapFromPattern(const std::wstring& pattern);

private:
    HBITMAP _getHBitmapFromExeIcon(const std::wstring& pattern);
    bool _cacheExeIcon(const std::wstring& pattern);
    HBITMAP _loadHBitmapFromIconCache(const std::wstring& pattern);
    HBITMAP _hIconToBitmapWithAlpha(HICON hIcon, int width, int height);

private:
    struct IconCache
    {
        HICON hIcon = NULL;
        LONG width = 0;
        LONG height = 0;
    };
    std::unordered_map<std::wstring, IconCache> _iconCacheMap;
};

#endif
