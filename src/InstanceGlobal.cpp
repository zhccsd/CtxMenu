#include "InstanceGlobal.h"

#define APP_NAME  L"CtxMenu"
#define APP_DESC  L"Custom Windows Context Menu Engine"
#define APP_CLSID L"{BD482773-21BB-4816-B094-48479B1ABF02}"
#define PATH_BUFFER_SIZE 65536u

InstanceGlobal* InstanceGlobal::GetInstance()
{
    static InstanceGlobal instance;
    return &instance;
}

InstanceGlobal::InstanceGlobal()
{
}

InstanceGlobal::~InstanceGlobal()
{
}

void InstanceGlobal::InitInstance(HINSTANCE hInstance)
{
    _hInstance = hInstance;
    _getModulePath(hInstance, MAX_PATH, _dllPath);
    _loadEnvironmentVariables();
}

void InstanceGlobal::UninitInstance()
{
    if (_pathBuffer)
    {
        delete[] _pathBuffer;
        _pathBuffer = nullptr;
    }
}

HINSTANCE InstanceGlobal::GetInstanceHandle() const
{
    return _hInstance;
}

std::wstring InstanceGlobal::GetDllPath() const
{
    return _dllPath;
}

std::wstring InstanceGlobal::GetDllDirPath() const
{
    if (_dllPath.empty())
    {
        return _dllPath;
    }
    size_t pos = _dllPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        return _dllPath.substr(0, pos);
    }
    return _dllPath;
}

std::wstring InstanceGlobal::GetConfigDirPath() const
{
    return GetDllDirPath();
}

void InstanceGlobal::AddRef()
{
    _refCount.fetch_add(1, std::memory_order_relaxed);
}

void InstanceGlobal::SubRef()
{
    _refCount.fetch_sub(1, std::memory_order_relaxed);
}

int32_t InstanceGlobal::GetRefCount() const
{
    return _refCount.load(std::memory_order_relaxed);
}

std::wstring InstanceGlobal::GetAppName() const
{
    return APP_NAME;
}

std::wstring InstanceGlobal::GetAppDesc() const
{
    return APP_DESC;
}

CLSID InstanceGlobal::GetAppClass() const
{
    CLSID clsid;
    HRESULT hr = CLSIDFromString(APP_CLSID, &clsid);
    if (FAILED(hr))
    {
        return CLSID_NULL;
    }
    return clsid;
}

std::wstring InstanceGlobal::GetAppClassId() const
{
    return APP_CLSID;
}

bool InstanceGlobal::IsAppClass(REFCLSID clsid) const
{
    CLSID appClass = GetAppClass();
    return IsEqualCLSID(clsid, appClass);
}

HRESULT InstanceGlobal::RegisterServer()
{
    HKEY hKeyRoot = _getRegistryRoot();
    std::wstring contextMenuPath = _getRegistryContextMenuHandlerPath();
    std::wstring backgroundContextMenuPath = _getRegistryBackgroundContextMenuHandlerPath();
    std::wstring clsidPath = _getRegistryCLSIDPath();
    std::wstring inprocServerPath = _getRegistryInprocServerPath();
    HRESULT hr = _registerCreateKey(hKeyRoot, clsidPath, {
        { L"", GetAppDesc() },
        });
    if (FAILED(hr))
    {
        return hr;
    }
    hr = _registerCreateKey(hKeyRoot, inprocServerPath, {
        { L"", GetDllPath() },
        { L"ThreadingModel", L"Apartment" },
        });
    if (FAILED(hr))
    {
        return hr;
    }
    hr = _registerCreateKey(hKeyRoot, contextMenuPath, {
        { L"", GetAppClassId() },
        });
    if (FAILED(hr))
    {
        return hr;
    }
    hr = _registerCreateKey(hKeyRoot, backgroundContextMenuPath, {
        { L"", GetAppClassId() },
        });
    if (FAILED(hr))
    {
        return hr;
    }
    hr = _registerCreateKey(hKeyRoot, _getRegistryFolderContextMenuHandlerPath(), {
        { L"", GetAppClassId() },
        });
    return hr;
}

HRESULT InstanceGlobal::UnregisterServer()
{
    HKEY hKeyRoot = _getRegistryRoot();
    std::wstring contextMenuPath = _getRegistryContextMenuHandlerPath();
    std::wstring backgroundContextMenuPath = _getRegistryBackgroundContextMenuHandlerPath();
    std::wstring folderContextMenuPath = _getRegistryFolderContextMenuHandlerPath();
    std::wstring clsidPath = _getRegistryCLSIDPath();
    RegDeleteTreeW(hKeyRoot, contextMenuPath.c_str());
    RegDeleteTreeW(hKeyRoot, backgroundContextMenuPath.c_str());
    RegDeleteTreeW(hKeyRoot, folderContextMenuPath.c_str());
    RegDeleteTreeW(hKeyRoot, clsidPath.c_str());
    return S_OK;
}

wchar_t* InstanceGlobal::GetPathBuffer(bool zeroMem)
{
    if (_pathBuffer == nullptr)
    {
        _pathBuffer = new(std::nothrow) wchar_t[PATH_BUFFER_SIZE];
    }
    if (_pathBuffer != nullptr && zeroMem)
    {
        memset(_pathBuffer, 0, PATH_BUFFER_SIZE * sizeof(wchar_t));
    }
    return _pathBuffer;
}

size_t InstanceGlobal::GetPathBufferSize() const
{
    return _pathBuffer ? PATH_BUFFER_SIZE : 0;
}

const std::map<std::wstring, std::wstring>& InstanceGlobal::EnvironmentVariables() const
{
    return _environmentVariables;
}

std::wstring InstanceGlobal::Utf8ToUtf16(const std::string& str) const
{
    if (str.empty())
    {
        return std::wstring();
    }
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), int(str.size()), NULL, 0);
    if (sizeNeeded <= 0)
    {
        return std::wstring();
    }
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), int(str.size()), &wstrTo[0], sizeNeeded);
    return wstrTo;
}

std::string InstanceGlobal::Utf16ToUtf8(const std::wstring& str) const
{
    if (str.empty())
    {
        return std::string();
    }
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), int(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0)
    {
        return std::string();
    }
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), int(str.size()), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}

int InstanceGlobal::StringReplace(std::wstring& str, const std::wstring& oldValue, const std::wstring& newValue) const
{
    if (oldValue.empty())
    {
        0;
    }
    int replacement = 0;
    size_t pos = 0;
    while ((pos = str.find(oldValue, pos)) != std::wstring::npos)
    {
        str.replace(pos, oldValue.length(), newValue);
        pos += newValue.length();
        replacement++;
    }
    return replacement;
}

std::wstring InstanceGlobal::GetFileNameFromPath(std::wstring path) const
{
    if (path.empty())
    {
        return path;
    }
    size_t pos = path.find_last_not_of(L"\\/");
    if (pos == std::wstring::npos)
    {
        return L"";
    }
    path = path.substr(0, pos + 1);
    if (path.empty())
    {
        return path;
    }
    pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        path = path.substr(pos + 1);
    }
    pos = path.find_last_of(L'.');
    if (pos != std::wstring::npos)
    {
        path.erase(pos);
    }
    return path;
}

bool InstanceGlobal::IsFilePath(const std::wstring& path) const
{
    if (path.empty())
    {
        return false;
    }
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

bool InstanceGlobal::IsDirectoryPath(const std::wstring& path) const
{
    if (path.empty())
    {
        return false;
    }
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

bool InstanceGlobal::IsPathExists(const std::wstring& path) const
{
    if (path.empty())
    {
        return false;
    }
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES)
    {
        return true;
    }
    DWORD ret = SearchPathW(
        NULL,
        path.c_str(),
        NULL,
        0,
        NULL,
        NULL
    );
    if (ret > 0)
    {
        return true;
    }
    return false;
}

bool InstanceGlobal::CopyTextToClipboard(const std::wstring& text) const
{
    if (text.empty())
    {
        return false;
    }
    if (!OpenClipboard(NULL))
    {
        return false;
    }
    if (!EmptyClipboard())
    {
        CloseClipboard();
        return false;
    }
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hGlobal)
    {
        CloseClipboard();
        return false;
    }
    void* pGlobal = GlobalLock(hGlobal);
    if (!pGlobal)
    {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    memcpy(pGlobal, text.c_str(), bytes);
    GlobalUnlock(hGlobal);
    if (!SetClipboardData(CF_UNICODETEXT, hGlobal))
    {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
}

bool InstanceGlobal::_getModulePath(HMODULE hModule, size_t bufferSize, std::wstring& result) const
{
    if (bufferSize == 0 || bufferSize > (32 * 1024))
    {
        return false;
    }
    wchar_t* buf = new(std::nothrow) wchar_t[bufferSize];
    if (!buf)
    {
        return false;
    }
    std::unique_ptr<wchar_t[]> ptr(buf);
    SetLastError(ERROR_SUCCESS);
    GetModuleFileNameW(hModule, buf, DWORD(bufferSize));
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        return _getModulePath(hModule, bufferSize * 2, result);
    }
    result = std::wstring(buf);
    return true;
}

void InstanceGlobal::_loadEnvironmentVariables()
{
    LPWCH envStrings = GetEnvironmentStringsW();
    if (!envStrings)
    {
        return;
    }
    _environmentVariables.clear();
    for (LPWCH var = envStrings; *var; var += wcslen(var) + 1)
    {
        std::wstring entry(var);
        size_t pos = entry.find(L'=');
        if (pos != std::wstring::npos)
        {
            std::wstring key = entry.substr(0, pos);
            std::wstring value = entry.substr(pos + 1);
            _environmentVariables[key] = value;
        }
    }
    FreeEnvironmentStringsW(envStrings);
}

HKEY InstanceGlobal::_getRegistryRoot() const
{
    if (IsUserAnAdmin())
    {
        return HKEY_LOCAL_MACHINE;
    }
    else
    {
        return HKEY_CURRENT_USER;
    }
}

std::wstring InstanceGlobal::_getRegistryContextMenuHandlerPath() const
{
    wchar_t buf[MAX_PATH];
    StringCchPrintfW(buf, ARRAYSIZE(buf), L"SOFTWARE\\Classes\\*\\shellex\\ContextMenuHandlers\\%s", APP_NAME);
    return std::wstring(buf);
}

std::wstring InstanceGlobal::_getRegistryBackgroundContextMenuHandlerPath() const
{
    wchar_t buf[MAX_PATH];
    StringCchPrintfW(buf, ARRAYSIZE(buf), L"SOFTWARE\\Classes\\Directory\\background\\shellex\\ContextMenuHandlers\\%s", APP_NAME);
    return std::wstring(buf);
}

std::wstring InstanceGlobal::_getRegistryFolderContextMenuHandlerPath() const
{
    wchar_t buf[MAX_PATH];
    StringCchPrintfW(buf, ARRAYSIZE(buf), L"SOFTWARE\\Classes\\Directory\\shellex\\ContextMenuHandlers\\%s", APP_NAME);
    return std::wstring(buf);
}

std::wstring InstanceGlobal::_getRegistryCLSIDPath() const
{
    wchar_t buf[MAX_PATH];
    StringCchPrintfW(buf, ARRAYSIZE(buf), L"SOFTWARE\\Classes\\CLSID\\%s", APP_CLSID);
    return std::wstring(buf);
}

std::wstring InstanceGlobal::_getRegistryInprocServerPath() const
{
    return _getRegistryCLSIDPath() + L"\\InprocServer32";
}

HRESULT InstanceGlobal::_registerCreateKey(
    HKEY hKey,
    const std::wstring& subKey,
    const std::map<std::wstring, std::wstring>& setValues) const
{
    HKEY hSubKey;
    LONG lResult = RegCreateKeyExW(
        hKey,
        subKey.c_str(),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        nullptr,
        &hSubKey,
        nullptr);
    if (lResult != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lResult);
    }
    for (const auto& [key, value] : setValues)
    {
        lResult = RegSetValueExW(
            hSubKey,
            key.c_str(),
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(value.c_str()),
            DWORD((value.size() + 1) * sizeof(wchar_t)));
        if (lResult != ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return HRESULT_FROM_WIN32(lResult);
        }
    }
    RegCloseKey(hSubKey);
    return S_OK;
}
