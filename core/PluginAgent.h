#ifndef MOUS_PLUGINAGENT_H
#define MOUS_PLUGINAGENT_H

#include <set>
#include <string>
#include <dlfcn.h>
#include <mous/ErrorCode.h>
#include <mous/PluginHelper.h>
namespace mous {

class IPluginAgent
{
public:
    virtual ~IPluginAgent() { }

    virtual EmPluginType GetType() const = 0;
    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;
    virtual const PluginInfo* GetInfo() = 0;
    virtual void* GetVpPlugin() = 0;
};

template<typename PluginSuperClass>
class PluginAgent: public IPluginAgent
{
    typedef PluginSuperClass* (*FnCreatePlugin)(void);
    typedef void (*FnReleasePlugin)(PluginSuperClass*);
    typedef const PluginInfo* (*FnGetPluginInfo)(void);

public:
    explicit PluginAgent(EmPluginType type):
	m_Type(type),
	m_pHandle(NULL),
	m_fnCreate(NULL),
	m_fnRelease(NULL),
	m_fnGetInfo(NULL),
	m_pPlugin(NULL)
    {

    }

    virtual ~PluginAgent()
    {

    }

    virtual EmPluginType GetType() const
    {
	return m_Type;
    }

    virtual EmErrorCode Open(const std::string& path)
    {
	m_pHandle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (m_pHandle == NULL)
	    return ErrorCode::MgrBadFormat;

	m_fnGetInfo = (FnGetPluginInfo)dlsym(m_pHandle, StrGetPluginInfo);
	if (m_fnGetInfo == NULL)
	    return ErrorCode::MgrBadFormat;

	m_fnCreate = (FnCreatePlugin)dlsym(m_pHandle, StrCreatePlugin);
	if (m_fnCreate == NULL)
	    return ErrorCode::MgrBadFormat;

	m_fnRelease = (FnReleasePlugin)dlsym(m_pHandle, StrReleasePlugin);
	if (m_fnCreate == NULL)
	    return ErrorCode::MgrBadFormat;

	m_pPlugin = m_fnCreate();
	if (m_pPlugin == NULL)
	    return ErrorCode::MgrBadFormat;

	return ErrorCode::Ok;
    }

    virtual void Close()
    {
	if (m_fnRelease != NULL) {
	    m_fnRelease(m_pPlugin);
	    m_fnCreate = NULL;
	    m_fnRelease = NULL;
	    m_fnGetInfo = NULL;
	    m_pPlugin = NULL;
	}

	if (m_pHandle != NULL) {
	    dlclose(m_pHandle);
	    m_pHandle = NULL;
	}

	m_Type = PluginType::None;
    }

    virtual const PluginInfo* GetInfo()
    {
	return (m_fnGetInfo != NULL) ? m_fnGetInfo() : NULL;
    }

    virtual void* GetVpPlugin()
    {
	return m_pPlugin;
    }

    PluginSuperClass* GetPlugin()
    {
	return m_pPlugin;
    }

private:
    EmPluginType m_Type;

    void* m_pHandle;

    FnCreatePlugin m_fnCreate;
    FnReleasePlugin m_fnRelease;
    FnGetPluginInfo m_fnGetInfo;

    PluginSuperClass* m_pPlugin;
};

}
#endif
