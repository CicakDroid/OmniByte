#include "ExportRegistry.h"
#include "../../Writers/CSharpWriter/CSharpWriter.h"
#include "../../Writers/JsonWriter/JsonWriter.h"
#include "../../Writers/DummyDllWriter/DummyDllWriter.h"
#include "../../Writers/HeaderWriter/HeaderWriter.h"

namespace omnibyte::dumper::export_core {

void ExportRegistry::registerDefaults() {
    registerExporter("csharp",   std::make_shared<writers::CSharpWriter>());
    registerExporter("json",     std::make_shared<writers::JsonWriter>());
    registerExporter("dummydll", std::make_shared<writers::DummyDllWriter>());
    registerExporter("header",   std::make_shared<writers::HeaderWriter>());
}

} // namespace omnibyte::dumper::export_core
