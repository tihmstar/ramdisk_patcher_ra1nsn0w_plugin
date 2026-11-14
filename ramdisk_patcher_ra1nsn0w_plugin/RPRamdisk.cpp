//
//  RPRamdisk.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#include "RPRamdisk.hpp"

#include <libgeneral/macros.h>

extern "C" {
#include <libdmg-hfsplus/hfs/hfslib.h>
char endianness;
}


#pragma mark RPRamdisk
RPRamdisk::RPRamdisk(const void *buf, size_t bufSize){
    {
        short int word = 0x0001;
        char *byte = (char *) &word;
        endianness = byte[0] ? IS_LITTLE_ENDIAN : IS_BIG_ENDIAN;
    }
    _locbuf = malloc(bufSize);
    memcpy(_locbuf, buf, bufSize);
    _af = createAbstractFileFromMemory(&_locbuf, bufSize);
    _afio = IOFuncFromAbstractFile(_af);
    _hfs = openVolume(_afio);
}

RPRamdisk::~RPRamdisk(){
    safeFreeCustom(_hfs, closeVolume);
    if (_afio) {
        _afio->close(_afio);
        _afio = NULL;
    }else if (_af) {
        _af->close(_af);
        _af = NULL;
    }
    _locbuf = NULL;
}

#pragma mark private
tihmstar::Mem RPRamdisk::extractFileHFS(const char *path){
    void *buf = NULL;
    AbstractFile *af = NULL;
    cleanup([&]{
        if (af){
            af->close(af); af = NULL;
        }
        safeFree(buf);
    });
    buf = malloc(0x1000*20);
    af = createAbstractFileFromMemory(&buf, 0x1000*20);
    get_hfs(_hfs, path, af);
    size_t realSize = af->tell(af);
    return {(const void*)buf,realSize};
}

tihmstar::Mem RPRamdisk::extractFileAPFS(const char *path){
    reterror("TODO");
}

void RPRamdisk::putFileHFS(const char *path, const void *buf, size_t bufSize){
    void *locbuf = NULL;
    AbstractFile *af = NULL;
    cleanup([&]{
        if (af){
            af->close(af); af = NULL;
        }
        safeFree(locbuf);
    });
    removeFile(path, _hfs);
    growRamdisk(bufSize);
    locbuf = malloc(bufSize);
    memcpy(locbuf, buf, bufSize);
    af = createAbstractFileFromMemory(&locbuf, bufSize);
    add_hfs(_hfs, af, path); af = NULL;
    chmodFile(path, 0777, _hfs);
    updateVolume(_hfs);
}

#pragma mark public
tihmstar::Mem RPRamdisk::extractFile(const char *path){
    return extractFileHFS(path);
    reterror("No ramdisk backend available for FS Type");
}

void RPRamdisk::putFile(const char *path, const void *buf, size_t bufSize){
    return putFileHFS(path, buf, bufSize);
    reterror("No ramdisk backend available for file creation");
}

void RPRamdisk::growRamdisk(size_t size){
    size_t addsize = size & ~0x3fff;
    if (size & 0x3fff) addsize += 0x4000;
    grow_hfs(_hfs, _hfs->volumeHeader->totalBlocks * _hfs->volumeHeader->blockSize +addsize);
}

const void *RPRamdisk::buf(){
    return _locbuf;
}

size_t RPRamdisk::bufSize(){
    return _hfs->volumeHeader->totalBlocks * _hfs->volumeHeader->blockSize;
}
