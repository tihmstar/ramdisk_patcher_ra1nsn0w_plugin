//
//  utils.cpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#include "utils.hpp"
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <libgeneral/macros.h>

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
