//
//  RPPF.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#ifndef RPPF_hpp
#define RPPF_hpp

#include <libpatchfinder/patchfinder.hpp>

class RPPF {
public:
#pragma mark pure virtual
    virtual const void *buf() = 0;
    virtual size_t bufSize() = 0;
    virtual const void *memoryForLoc(tihmstar::patchfinder::patchfinder::loc_t loc) = 0;

#pragma mark provider
    void applyPatches(std::vector<tihmstar::patchfinder::patch> patches);

#pragma mark no-provider
    virtual std::vector<tihmstar::patchfinder::patch> getASRSigcheckPatches();
    virtual std::vector<tihmstar::patchfinder::patch> getLibIMG4SigcheckPatches();
    virtual std::vector<tihmstar::patchfinder::patch> getRestoredSigcheckPatches();
    virtual std::vector<tihmstar::patchfinder::patch> getASRHardcodedPWPatches(const char *rootfsPW);
};

#endif /* RPPF_hpp */
