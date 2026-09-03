// YaraScan — YARA rule scanning engine.
// Optional plugin: requires libyara (https://github.com/VirusTotal/libyara).
// License: GPL-3.0 (libyara)

#include "YaraScan.h"
#include <cstring>
#include <fstream>

#ifdef HAS_YARA
#include <yara.h>

struct omnibyte::deob::YaraScanEngine::Impl {
    YR_COMPILER* compiler = nullptr;
    YR_RULES* rules = nullptr;
    bool initialized = false;

    ~Impl() {
        if (rules) yr_rules_destroy(rules);
        if (compiler) yr_compiler_destroy(compiler);
    }
};

omnibyte::deob::YaraScanEngine::~YaraScanEngine() {
    delete impl_;
}

bool omnibyte::deob::YaraScanEngine::initialize() {
    if (impl_ && impl_->initialized) return true;

    if (yr_initialize() != ERROR_SUCCESS) return false;

    impl_ = new Impl();
    if (yr_compiler_create(&impl_->compiler) != ERROR_SUCCESS) {
        delete impl_;
        impl_ = nullptr;
        return false;
    }

    impl_->initialized = true;
    return true;
}

bool omnibyte::deob::YaraScanEngine::addRules(const std::string& rulesText) {
    if (!impl_ || !impl_->compiler) return false;

    int errors = yr_compiler_add_string(impl_->compiler, rulesText.c_str(), nullptr);
    if (errors > 0) return false;

    if (impl_->rules) yr_rules_destroy(impl_->rules);
    impl_->rules = nullptr;

    return yr_compiler_get_rules(impl_->compiler, &impl_->rules) == ERROR_SUCCESS;
}

static int yara_callback(YR_MATCH* match, void* user_data) {
    auto* hits = static_cast<std::vector<omnibyte::deob::YaraHit>*>(user_data);
    omnibyte::deob::YaraHit hit;
    hit.offset = static_cast<uintptr_t>(match->base + match->offset);
    hit.ruleName = match->rule->identifier;
    hit.confidence = 0.85;

    if (match->meta) {
        for (YR_META* meta = match->meta; meta->identifier; ++meta) {
            if (meta->type == META_TYPE_STRING) {
                hit.meta = meta->string;
                break;
            }
        }
    }

    hits->push_back(hit);
    return CALLBACK_CONTINUE;
}

std::vector<omnibyte::deob::YaraHit> omnibyte::deob::YaraScanEngine::scanRegion(const uint8_t* data, size_t size) const {
    std::vector<YaraHit> hits;
    if (!impl_ || !impl_->rules) return hits;

    yr_rules_scan_mem(impl_->rules, data, size, 0, yara_callback, &hits, nullptr);
    return hits;
}

std::vector<omnibyte::deob::YaraHit> omnibyte::deob::YaraScanEngine::scanFile(const std::string& filePath) const {
    std::vector<YaraHit> hits;
    if (!impl_ || !impl_->rules) return hits;

    yr_rules_scan_file(impl_->rules, filePath.c_str(), 0, yara_callback, &hits, nullptr);
    return hits;
}

bool omnibyte::deob::YaraScanEngine::isInitialized() const {
    return impl_ && impl_->initialized;
}

#else

// Stub implementation when libyara is not available
struct omnibyte::deob::YaraScanEngine::Impl {};

omnibyte::deob::YaraScanEngine::~YaraScanEngine() { delete impl_; }

bool omnibyte::deob::YaraScanEngine::initialize() {
    impl_ = new Impl();
    return false;
}

bool omnibyte::deob::YaraScanEngine::addRules(const std::string&) {
    return false;
}

std::vector<omnibyte::deob::YaraHit> omnibyte::deob::YaraScanEngine::scanRegion(const uint8_t*, size_t) const {
    return {};
}

std::vector<omnibyte::deob::YaraHit> omnibyte::deob::YaraScanEngine::scanFile(const std::string&) const {
    return {};
}

bool omnibyte::deob::YaraScanEngine::isInitialized() const {
    return false;
}

#endif
