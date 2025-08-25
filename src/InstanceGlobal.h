#ifndef INSTANCEGLOBAL_H
#define INSTANCEGLOBAL_H
#include <Windows.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <comdef.h>
#include <strsafe.h>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <map>
#include <functional>
#include "GlobalDef.h"
#include "Logger.h"

class InstanceGlobal
{
public:
    static InstanceGlobal* GetInstance();

private:
    InstanceGlobal();

public:
    ~InstanceGlobal();

    void InitInstance(HINSTANCE hInstance);
    void UninitInstance();
    HINSTANCE GetInstanceHandle() const;
    std::wstring GetDllPath() const;
    std::wstring GetDllDirPath() const;
    std::wstring GetConfigDirPath() const;
    void AddRef();
    void SubRef();
    int32_t GetRefCount() const;
    std::wstring GetAppName() const;
    std::wstring GetAppDesc() const;
    CLSID GetAppClass() const;
    std::wstring GetAppClassId() const;
    bool IsAppClass(REFCLSID clsid) const;
    HRESULT RegisterServer();
    HRESULT UnregisterServer();
    wchar_t* GetPathBuffer(bool zeroMem = false);
    size_t GetPathBufferSize() const;
    const std::map<std::wstring, std::wstring>& EnvironmentVariables() const;

    // utility functions
    std::wstring Utf8ToUtf16(const std::string& str) const;
    void StringReplace(std::wstring& str, const std::wstring& oldValue, const std::wstring& newValue) const;

private:
    bool _getModulePath(HMODULE hModule, size_t bufferSize, std::wstring& result) const;
    void _loadEnvironmentVariables();
    HKEY _getRegistryRoot() const;
    std::wstring _getRegistryContextMenuHandlerPath() const;
    std::wstring _getRegistryBackgroundContextMenuHandlerPath() const;
    std::wstring _getRegistryFolderContextMenuHandlerPath() const;
    std::wstring _getRegistryCLSIDPath() const;
    std::wstring _getRegistryInprocServerPath() const;
    HRESULT _registerCreateKey(
        HKEY hKey,
        const std::wstring& subKey,
        const std::map<std::wstring, std::wstring>& setValues = std::map<std::wstring, std::wstring>()) const;

private:
    HINSTANCE _hInstance = nullptr;
    std::wstring _dllPath;
    std::atomic_int32_t _refCount = 0;
    wchar_t* _pathBuffer = nullptr;
    std::map<std::wstring, std::wstring> _environmentVariables;
};

#define gInstance InstanceGlobal::GetInstance()

#endif
