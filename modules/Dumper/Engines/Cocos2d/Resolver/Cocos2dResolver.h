#pragma once
// Cocos2d — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver resolves engine symbols via live process (if needed).
// For Cocos2d-x Lua variant: may need XXTEA key extraction from native lib.
// For Cocos Creator: may need asset manager resolution.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::cocos2d {

class Cocos2dResolver {
public:
    // Resolve Cocos2d engine symbols via live process.
    //
    // Flow: profile->symbolFor(key) -> return mangled/demangled name ->
    //       SymbolResolver (xDL / dlsym) -> resolve live address in target process.
    //
    // Unlike UE (patternFor -> AOB scan -> RIP-relative), Cocos2d uses
    // direct symbol export lookup. offsetOf()/structSize() return 0 and
    // MUST NOT be read by this resolver — symbolFor() is the only path.
    //
    // Example for Lua variant:
    //   auto name = profile->symbolFor("luaL_loadbuffer");
    //   auto addr = xdl_sym(targetHandle, name->c_str());
    //   // addr is now the live address of luaL_loadbuffer in target process
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::cocos2d
