//
//  RPPF32.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 13.11.26.
//

#ifndef RPPF32_hpp
#define RPPF32_hpp

#include "RPPF.hpp"
#include <libpatchfinder/machopatchfinder32.hpp>

class RPPF32 : public tihmstar::patchfinder::machopatchfinder32, public RPPF {
public:
    using tihmstar::patchfinder::machopatchfinder32::machopatchfinder32;
    
#pragma mark reexpose
    virtual const void *buf() override;
    virtual size_t bufSize() override;
    virtual const void *memoryForLoc(tihmstar::patchfinder::patchfinder::loc_t loc) override;
    
#pragma mark patches
    virtual std::vector<tihmstar::patchfinder::patch> getASRSigcheckPatches() override;
    
    virtual std::vector<tihmstar::patchfinder::patch> getASRHardcodedPWPatches(const char *rootfsPW) override;
};
#endif /* RPPF32_hpp */
