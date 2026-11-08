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
"      --rdisk-replace-reboot <PATH>\t\tReplace /sbin/reboot with custom file\n" \
"\n"
;

static struct option longopts[] = {
    { "rdisk-replace-reboot",      no_argument,    NULL,  0  },
    { NULL, 0, NULL, 0 }
};

static std::shared_ptr<PluginObj> RPObj_init(void){
    return std::make_shared<RPObj>();
}

static Plugin gPluginDef = {
    .apiVersion = RA1NSN0W_PLUGIN_API_VERSION,
    .cmdHelp = helpscreen,
    .longopts = longopts,
    .init = RPObj_init,
};

#pragma mark constructor
__attribute__ ((constructor))
void constructor(void){
    pluginRegister(&gPluginDef);
}

__attribute__((destructor))
void destructor(void){
    pluginUnregister(&gPluginDef);
}
