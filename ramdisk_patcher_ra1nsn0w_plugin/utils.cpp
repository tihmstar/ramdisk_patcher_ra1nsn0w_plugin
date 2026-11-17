//
//  utils.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#include "utils.hpp"
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <libgeneral/macros.h>
#include <libgeneral/ByteOrder.hpp>
#include <libgeneral/Mem.hpp>
#include <libgeneral/Utils.hpp>

#include <openssl/sha.h>

#ifdef XCODE
static void DumpHex(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}
#endif

std::string utils::getTmpdir(){
    static std::string tmpdir;
    static bool tmpdirIsInited = false;
    struct stat st = {};
    //make sure tmpdir was created *and* actually exists
    if (!tmpdirIsInited || stat(tmpdir.c_str(), &st)) {
        tmpdir.clear();
        tmpdirIsInited = true;
        
#ifdef WIN32
        {
            tmpdir.resize(MAX_PATH);
            DWORD tmpdirlen = 0;
            retassure(tmpdirlen = GetTempPathA((DWORD)tmpdir.size(),(char*)tmpdir.data()), "Failed to get tmpdir");
            tmpdir.resize(tmpdirlen);
        }
#else
        //seems like TMPDIR env var is set on macOS app sandbox, but not on linux
        if (const char *e = getenv("TMPDIR")) {
            tmpdir = e;
        }else{
#ifndef WIN32
            tmpdir = "/tmp/";
#endif
        }
#endif
        if (tmpdir.back() != PATH_DELIMITER_CHAR) tmpdir.push_back(PATH_DELIMITER_CHAR);
        tmpdir += PACKAGE_NAME ".XXXXXX";
        
        //create tmpdir
#ifdef XCODE
        retassure(!mkdir(tmpdir.data(), 0700) || errno == EEXIST,"Failed to mkdir");
#elif defined(HAVE_MKDTEMP)
        retassure(mkdtemp((char*)tmpdir.data()),"Failed to make tmpdir");
#elif defined(WIN32)
        {
            for (int i=0; i<10; i++) {
                int r = rand();
                int sel = rand() % 3;
                if (sel == 0){
                    tmpdir.push_back('0'+(r%10));
                }else if (sel == 1){
                    tmpdir.push_back('a'+(r%('z'-'a')));
                }else{
                    tmpdir.push_back('A'+(r%('z'-'a')));
                }
            }
            retassure(!mkdir(tmpdir.data()) || errno == EEXIST,"Failed to mkdir");
        }
#else
#   error unavailable tmpdir implementation
#endif
        if (tmpdir.back() != PATH_DELIMITER_CHAR) tmpdir.push_back(PATH_DELIMITER_CHAR);
        debug("tmpdir=%s",tmpdir.c_str());
    }
    return tmpdir;
}

std::string utils::genpassp(const char *platform, const void *ramdiskBuf, size_t ramdiskSize){
    uint8_t platformHash[SHA_DIGEST_LENGTH] = {};
    uint64_t magic[4] = {
        0xAD79D29DE5E2AC9EULL,
        0xE6AF2EB19E23925BULL,
        0x3F1375B4BD88815CULL,
        0x3BDFF4E5564A9F87ULL,
    };
    uint32_t salt[4] = {};
    SHA256_CTX ctx = {};
    
    SHA1((const uint8_t*)platform, strlen(platform), platformHash);
    
    {
        uint64_t pepper = cnvBETOH(*(uint64_t*)platformHash);
        magic[0] += pepper;
        magic[1] += pepper;
        magic[2] += pepper;
        magic[3] += pepper;
    }
    
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, magic, sizeof(magic));
    
    for (int i=0; i<ARRAYOF(salt); i++) {
        salt[i] = (magic[i] % ramdiskSize) & 0xFFFFFE00;
    }
    
    qsort(salt, 4, 4, [](const void *a, const void *b)->int{
        return (*(uint32_t*)a) > (*(uint32_t*)b);
    });
    
    {
        bool move = true;
        for (int i=0; i<ARRAYOF(salt); i++) {
            if (move) magic[i] = salt[i];
            if (i+1< ARRAYOF(salt)){
                uint64_t diff = salt[i+1] - salt[i];
                if (diff < 0x4000) {
                    magic[i] += diff<<32;
                    move = false;
                    continue;
                }
            }
            uint64_t left = ramdiskSize - salt[i];
            if (left > 0x4000) left = 0x4000;
            magic[i] += left<<32;
            move = true;
        }
        if (ramdiskSize - salt[3] < 0x4000){
            memmove(&magic[1], &magic[0], sizeof(magic) - sizeof(*magic));
            magic[0] = ((uint64_t)(salt[3] + 0x4000 - ramdiskSize)) << 32;
        }
    }
    
    {
        uint32_t *v54 = (uint32_t*)magic;
        uint64_t ramdiskSizeInBlocks = ramdiskSize >> 9;
        int j = 0;
        uint64_t chunkInBlocks = 0;
        for (uint32_t offsetInBlocks = 0; offsetInBlocks < ramdiskSizeInBlocks; offsetInBlocks += chunkInBlocks) {
            chunkInBlocks = ramdiskSizeInBlocks - offsetInBlocks;
            if (chunkInBlocks > 2048) chunkInBlocks = 2048;

            uint64_t offsetInBytes = offsetInBlocks << 9;
            const uint8_t *buf = ((const uint8_t*)ramdiskBuf) + offsetInBytes;
            SHA256_Update(&ctx, buf, chunkInBlocks << 9);

            uint64_t chunkEnd = (chunkInBlocks << 9) + offsetInBytes;

            for (int i = j; i < 4; i++) {
                uint64_t offset = v54[2 * i];
                uint64_t len = v54[2 * i + 1] + offset;
                if (offsetInBytes >= len) {
                    j = i + 1;
                } else {
                    if (chunkEnd <= offset) break;
                    if (offsetInBytes > offset) offset = offsetInBytes;
                    if (chunkEnd < len) len = (chunkInBlocks << 9) + offsetInBytes;
                    SHA256_Update(&ctx, buf + offset - offsetInBytes, len - offset);
                }
            }
        }
    }
    {
        char hashstr[65] = {};
        size_t hashstrSize = sizeof(hashstr);
        uint8_t digest[32] = {};
        SHA256_Final(digest, &ctx);
        tihmstar::bytesToHex(digest, sizeof(digest), hashstr, &hashstrSize);
        return hashstr;
    }
}
