//
//  RPPF64.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#ifndef RPPF64_hpp
#define RPPF64_hpp

#include "RPPF.hpp"
#include <libpatchfinder/machopatchfinder64.hpp>

class RPPF64 : public tihmstar::patchfinder::machopatchfinder64, public RPPF {
public:
    using tihmstar::patchfinder::machopatchfinder64::machopatchfinder64;
    
#pragma mark reexpose
    virtual const void *buf() override;
    virtual size_t bufSize() override;
    virtual const void *memoryForLoc(tihmstar::patchfinder::patchfinder::loc_t loc) override;

#pragma mark patches
    virtual std::vector<tihmstar::patchfinder::patch> getASRSigcheckPatches() override;
    
    virtual std::vector<tihmstar::patchfinder::patch> getLibIMG4SigcheckPatches() override;

    virtual std::vector<tihmstar::patchfinder::patch> getRestoredSigcheckPatches() override;

    virtual std::vector<tihmstar::patchfinder::patch> getASRHardcodedPWPatches(const char *rootfsPW) override;
};


#endif /* RPPF64_hpp */
