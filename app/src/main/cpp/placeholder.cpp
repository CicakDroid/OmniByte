// OmniByte native placeholder.
//
// Stage 1 scaffolding only: no business logic. This translation unit exists so
// the aggregated `omnibyte` shared library has a non-empty source and links a
// loadable .so. Real engine-core / runtime / dumper entry points are added in
// later stages.
extern "C" int omnibyte_placeholder_init() {
    return 0;
}
