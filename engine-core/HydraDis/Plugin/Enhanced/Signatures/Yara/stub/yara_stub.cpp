// Stub for Yara plugin when libyara is not available.
// Provides the factory function so linking succeeds.

#include "Plugin/IPlugin.h"
#include <memory>

namespace omnibyte::hydradis::plugin {

class YaraStubPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/Yara"; }
    std::string version() const override { return "2.0.0-stub"; }

    bool onLoad() override { return false; }

    PluginResult onRun(const PluginContext&) override {
        PluginResult result;
        result.errorMessage = "YARA not available (libyara not linked)";
        return result;
    }

    void onUnload() override {}
};

extern "C" std::unique_ptr<IPlugin> create_yara_plugin() {
    return std::make_unique<YaraStubPlugin>();
}

extern "C" int yara_plugin_init() { return 0; }

} // namespace omnibyte::hydradis::plugin
