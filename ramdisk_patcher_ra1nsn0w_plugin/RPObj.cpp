//
//  RPObj.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#include "RPObj.hpp"
#include "RPRamdisk.hpp"
#include "utils.hpp"
#include <libgeneral/macros.h>
#include <libgeneral/Utils.hpp>

using namespace tihmstar;

#pragma mark RPObj
RPObj::RPObj()
: _replaceReboot(NULL)
{
    //
}

RPObj::~RPObj(){
    //
}

#pragma mark public
bool RPObj::argparse(std::string longopt, const char *optarg){
    bool ret = false;
    if (longopt == "rdisk-replace-reboot") {
        info("Enabling replace reboot");
        _replaceReboot = optarg;
        ret = true;
    }
    
error:
    if (ret){
        _haveAnyRamdiskPatchesEnabled = true;
    }
    return ret;
}

std::vector<patchfinder::patch> RPObj::patcher(uint32_t component, const void *buf, size_t bufSize){
    return {};
}

bool RPObj::wantsPatchingRamdisk(){
    return _haveAnyRamdiskPatchesEnabled;
}

tihmstar::Mem RPObj::ramdiskPatcher(const void *buf, size_t bufSize){
    RPRamdisk ramdisk(buf, bufSize);
    
    if (_replaceReboot){
        auto nf = tihmstar::readFile(_replaceReboot);
        ramdisk.putFile("/sbin/reboot", nf.data(), nf.size());
        info("Ramdisk binary replaced!");
    }

    info("Ramdisk generated!");
    return {ramdisk.buf(), ramdisk.bufSize()};
}
