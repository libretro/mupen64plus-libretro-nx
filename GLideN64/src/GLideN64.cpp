char pluginName[] = "GLideN64";
#ifdef PLUGIN_REVISION
char pluginNameWithRevision[] = "GLideN64 rev." PLUGIN_REVISION;
#else // PLUGIN_REVISION
char pluginNameWithRevision[] = "GLideN64 rev.NX";
#endif // PLUGIN_REVISION
wchar_t pluginNameW[] = L"GLideN64 rev.NX";
void (*CheckInterrupts)( void );
