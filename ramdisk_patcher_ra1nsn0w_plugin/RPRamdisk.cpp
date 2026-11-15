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
    locbuf = malloc(bufSize);
    memcpy(locbuf, buf, bufSize);
    af = createAbstractFileFromMemory(&locbuf, bufSize);
    add_hfs(_hfs, af, path); af = NULL;
    chmodFile(path, 0777, _hfs);
}

void RPRamdisk::removeFileHFS(const char *path){
    ::removeFile(path, _hfs);
}

std::vector<std::string> RPRamdisk::listFilesAtPathHFS(const char *path, bool recursive){
    HFSPlusCatalogRecord *record = NULL;
    cleanup([&]{
        safeFree(record);
    });
    char* name = NULL; //dont't free this? idk
    std::vector<std::string> ret;
    assure(record = getRecordFromPath(path, _hfs, &name, NULL));
    
    if(record != NULL) {
        if(record->recordType == kHFSPlusFolderRecord){
            CatalogRecordList* theList = NULL;
            cleanup([&]{
                safeFreeCustom(theList, releaseCatalogRecordList);
            });
            std::string self = path;
            if (self.back() != '/') self += '/';
            ret.push_back(self);
            for (CatalogRecordList* list = (theList = getFolderContents(((HFSPlusCatalogFolder*)record)->folderID, _hfs)); list; list = list->next) {
                char *curname = NULL;
                cleanup([&]{
                    safeFree(curname);
                });
                curname = unicodeToAscii(&list->name);
                std::string cur = self + curname;
                if (strncmp(curname, ".HFS+ Private Directory Data", sizeof(".HFS+ Private Directory Data")-1) == 0) continue;
                if (!strlen(curname)) continue;
                if(list->record->recordType == kHFSPlusFolderRecord) {
                    auto retlist = listFilesAtPathHFS(cur.c_str(), recursive);
                    ret.insert(ret.end(), retlist.begin(), retlist.end());
                } else if(list->record->recordType == kHFSPlusFileRecord) {
                    ret.push_back(cur);
                }
            }
        }else{
            return {path};
        }
    } else {
        return {path};
    }
    return ret;
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

void RPRamdisk::removeFile(const char *path){
    return removeFileHFS(path);
    reterror("No ramdisk backend available for file deleting");
}

std::vector<std::string> RPRamdisk::listFilesAtPath(const char *path, bool recursive){
    return listFilesAtPathHFS(path, recursive);
    reterror("No ramdisk backend available for file listing");
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
