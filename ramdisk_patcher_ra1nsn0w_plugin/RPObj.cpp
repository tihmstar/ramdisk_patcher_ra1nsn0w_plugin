//
//  RPObj.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#include "RPObj.hpp"
#include "RPPF32.hpp"
#include "RPRamdisk.hpp"
#include "utils.hpp"
#include <libgeneral/macros.h>
#include <libgeneral/Utils.hpp>

using namespace tihmstar;

#define ASR_RAMDISK_PATH "/usr/sbin/asr"

static tihmstar::Mem resignFile(const void *buf, size_t bufSize){
    std::string tmpdir = utils::getTmpdir();
    std::string tmp_path = tmpdir + PATH_DELIMITER_STRING "asr";
    cleanup([&]{
        unlink(tmp_path.c_str());
        remove(tmpdir.c_str());
    });
    tihmstar::writeFile(tmp_path.c_str(), buf, bufSize);
    /*
        YES i know this is _extremely_ ugly, but honestly i'm currently too lazy to do it properly, sorry :/
     */
    {
        int err = 0;
        char command[0x400] = {};
        retassure(snprintf(command, sizeof(command), "/usr/local/bin/ldid -M -S %s",tmp_path.c_str()) < sizeof(command), "Failed to construct command for ldid");
        retassure((err = system(command)) == 0,"ldid failed with err=%d",err);
    }
    return readFile(tmp_path.c_str());
}

#pragma mark RPObj
RPObj::RPObj()
: _replaceReboot(NULL)
, _doRPatchASRSigchek{false}
{
    //
}

RPObj::~RPObj(){
    info("~RPObj");
}

#pragma mark public
bool RPObj::argparse(std::string longopt, const char *optarg){
    bool ret = false;
    if (longopt == "rpatch-asr-sigcheck") {
        info("Enabling ramdisk ASR sigcheck patch!");
        _doRPatchASRSigchek = true;
        ret = true;
    } else if (longopt == "rdisk-replace-reboot") {
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
    
    {
        tihmstar::Mem asr;
        std::vector<tihmstar::patchfinder::patch> asrPatches;
        std::shared_ptr<RPPF32> asrPatcher;
        
        if (_doRPatchASRSigchek) {
            if (!asr.size()) asr = ramdisk.extractFile(ASR_RAMDISK_PATH);
            asrPatcher = std::make_shared<RPPF32>(asr.data(), asr.size());
            {
                auto p = asrPatcher->getASRSigcheckPatches();
                asrPatches.insert(asrPatches.end(), p.begin(), p.end());
            }
        }
            
        if (asrPatches.size()) {
            asrPatcher->applyPatches(asrPatches);
            asr = resignFile(asrPatcher->buf(), asrPatcher->bufSize());
            ramdisk.putFile(ASR_RAMDISK_PATH, asr.data(), asr.size());
        }
    }
    
    if (_replaceReboot){
        auto nf = tihmstar::readFile(_replaceReboot);
        ramdisk.putFile("/sbin/reboot", nf.data(), nf.size());
        info("Ramdisk binary replaced!");
    }

    info("Ramdisk generated!");
    return {ramdisk.buf(), ramdisk.bufSize()};
}
