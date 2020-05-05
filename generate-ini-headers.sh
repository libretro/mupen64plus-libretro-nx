#!/bin/bash

# Make sure we're in the script's directory
cd "$(dirname "$0")"

# Generate mupen64plus.ini.h
MUPEN_CORE_HEADER="#ifndef __INIFILE_H__
#define __INIFILE_H__

char inifile[] =
"
MUPEN_CORE_CONTENTS=$(while read -r f; do sed -nr 's/\\/\\\\/g; s/$/\\n/g; s/(^.*$)/"\1"/gp' <<< $f ; done < ./mupen64plus-core/data/mupen64plus.ini)
MUPEN_CORE_FOOTER=";

#endif
"
cat <<< ${MUPEN_CORE_HEADER}${MUPEN_CORE_CONTENTS}${MUPEN_CORE_FOOTER} > custom/mupen64plus-core/main/mupen64plus.ini.h

# Generate GLideN64.custom.ini.h 
GLIDEN64_HEADER="#ifndef __CUSTOM_INI__
#define __CUSTOM_INI__

char customini[] =
"
GLIDEN64_CONTENTS=$(while read -r f; do sed -nr 's/\\/\\\\/g; s/$/\\n/g; s/(^.*$)/"\1"/gp' <<< $f; done < ./GLideN64/ini/GLideN64.custom.ini)
GLIDEN64_FOOTER=";

#endif
"
cat <<< ${GLIDEN64_HEADER}${GLIDEN64_CONTENTS}${GLIDEN64_FOOTER} > custom/GLideN64/GLideN64.custom.ini.h
