//
//  RPRamdisk.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#ifndef RPRamdisk_hpp
#define RPRamdisk_hpp

#include <libgeneral/Mem.hpp>

#include <memory>

#include <stdlib.h>

extern "C" {
#include <xpwn/dmg/dmgfile.h>
}

class RPRamdisk {
    void *_locbuf;
    AbstractFile *_af;
    io_func *_afio;
    Volume *_hfs;
    
    tihmstar::Mem extractFileHFS(const char *path);
    tihmstar::Mem extractFileAPFS(const char *path);
    void putFileHFS(const char *path, const void *buf, size_t bufSize);
public:
    RPRamdisk(const void *buf, size_t bufSize);
    ~RPRamdisk();
    
    tihmstar::Mem extractFile(const char *path);
    void putFile(const char *path, const void *buf, size_t bufSize);
    void growRamdisk(size_t size);

    const void *buf();
    size_t bufSize();
};

#endif /* RPRamdisk_hpp */
