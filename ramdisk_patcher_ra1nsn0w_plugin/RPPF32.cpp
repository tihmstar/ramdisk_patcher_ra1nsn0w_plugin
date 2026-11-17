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

#pragma mark reexpose
const void *RPPF32::buf(){
    return tihmstar::patchfinder::machopatchfinder32::machopatchfinder32::buf();
}

size_t RPPF32::bufSize(){
    return tihmstar::patchfinder::machopatchfinder32::machopatchfinder32::bufSize();
}

const void *RPPF32::memoryForLoc(tihmstar::patchfinder::patchfinder::loc_t loc){
    return tihmstar::patchfinder::machopatchfinder32::machopatchfinder32::memoryForLoc(loc);
}

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
    RETCACHEPATCHES;
}

std::vector<tihmstar::patchfinder::patch> RPPF32::getASRHardcodedPWPatches(const char *rootfsPW){
    std::vector<patch> patches;
    loc_t str = findstr("Couldn't read device", true);
    debug("str=0x%016llx",str);

    loc_t ref = find_literal_ref(str);
    debug("ref=0x%016llx",ref);

    loc_t bof = find_bof(ref);
    debug("bof=0x%016llx",bof);
    
    loc_t CFStringCreateWithCString = find_sym_stub("_CFStringCreateWithCString");
    debug("CFStringCreateWithCString=0x%016llx",CFStringCreateWithCString);

    loc_t kCFAllocatorDefault = find_lazy_sym_loc("_kCFAllocatorDefault");
    debug("kCFAllocatorDefault=0x%016llx",kCFAllocatorDefault);

    uint32_t shellcode_insn_cnt = 19; //commitment
    loc_t shellcode = bof;
    debug("shellcode=0x%016llx",shellcode);

#define cPC (shellcode+(insnNum)*2)
#define pushINSNCPC(_pinsn) do {auto _pinsnn = _pinsn; pushINSN(_pinsnn); insnNum += (_pinsnn.insnsize()>>1);} while (0);

    int insnNum = 0;
    {
        uint32_t val = kCFAllocatorDefault-(shellcode+(insnNum*2)+3*4);
        pushINSNCPC(thumb::new_T3_immediate_movw(cPC, val, 0));
        pushINSNCPC(thumb::new_T1_immediate_movt(cPC, val>>16, 0));
        pushINSNCPC(thumb::new_T2_register_add(cPC, 0, 15));
    }
    pushINSNCPC(thumb::new_T1_immediate_ldr(cPC, 0, 0, 0));
    pushINSNCPC(thumb::new_T3_immediate_movw(cPC, 0x100, 2));
    pushINSNCPC(thumb::new_T1_immediate_movt(cPC, 0x800, 2));
    pushINSNCPC(thumb::new_T1_immediate_movs(cPC, 2, 1));
    {
        uint32_t val = CFStringCreateWithCString-(shellcode+(insnNum*2)+3*4);
        pushINSNCPC(thumb::new_T3_immediate_movw(cPC, val, 3));
        pushINSNCPC(thumb::new_T1_immediate_movt(cPC, val>>16, 3));
        pushINSNCPC(thumb::new_T2_register_add(cPC, 3, 15));
    }
    pushINSNCPC(thumb::new_T2_register_add(cPC, 1, 15));
    pushINSNCPC(thumb::new_T1_immediate_ldr(cPC, 0, 0, 0));
    pushINSNCPC(thumb::new_T1_general_bx(cPC, 3));
    assure(insnNum == shellcode_insn_cnt);
#undef cPC
    
    patches.push_back({
        shellcode+shellcode_insn_cnt*2,
        rootfsPW,
        strlen(rootfsPW)+1
    });

    return patches;
}
