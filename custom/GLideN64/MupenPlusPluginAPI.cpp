#include "PluginAPI.h"
#include "Types.h"
#include "OpenGL.h"

extern "C" {

EXPORT int CALL gln64RomOpen(void)
{
	api().RomOpen();
	return 1;
}

EXPORT m64p_error CALL gln64PluginGetVersion(
	m64p_plugin_type * _PluginType,
	int * _PluginVersion,
	int * _APIVersion,
	const char ** _PluginNamePtr,
	int * _Capabilities
)
{
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL gln64PluginStartup(
	m64p_dynlib_handle CoreLibHandle,
	void *Context,
	void (*DebugCallback)(void *, int, const char *)
)
{
	return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL gln64PluginShutdown(void)
{
	video().stop();
	return M64ERR_SUCCESS;
}

EXPORT void CALL gln64ReadScreen2(void *dest, int *width, int *height, int front)
{
}

EXPORT void CALL gln64SetRenderingCallback(void (*callback)(int))
{
}

} // extern "C"
