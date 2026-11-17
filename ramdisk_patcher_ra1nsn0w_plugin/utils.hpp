//
//  utils.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#ifndef utils_hpp
#define utils_hpp

#include <iostream>

#ifdef WIN32
#define PATH_DELIMITER_STRING "\\"
#define PATH_DELIMITER_CHAR '\\'
#else
#define PATH_DELIMITER_STRING "/"
#define PATH_DELIMITER_CHAR '/'
#endif

namespace utils {
std::string getTmpdir();
std::string genpassp(const char *platform, const void *ramdiskBuf, size_t ramdiskSize);
}

#endif /* utils_hpp */
