//
//  RPObj.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#include "RPObj.hpp"
#include "RPPF.hpp"
#include "RPPF64.hpp"
#include "RPPF32.hpp"
#include "utils.hpp"

#include <Ramdisk/Ramdisk.hpp>
#include <libgeneral/macros.h>
#include <libgeneral/Utils.hpp>

#include <libpatchfinder/ibootpatchfinder/ibootpatchfinder32.hpp>
#include <libpatchfinder/ibootpatchfinder/ibootpatchfinder64.hpp>

using namespace tihmstar;

#define ASR_RAMDISK_PATH "/usr/sbin/asr"
#define LIBIMG4_RAMDISK_PATH "/usr/lib/libimg4.dylib"
#define RESTORED_RAMDISK_PATH "/usr/local/bin/restored_external"

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

static std::shared_ptr<RPPF> make_RPPF(const void *buf, size_t size){
    try {return std::make_shared<RPPF64>(buf, size);} catch (...) {}
    try {return std::make_shared<RPPF32>(buf, size);} catch (...) {}
    reterror("Failed to make RRPF");
}

#pragma mark RPObj
RPObj::RPObj()
: _haveAnyRamdiskPatchesEnabled{false}
, _doRPatchASRSigcheck{false}
, _doRPatchASRHardcodepw{false}
, _doRPatchLibIMG4Sigcheck{false}
, _doRPatchRestoredSigcheck{false}
, _doRPatchDeleteNVMEFW{false}
{
    //
}

RPObj::~RPObj(){
    //
}

#pragma mark public
bool RPObj::argparse(std::string longopt, const char *optarg){
    bool ret = true;
    if (longopt == "rpatch-asr-sigcheck") {
        info("Enabling ramdisk ASR sigcheck patch!");
        _doRPatchASRSigcheck = true;
    }else if (longopt == "rpatch-asr-hardcodepw") {
        info("Enabling ramdisk ASR hardcodepw patch!");
        _doRPatchASRHardcodepw = true;
    }else if (longopt == "rpatch-libimg4-sigcheck") {
        info("Enabling ramdisk libimg4 sigcheck patch!");
        _doRPatchLibIMG4Sigcheck = true;
    }else if (longopt == "rpatch-restored-sigcheck") {
        info("Enabling ramdisk restored sigcheck patch!");
        _doRPatchRestoredSigcheck = true;
    }else if (longopt == "rpatch-delete-nvmefw") {
        info("Enabling ramdisk delete nvme firmware!");
        _doRPatchDeleteNVMEFW = true;
    }else if (longopt == "rpatch-rm") {
        info("Deleting path '%s' in ramdisk",optarg);
        _actions.push_back({
            .type = kRDActionTypeDelete,
            .path = optarg,
        });
    }else if (longopt == "rpatch-put") {
        std::string putsrc = optarg;
        ssize_t comma = putsrc.find(",");
        if (comma == std::string::npos) {
            info("Creating empty file '%s' in ramdisk",putsrc.c_str());
            _actions.push_back({
                .type = kRDActionTypePut,
                .path = putsrc,
            });
        }else{
            std::string putdst = putsrc.substr(comma+1);
            putsrc = putsrc.substr(0,comma);
            info("Putting file '%s' to '%s' in ramdisk",putsrc.c_str(),putdst.c_str());
            _actions.push_back({
                .type = kRDActionTypePut,
                .path = putsrc,
                .args = {
                    putdst,
                },
            });
        }
    }else{
        ret = false;
    }

error:
    if (ret){
        _haveAnyRamdiskPatchesEnabled = true;
    }
    return ret;
}

std::vector<patchfinder::patch> RPObj::patcher(uint32_t component, const void *buf, size_t bufSize){
    if (component == 'ssbi' || component == 'cebi'){
        if (_doRPatchASRHardcodepw && !_platform.size()){
            patchfinder::ibootpatchfinder *ibpf = nullptr;
            cleanup([&]{
                safeDelete(ibpf);
            });
            try {
                ibpf = patchfinder::ibootpatchfinder32::make_ibootpatchfinder32(buf, bufSize);
                debug("Inited IBPF32");
            } catch (...) {
                debug("Failed initing IBPF32");
            }
            try {
                ibpf = patchfinder::ibootpatchfinder64::make_ibootpatchfinder64(buf, bufSize);
                debug("Inited IBPF64");
            } catch (...) {
                debug("Failed initing IBPF64");
            }
            retassure(ibpf, "Failed making IBPF");
            if (!ibpf->has_kernel_load()) {
                debug("Component '%.4s' doesn't have kernel load, skipping",&component);
            }else{
                _platform = ibpf->get_platform_name();
                info("Got platform-name=%s",_platform.c_str());
            }
        }
    }
    return {};
}

bool RPObj::wantsPatchingRamdisk(){
    return _haveAnyRamdiskPatchesEnabled;
}

tihmstar::Mem RPObj::ramdiskPatcher(const void *buf, size_t bufSize){
    std::shared_ptr<Ramdisk::Ramdisk> ramdisk = Ramdisk::Ramdisk::make_ramdisk(buf, bufSize);

    if (_doRPatchASRSigcheck || _doRPatchASRHardcodepw){
        tihmstar::Mem asr;
        std::vector<tihmstar::patchfinder::patch> asrPatches;
        std::shared_ptr<RPPF> asrPatcher;

        if (!asr.size()) asr = ramdisk->extractFile(ASR_RAMDISK_PATH);
        asrPatcher = make_RPPF(asr.data(), asr.size());

        if (_doRPatchASRSigcheck) {
            auto p = asrPatcher->getASRSigcheckPatches();
            asrPatches.insert(asrPatches.end(), p.begin(), p.end());
        }
        
        if (_doRPatchASRHardcodepw) {
            retassure(_platform.size(), "Can't derive passp for unknown platform!");
            std::string passp = utils::genpassp(_platform.c_str(), buf, bufSize);
            info("[RPObj] Rootfs pass: %s",passp.c_str());
            auto p = asrPatcher->getASRHardcodedPWPatches(passp.c_str());
            asrPatches.insert(asrPatches.end(), p.begin(), p.end());
        }
            
        if (asrPatches.size()) {
            asrPatcher->applyPatches(asrPatches);
            asr = resignFile(asrPatcher->buf(), asrPatcher->bufSize());
            ramdisk->putFile(ASR_RAMDISK_PATH, asr.data(), asr.size());
        }
    }
    
    {
        tihmstar::Mem restored;
        std::vector<tihmstar::patchfinder::patch> restoredPatches;
        std::shared_ptr<RPPF> restoredPatcher;
        
        if (_doRPatchRestoredSigcheck) {
            if (!restored.size()) restored = ramdisk->extractFile(RESTORED_RAMDISK_PATH);
            restoredPatcher = make_RPPF(restored.data(), restored.size());
            {
                auto p = restoredPatcher->getRestoredSigcheckPatches();
                restoredPatches.insert(restoredPatches.end(), p.begin(), p.end());
            }
        }
            
        if (restoredPatches.size()) {
            restoredPatcher->applyPatches(restoredPatches);
            restored = resignFile(restoredPatcher->buf(), restoredPatcher->bufSize());
            ramdisk->putFile(RESTORED_RAMDISK_PATH, restored.data(), restored.size());
        }
    }
    
    {
        tihmstar::Mem libimg4;
        std::vector<tihmstar::patchfinder::patch> libimg4Patches;
        std::shared_ptr<RPPF> libimg4Patcher;
        
        if (_doRPatchLibIMG4Sigcheck) {
            if (!libimg4.size()) libimg4 = ramdisk->extractFile(LIBIMG4_RAMDISK_PATH);
            libimg4Patcher = make_RPPF(libimg4.data(), libimg4.size());
            {
                auto p = libimg4Patcher->getLibIMG4SigcheckPatches();
                libimg4Patches.insert(libimg4Patches.end(), p.begin(), p.end());
            }
        }
            
        if (libimg4Patches.size()) {
            libimg4Patcher->applyPatches(libimg4Patches);
            libimg4 = resignFile(libimg4Patcher->buf(), libimg4Patcher->bufSize());
            ramdisk->putFile(LIBIMG4_RAMDISK_PATH, libimg4.data(), libimg4.size());
        }
    }
    
    if (_doRPatchDeleteNVMEFW){
        auto files = ramdisk->listFilesAtPath("/usr/standalone/firmware", true);
        for (auto f : files){
            if (f.ends_with(".pak")) ramdisk->rm(f.c_str());
        }
    }
    
    for (auto a : _actions) {
        switch (a.type) {
            case kRDActionTypeDelete:
                info("[RPObj] Deleting '%s'",a.path.c_str());
                ramdisk->rm(a.path.c_str(), true);
                break;
            
            case kRDActionTypePut:
            {
                if (a.args.size()) {
                    std::string putsrc = a.path;
                    std::string putdst = a.args.at(0);
                    info("[RPObj] Putting file '%s' to '%s' in ramdisk",putsrc.c_str(),putdst.c_str());
                    auto f = tihmstar::readFile(putsrc.c_str());
                    ramdisk->putFile(putdst.c_str(), f.data(), f.size());
                }else{
                    std::string putdst = a.path;
                    info("[RPObj] Creating empty file '%s' in ramdisk",putdst.c_str());
                    ramdisk->putFile(putdst.c_str(), NULL, 0);
                }
            }
                break;
                
            default:
                reterror("Unhandled action type %d", a.type);
        }
    }
    
    return {ramdisk->buf(), ramdisk->bufSize()};
}
