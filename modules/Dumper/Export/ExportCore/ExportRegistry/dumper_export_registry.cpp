// ExportCore/ExportRegistry — Registry singleton implementation.
#include "ExportRegistry.h"

namespace omnibyte::dumper::export_core {

// Static registration of default exporters will be handled by the init function
extern "C" int dumper_export_registry_init() {
    // Registry is auto-initialized via static singleton
    return 0;
}

} // namespace omnibyte::dumper::export_core
