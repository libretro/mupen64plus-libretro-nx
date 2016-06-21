#include "GLideN64_mupenplus.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../Config.h"
#include "../GLideN64.h"
#include "../OpenGL.h"
#include "../GBI.h"
#include "../RSP.h"
#include "../Log.h"
extern "C" {
#include <sys/stat.h>
#include "osal_files.h"
#include <dirent.h>
}

Config config;

void Config_LoadConfig()
{
	config.resetToDefaults();
}

EXPORT int CALL osal_path_existsW(const wchar_t *_path) {
    char path[PATH_MAX];
    wcstombs(path, _path, PATH_MAX);
    struct stat fileinfo;
    return stat(path, &fileinfo) == 0 ? 1 : 0;
}

EXPORT int CALL osal_is_directory(const wchar_t * _name) {
    char name[PATH_MAX + 1];
    wcstombs(name, _name, PATH_MAX);
    DIR* dir;
    dir = opendir(name);
    if(dir != NULL)
    {
        closedir(dir);
        return 1;
    }
    return 0;
}
