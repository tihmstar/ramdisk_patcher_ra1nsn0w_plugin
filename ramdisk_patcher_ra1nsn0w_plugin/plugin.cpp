//
//  plugin.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#include "RPObj.hpp"

#include <libgeneral/macros.h>
#include <ra1nsn0w/ra1nsn0w.hpp>
#include <ra1nsn0w/ra1nsn0w_plugins.hpp>

using namespace tihmstar;
using namespace tihmstar::ra1nsn0w;

#pragma mark global vars
static const char helpscreen[] =
"Ramdisk Patcher:\n"
"      --rpatch-asr-sigcheck\t\t\tPatch ASR Sigchecks (inside ramdisk)\n" \
"      --rpatch-asr-hardcodepw\t\t\tPatch ASR to hardcode rootfs pw\n" \
"      --rpatch-libimg4-sigcheck\t\t\tPatch libimg4 Sigchecks (inside ramdisk)\n" \
"      --rpatch-restored-sigcheck\t\tPatch restored external Sigchecks (inside ramdisk)\n" \
"      --rpatch-delete-nvmefw\t\t\tDelete nvme firmware (inside ramdisk)\n" \
"      --rpatch-rm <PATH>\t\t\tDelete PATH inside ramdisk (recursively)\n" \
"      --rpatch-put [<SRC>,]<DST>\t\tPut file from local <src> to <dst> on ramdisk, if <SRC> is omitted an empty file is created\n" \
"\n"
;

static struct option longopts[] = {
    { "rpatch-asr-sigcheck",        no_argument,        NULL,  0  },
    { "rpatch-asr-hardcodepw",      no_argument,        NULL,  0  },
    { "rpatch-libimg4-sigcheck",    no_argument,        NULL,  0  },
    { "rpatch-restored-sigcheck",   no_argument,        NULL,  0  },
    { "rpatch-delete-nvmefw",       no_argument,        NULL,  0  },
    { "rpatch-rm",                  required_argument,  NULL,  0  },
    { "rpatch-put",                 required_argument,  NULL,  0  },
    { NULL, 0, NULL, 0 }
};

static std::shared_ptr<PluginObj> RPObj_init(void){
    return std::make_shared<RPObj>();
}

Plugin pluginDef = {
    .apiVersion = RA1NSN0W_PLUGIN_API_VERSION,
    .pluginName = "ramdisk_patcher_ra1nsn0w_plugin",
    .cmdHelp = helpscreen,
    .longopts = longopts,
    .init = RPObj_init,
};
