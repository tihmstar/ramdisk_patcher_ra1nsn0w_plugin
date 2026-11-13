//
//  main.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//


#include <libgeneral/macros.h>
#include <libgeneral/Utils.hpp>

#include "RPRamdisk.hpp"
#include "RPObj.hpp"

MAINFUNCTION
int main_r(int argc, const char * argv[]) {
    info("%s",VERSION_STRING);
    
    auto rdsk = tihmstar::readFile(argv[1]);
    
    RPObj rp;
    rp.argparse("rdisk-replace-reboot", "/bin/ls");
    rp.argparse("rpatch-asr-sigcheck", NULL);
    auto rd = rp.ramdiskPatcher(rdsk.data(), rdsk.size());
    
    tihmstar::writeFile("dbg.dmg", rd.data(), rd.size());
    
    info("Done");
    return 0;
}
