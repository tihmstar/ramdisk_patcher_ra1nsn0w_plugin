//
//  plugin.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
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
"      --rdisk-replace-reboot <PATH>\t\tReplace /sbin/reboot with custom file\n" \
"\n"
;

static struct option longopts[] = {
    { "rpatch-asr-sigcheck",        no_argument,            NULL,  0  },
    { "rdisk-replace-reboot",       required_argument,      NULL,  0  },
    { NULL, 0, NULL, 0 }
};

static std::shared_ptr<PluginObj> RPObj_init(void){
    return std::make_shared<RPObj>();
}

Plugin pluginDef = {
    .apiVersion = RA1NSN0W_PLUGIN_API_VERSION,
    .pluginName = "ramdisk_patcher",
    .cmdHelp = helpscreen,
    .longopts = longopts,
    .init = RPObj_init,
};
