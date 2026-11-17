//
//  RPObj.hpp
//  ramdisk_patcher_ra1nsn0w_plugin
//
//  Created by tihmstar on 17.11.26.
//

#ifndef RPObj_hpp
#define RPObj_hpp

#include <stdint.h>
#include <ra1nsn0w/ra1nsn0w_plugins.hpp>

#include <iostream>
#include <vector>

class RPObj : public tihmstar::ra1nsn0w::PluginObj {
    enum RDActionType{
        kRDActionTypeInvalid = 0,
        kRDActionTypeDelete,
        kRDActionTypePut,
    };
    struct RDAction{
        RDActionType type;
        std::string path;
        std::vector<std::string> args;
    };
    
    bool _haveAnyRamdiskPatchesEnabled;
    bool _doRPatchASRSigcheck;
    bool _doRPatchASRHardcodepw;
    bool _doRPatchLibIMG4Sigcheck;
    bool _doRPatchRestoredSigcheck;
    bool _doRPatchDeleteNVMEFW;
    std::string _platform;
    
    std::vector<RDAction> _actions;
public:
    RPObj();
    ~RPObj();
    
    virtual bool argparse(std::string longopt, const char *optarg) override;
    virtual std::vector<tihmstar::patchfinder::patch> patcher(uint32_t component, const void *buf, size_t bufSize) override;
    virtual bool wantsPatchingRamdisk() override;
    virtual tihmstar::Mem ramdiskPatcher(const void *buf, size_t bufSize) override;
};

#endif /* RPObj_hpp */
