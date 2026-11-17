//
//  main.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//


#include <libgeneral/macros.h>
#include <libgeneral/Utils.hpp>

#include "RPPF64.hpp"
#include "RPObj.hpp"

MAINFUNCTION
int main_r(int argc, const char * argv[]) {
    info("%s",VERSION_STRING);
    
    auto rdsk = tihmstar::readFile(argv[1]);
    
    RPObj rp;
//    rp.argparse("rpatch-delete-nvmefw", "");
    rp.argparse("rpatch-asr-hardcodepw", "");
//    rp.argparse("rpatch-libimg4-sigcheck", "");
    {
        auto f = tihmstar::readFile(argv[2]);
        rp.patcher('cebi', f.data(), f.size());
    }
    auto rd = rp.ramdiskPatcher(rdsk.data(), rdsk.size());
    
    tihmstar::writeFile("dbg.dmg", rd.data(), rd.size());
    
    info("Done");
    return 0;
}
