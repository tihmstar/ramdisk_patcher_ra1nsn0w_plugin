//
//  RPPF32.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 13.11.26.
//

#include "RPPF32.hpp"

#define pushINSN(pinsn) do {auto pinsnn = pinsn; uint32_t opcode = pinsnn.opcode();patches.push_back({(loc_t)pinsnn,&opcode,pinsnn.insnsize()});} while (0)
#define addPatches(func) do {auto p = func;patches.insert(patches.end(), p.begin(), p.end());} while (0)

//without caching
#   define RETCACHEPATCHES return patches
#   define UNCACHEPATCHES std::vector<patch> patches
#   define RETCACHELOC(loc) do {loc_t l = (loc); return l;} while(0)
#   define UNCACHELOC

using namespace tihmstar::patchfinder;
using namespace tihmstar::libinsn;
using namespace tihmstar::libinsn::arm32;

#pragma mark public
std::vector<patch> RPPF32::getASRSigcheckPatches(){
    UNCACHEPATCHES;
    loc_t str = findstr("Image failed signature verification", true);
    debug("str=0x%016llx",str);
    loc_t ref = find_literal_ref(str);
    debug("ref=0x%016llx",ref);

    loc_t str2 = findstr("Image passed signature verification", true);
    debug("str2=0x%016llx",str2);

    loc_t ref2 = find_literal_ref(str2);
    debug("ref2=0x%016llx",ref2);

    pushINSN(thumb::new_T2_immediate_b(ref, ref2));

    {
        loc_t str = 0;
        try {
            str = findstr("No image hash to compare.", false);
        } catch (...) {
            //
        }
        if (str){
            /*
                iPhone5,2 iOS 10.3.4
             */
            debug("str=0x%016llx",str);

            loc_t ref = find_literal_ref(str);
            debug("ref=0x%016llx",ref);
            auto iter = _vmemThumb->getIter(ref);
            while ((--iter).supertype() != arm32::sut_branch_imm)
                ;
            ++iter;
            loc_t blk = iter;
            debug("blk=0x%016llx",blk);
            
            loc_t bref = find_branch_ref(blk, -0x400);
            debug("bref=0x%016llx",bref);
            iter = bref;
            pushINSN(thumb::new_T1_general_nop(iter.pc()));
            if (iter().insnsize() == 4){
                pushINSN(thumb::new_T1_general_nop(iter.pc()+2));
            }
        }
    }
    
    {
        /*
            iPhone5,2 iOS 9.3.5
         */
        //missing
    }
    
    RETCACHEPATCHES;
}

void RPPF32::applyPatches(std::vector<tihmstar::patchfinder::patch> patches){
    for (auto p : patches) {
        uint8_t *file = (uint8_t*)buf();
        uint64_t off = (uint64_t)((const uint8_t *)memoryForLoc(p._location) - file);
#ifdef DEBUG
        printf("RPPF32: Applying patch=%p : ",(void*)p._location);
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
    debug("RPPF32: Patches applied!");
#endif
}
