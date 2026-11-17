//
//  RPPF64.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#include "RPPF64.hpp"

#define pushINSN(pinsn) do {arm64::insn pinsnn = pinsn; uint32_t opcode = pinsnn.opcode();patches.push_back({pinsnn,&opcode,sizeof(opcode)});} while (0)
#define addPatches(func) do {auto p = func;patches.insert(patches.end(), p.begin(), p.end());} while (0)

//without caching
#   define RETCACHEPATCHES return patches
#   define UNCACHEPATCHES std::vector<patch> patches
#   define RETCACHELOC(loc) do {loc_t l = (loc); return l;} while(0)
#   define UNCACHELOC

using namespace tihmstar::patchfinder;
using namespace tihmstar::libinsn;
using namespace tihmstar::libinsn::arm64;

#pragma mark reexpose
const void *RPPF64::buf(){
    return tihmstar::patchfinder::machopatchfinder64::machopatchfinder64::buf();
}

size_t RPPF64::bufSize(){
    return tihmstar::patchfinder::machopatchfinder64::machopatchfinder64::bufSize();
}

const void *RPPF64::memoryForLoc(tihmstar::patchfinder::patchfinder::loc_t loc){
    return tihmstar::patchfinder::machopatchfinder64::machopatchfinder64::memoryForLoc(loc);
}

#pragma mark public
std::vector<patch> RPPF64::getASRSigcheckPatches(){
    UNCACHEPATCHES;
    loc_t ref = 0;
    loc_t str = 0;
    try {
        str = findstr("Image failed signature verification", true);
        debug("str=0x%016llx",str);
    } catch (...) {
        //
    }
    if (str){
        /*
            Pre-iOS 15.0
         */
        ref = find_literal_ref(str);
        debug("ref=0x%016llx",ref);
    }else{
        /*
            Post iOS 15.0
         */
        str = findstr("Image failed signature verification", false); //in iOS 15.0 the added a "." at the end. Thanks!
        debug("str=0x%016llx",str);

        ref = find_literal_ref(str);
        debug("ref=0x%016llx",ref);
        
        loc_t bof = find_bof(ref);
        debug("bof=0x%016llx",bof);
        if (ref-bof < 4*10) {
            debug("failfunc detected!");
            ref = find_call_ref(bof);
            debug("ref_fixed=0x%016llx",ref);
        }
    }

    loc_t str2 = findstr("Image passed signature verification", true);
    debug("str2=0x%016llx",str2);

    loc_t ref2 = find_literal_ref(str2);
    debug("ref2=0x%016llx",ref2);

    pushINSN(insn::new_immediate_b(ref, ref2));

    RETCACHEPATCHES;
}

std::vector<patch> RPPF64::getLibIMG4SigcheckPatches(){
    UNCACHEPATCHES;
    
    loc_t func = find_sym("__img4_firmware_object_property_callback");
    debug("func=0x%016llx",func);

    auto iter = _vmem->getIter(func);
    while (++iter != insn::ret)
        ;

    loc_t eof = iter;
    debug("eof=0x%016llx",eof);
    while (--iter != insn::mov || iter().rd() != 0)
        ;
    
    loc_t tgt = iter;
    debug("tgt=0x%016llx",tgt);
    
    pushINSN(insn::new_immediate_movz(tgt, 0, 0, 0));
    
    RETCACHEPATCHES;
}

std::vector<patch> RPPF64::getRestoredSigcheckPatches(){
    UNCACHEPATCHES;
    
    loc_t str = findstr("Transfer: Digest mismatch (expected", false);
    debug("str=0x%016llx",str);

    loc_t ref = find_literal_ref(str);
    debug("ref=0x%016llx",ref);

    loc_t str2 = findstr("Transfer: Digest matched bytes streamed", false);
    debug("str2=0x%016llx",str2);

    loc_t ref2 = find_literal_ref(str2);
    debug("ref2=0x%016llx",ref2);

    auto iter = _vmem->getIter(ref2);
    if (--iter != insn::adrp) ++iter;
    
    pushINSN(insn::new_immediate_b(ref, iter));
    
    loc_t str3 = findstr("read-back digest verification of %s succeeded", false);
    debug("str3=0x%016llx",str3);

    loc_t ref3 = find_literal_ref(str3);
    debug("ref3=0x%016llx",ref3);

    iter = ref3;

    while ((--iter).supertype() != insn::sut_branch_imm)
        ;
    ++iter;
    
    loc_t bref = find_branch_ref(iter, -0x100);
    debug("bref=0x%016llx",bref);

    pushINSN(insn::new_immediate_b(bref, iter));
    
    RETCACHEPATCHES;
}

std::vector<tihmstar::patchfinder::patch> RPPF64::getASRHardcodedPWPatches(const char *rootfsPW){
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

    loc_t shellcode = bof;
    debug("shellcode=0x%016llx",shellcode);
    
    uint32_t shellcode_insn_cnt = 6; //commitment

#define cPC (shellcode+(insnNum++)*4)
    int insnNum = 0;
    pushINSN(insn::new_literal_ldr(cPC, kCFAllocatorDefault, 0));
    pushINSN(insn::new_immediate_ldr_unsigned(cPC, 0, 0, 0));
    pushINSN(insn::new_immediate_movz(cPC, 0x800, 2, 16, true));
    pushINSN(insn::new_immediate_movk(cPC, 0x100, 2, 0, true));
    pushINSN(insn::new_general_adr(cPC, shellcode+shellcode_insn_cnt*4, 1));
    pushINSN(insn::new_immediate_b(cPC, CFStringCreateWithCString));
    assure(insnNum == shellcode_insn_cnt);
#undef cPC
    
    patches.push_back({
        shellcode+shellcode_insn_cnt*4,
        rootfsPW,
        strlen(rootfsPW)+1
    });

    return patches;
}
