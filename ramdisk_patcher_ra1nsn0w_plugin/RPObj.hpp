//
//  RPObj.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 08.11.26.
//

#ifndef RPObj_hpp
#define RPObj_hpp

#include <stdint.h>
#include <ra1nsn0w/ra1nsn0w_plugins.hpp>

class RPObj : public tihmstar::ra1nsn0w::PluginObj {
    bool _haveAnyRamdiskPatchesEnabled;
    bool _doRPatchASRSigchek;
    const char *_replaceReboot;
public:
    RPObj();
    virtual ~RPObj() override;
    
    virtual bool argparse(std::string longopt, const char *optarg) override;
    virtual std::vector<tihmstar::patchfinder::patch> patcher(uint32_t component, const void *buf, size_t bufSize) override;
    virtual bool wantsPatchingRamdisk() override;
    virtual tihmstar::Mem ramdiskPatcher(const void *buf, size_t bufSize) override;
};

#endif /* RPObj_hpp */
