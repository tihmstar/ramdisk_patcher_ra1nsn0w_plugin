//
//  RPPF32.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 13.11.26.
//

#ifndef RPPF32_hpp
#define RPPF32_hpp

#include <libpatchfinder/machopatchfinder32.hpp>

class RPPF32 : public tihmstar::patchfinder::machopatchfinder32 {
public:
    using tihmstar::patchfinder::machopatchfinder32::machopatchfinder32;
    
    std::vector<tihmstar::patchfinder::patch> getASRSigcheckPatches();
    
    void applyPatches(std::vector<tihmstar::patchfinder::patch> patches);
};
#endif /* RPPF32_hpp */
