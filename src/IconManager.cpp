#include "IconManager.h"

IconManager::IconManager()
{
}

IconManager::~IconManager()
{
    for (const auto& [exePath, iconCache] : _iconCacheMap)
    {
        if (iconCache.hIcon)
        {
            DestroyIcon(iconCache.hIcon);
        }
    }
    _iconCacheMap.clear();
}

HBITMAP IconManager::getHBitmapFromPattern(const std::wstring& pattern)
{
    // exeIcon:exeFilePath
    const std::wstring exeIconPrefix = L"exeIcon:";
    if (pattern.find(exeIconPrefix, 0) == 0)
    {
        std::wstring exePath = pattern.substr(exeIconPrefix.size());
        return _getHBitmapFromExeIcon(exePath);
    }
    return NULL;
}

HBITMAP IconManager::_getHBitmapFromExeIcon(const std::wstring& exePath)
{
    if (exePath.empty())
    {
        return NULL;
    }
    if (!_cacheExeIcon(exePath))
    {
        return NULL;
    }
    return _loadHBitmapFromCache(exePath);
}

bool IconManager::_cacheExeIcon(const std::wstring& exePath)
{
    auto iter = _iconCacheMap.find(exePath);
    if (iter != _iconCacheMap.end())
    {
        return true;
    }
    UINT iconCount = ExtractIconExW(exePath.c_str(), -1, NULL, NULL, 0);
    if (iconCount == 0)
    {
        return false;
    }
    HICON hSmallIcon = NULL;
    ExtractIconExW(exePath.c_str(), 0, NULL, &hSmallIcon, 1);
    if (!hSmallIcon)
    {
        return false;
    }
    ICONINFO iconInfo = { 0 };
    if (!GetIconInfo(hSmallIcon, &iconInfo))
    {
        DestroyIcon(hSmallIcon);
        _iconCacheMap[exePath] = { NULL, 0, 0 };
        return false;
    }
    if (!iconInfo.hbmColor)
    {
        DestroyIcon(hSmallIcon);
        _iconCacheMap[exePath] = { NULL, 0, 0 };
        return false;
    }
    BITMAP bm;
    GetObject(iconInfo.hbmColor, sizeof(bm), &bm);
    _iconCacheMap[exePath] = { hSmallIcon, bm.bmWidth, bm.bmHeight };
    return true;
}

HBITMAP IconManager::_loadHBitmapFromCache(const std::wstring& exePath)
{
    auto iter = _iconCacheMap.find(exePath);
    if (iter == _iconCacheMap.end())
    {
        return NULL;
    }
    const IconCache& iconCache = iter->second;
    if (!iconCache.hIcon || iconCache.width <= 0 || iconCache.height <= 0)
    {
        return NULL;
    }
    return _hIconToBitmapWithAlpha(iconCache.hIcon, iconCache.width, iconCache.height);
}

HBITMAP IconManager::_hIconToBitmapWithAlpha(HICON hIcon, int width, int height)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HDC hdc = GetDC(NULL);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (hBitmap && pvBits)
    {
        HDC hMemDC = CreateCompatibleDC(hdc);
        HGDIOBJ oldBmp = SelectObject(hMemDC, hBitmap);
        DrawIconEx(hMemDC, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);
        SelectObject(hMemDC, oldBmp);
        DeleteDC(hMemDC);
    }
    ReleaseDC(NULL, hdc);
    return hBitmap;
}
