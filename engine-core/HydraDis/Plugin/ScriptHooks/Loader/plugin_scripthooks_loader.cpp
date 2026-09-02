#include "Plugin/IPlugin.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class ScriptHooksLoaderPlugin : public IPlugin {
public:
    std::string name() const override { return "ScriptHooks/Loader"; }
    std::string version() const override { return "0.1.0-dev"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;
        result.success = false;
        result.errorMessage = "ScriptHooks/Loader: script loading and validation not yet implemented";
        return result;
    }

    void onUnload() override {}
};

extern "C" std::unique_ptr<IPlugin> create_scripthooks_loader_plugin() {
    return std::make_unique<ScriptHooksLoaderPlugin>();
}

extern "C" int plugin_scripthooks_loader_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin
