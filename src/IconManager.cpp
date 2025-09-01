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
    // exeIcon:exeFilePath@index
    const std::wstring exeIconPrefix = L"exeIcon:";
    if (pattern.find(exeIconPrefix, 0) == 0)
    {
        std::wstring exePattern = pattern.substr(exeIconPrefix.size());
        return _getHBitmapFromExeIcon(exePattern);
    }
    return NULL;
}

HBITMAP IconManager::_getHBitmapFromExeIcon(const std::wstring& pattern)
{
    if (pattern.empty())
    {
        return NULL;
    }
    if (!_cacheExeIcon(pattern))
    {
        return NULL;
    }
    return _loadHBitmapFromIconCache(pattern);
}

bool IconManager::_cacheExeIcon(const std::wstring& pattern)
{
    auto iter = _iconCacheMap.find(pattern);
    if (iter != _iconCacheMap.end())
    {
        return true;
    }
    std::wstring exePath = pattern;
    int iconIndex = 0;
    auto atPos = pattern.find_last_of(L'@');
    if (atPos != std::wstring::npos)
    {
        std::wstring indexStr = pattern.substr(atPos + 1);
        exePath = pattern.substr(0, atPos);
        try
        {
            iconIndex = std::stoi(indexStr.c_str());
        }
        catch (...)
        {
            iconIndex = 0;
        }
    }
    UINT iconCount = ExtractIconExW(exePath.c_str(), -1, NULL, NULL, 0);
    if (iconCount == 0)
    {
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    if (iconCount > 99999)
    {
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    if (iconIndex >= static_cast<int>(iconCount))
    {
        iconIndex = 0;
    }
    if (iconIndex < 0)
    {
        /*
        * from MS Learn:
        * If this value is a negative number and either phiconLarge or phiconSmall is not NULL,
        * the function begins by extracting the icon whose resource identifier is equal to the absolute value of nIconIndex.
        * For example, use -3 to extract the icon whose resource identifier is 3.
        * 
        * so negative index is ok, and makes it even easier to find icons in shell32.dll and imageres.dll
        */
        //iconIndex = 0;
    }
    HICON hSmallIcon = NULL;
    ExtractIconExW(exePath.c_str(), iconIndex, NULL, &hSmallIcon, 1);
    if (!hSmallIcon)
    {
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    ICONINFO iconInfo = { 0 };
    if (!GetIconInfo(hSmallIcon, &iconInfo))
    {
        DestroyIcon(hSmallIcon);
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    if (iconInfo.hbmMask)
    {
        DeleteObject(iconInfo.hbmMask);
    }
    if (!iconInfo.hbmColor)
    {
        DestroyIcon(hSmallIcon);
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    BITMAP bm;
    int ret = GetObjectW(iconInfo.hbmColor, sizeof(bm), &bm);
    if (iconInfo.hbmColor)
    {
        DeleteObject(iconInfo.hbmColor);
    }
    if (ret == 0)
    {
        DestroyIcon(hSmallIcon);
        _iconCacheMap[pattern] = { NULL, 0, 0 };
        return false;
    }
    _iconCacheMap[pattern] = { hSmallIcon, bm.bmWidth, bm.bmHeight };
    return true;
}

HBITMAP IconManager::_loadHBitmapFromIconCache(const std::wstring& pattern)
{
    auto iter = _iconCacheMap.find(pattern);
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
