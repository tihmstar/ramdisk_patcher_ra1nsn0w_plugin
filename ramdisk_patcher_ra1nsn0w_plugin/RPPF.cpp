//
//  RPPF.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#include "RPPF.hpp"

#pragma mark provider

void RPPF::applyPatches(std::vector<tihmstar::patchfinder::patch> patches){
    for (auto p : patches) {
        uint8_t *file = (uint8_t*)buf();
        uint64_t off = (uint64_t)((const uint8_t *)memoryForLoc(p._location) - file);
#ifdef DEBUG
        printf("RPPF: Applying patch=%p : ",(void*)p._location);
        for (int i=0; i<p.getPatchSize(); i++) {
            printf("%02x",((uint8_t*)p.getPatch())[i]);
        }
        if (p.getPatchSize() == 4) {
            printf(" 0x%08x",*(uint32_t*)p.getPatch());
        } else if (p.getPatchSize() == 2) {
            printf(" 0x%04x",*(uint16_t*)p.getPatch());
        }
        printf("\n");
#endif
        memcpy(&file[off], p.getPatch(), p.getPatchSize());
    }
#ifdef DEBUG
    debug("RPPF: Patches applied!");
#endif
}

#pragma mark noimplement
std::vector<tihmstar::patchfinder::patch> RPPF::getASRSigcheckPatches(){
    FAIL_UNIMPLEMENTED;
}

std::vector<tihmstar::patchfinder::patch> RPPF::getLibIMG4SigcheckPatches(){
    FAIL_UNIMPLEMENTED;
}

std::vector<tihmstar::patchfinder::patch> RPPF::getRestoredSigcheckPatches(){
    FAIL_UNIMPLEMENTED;
}

std::vector<tihmstar::patchfinder::patch> RPPF::getASRHardcodedPWPatches(const char *rootfsPW){
    FAIL_UNIMPLEMENTED;
}
