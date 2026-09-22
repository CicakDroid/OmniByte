#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Analysis/Confidences.h"
#include "Analysis/Exports.h"
#include "Analysis/Imports.h"
#include "Analysis/Strings.h"

using namespace omnibyte::hydradis;

static int passed = 0;
static int failed = 0;

#define TEST(name) \
    do { printf("  %-50s ", #name); } while(0)

#define PASS() \
    do { printf("[PASS]\n"); passed++; } while(0)

#define FAIL(msg) \
    do { printf("[FAIL] %s\n", msg); failed++; } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { FAIL(#cond); return; } } while(0)

#define ASSERT_EQ(a, b) \
    do { if ((a) != (b)) { FAIL(#a " == " #b); return; } } while(0)

#define ASSERT_FALSE(cond) \
    do { if (cond) { FAIL("!(" #cond ")"); return; } } while(0)

// ── Confidences ──────────────────────────────────────────────────────

static void test_confidences_empty() {
    TEST(confidences_empty_input);
    Confidences c;
    std::vector<uint8_t> empty;
    auto result = c.analyzeConfidences(empty);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.overallConfidence, 0.0);
    PASS();
}

static void test_confidences_name() {
    TEST(confidences_name);
    Confidences c;
    ASSERT_EQ(c.name(), std::string("Confidences"));
    PASS();
}

static void test_confidences_code_data() {
    TEST(confidences_arm64_code);
    Confidences c;
    std::vector<uint8_t> code = {
        0xFF, 0x03, 0x00, 0xD1,  // sub sp, sp, #0x3f
        0xFD, 0x7B, 0xC1, 0xA9,  // stp x29, x30, [sp, #-16]!
        0xF3, 0x03, 0x00, 0xAA,  // mov x19, x0
        0x1F, 0x20, 0x03, 0xD5,  // nop
    };
    auto result = c.analyzeConfidences(code);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.overallConfidence >= 0.0);
    ASSERT_TRUE(result.overallConfidence <= 1.0);
    ASSERT_TRUE(result.categoryConfidence.count("instructions"));
    ASSERT_TRUE(result.categoryConfidence.count("entropy"));
    ASSERT_TRUE(result.categoryConfidence.count("patterns"));
    PASS();
}

// ── Strings ──────────────────────────────────────────────────────────

static void test_strings_empty() {
    TEST(strings_empty_input);
    Strings s;
    auto result = s.analyzeStrings(nullptr, 0);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.classifiedStrings, static_cast<size_t>(0));
    PASS();
}

static void test_strings_name() {
    TEST(strings_name);
    Strings s;
    ASSERT_EQ(s.name(), std::string("Strings"));
    PASS();
}

static void test_strings_extract_basic() {
    TEST(strings_extract_basic);
    Strings s;
    const char* data = "hello world\x00\x00short\x00"
                       "http://example.com/path\x00"
                       "%s format string\x00"
                       "0xDEADBEEF\x00";
    auto result = s.analyzeStrings(
        reinterpret_cast<const uint8_t*>(data),
        std::strlen(data)
    );
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.classifiedStrings >= 3);
    bool foundUrl = false, foundFormat = false, foundHex = false;
    for (auto& [addr, cat] : result.stringClassifications) {
        if (cat == "url") foundUrl = true;
        if (cat == "format") foundFormat = true;
        if (cat == "hex_constant") foundHex = true;
    }
    ASSERT_TRUE(foundUrl);
    ASSERT_TRUE(foundFormat);
    ASSERT_TRUE(foundHex);
    PASS();
}

static void test_strings_min_length() {
    TEST(strings_respects_min_length);
    Strings s;
    const char* data = "ab\x00\x00";
    auto result = s.analyzeStrings(
        reinterpret_cast<const uint8_t*>(data), 2
    );
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.classifiedStrings, static_cast<size_t>(0));
    PASS();
}

static void test_strings_classify_numeric() {
    TEST(strings_classify_numeric);
    Strings s;
    const char* data = "12345678\x00";
    auto result = s.analyzeStrings(
        reinterpret_cast<const uint8_t*>(data), std::strlen(data)
    );
    ASSERT_TRUE(result.success);
    bool foundNumeric = false;
    for (auto& [addr, cat] : result.stringClassifications) {
        if (cat == "numeric_constant") foundNumeric = true;
    }
    ASSERT_TRUE(foundNumeric);
    PASS();
}

static void test_strings_classify_identifier() {
    TEST(strings_classify_identifier);
    Strings s;
    const char* data = "mainActivity\x00";
    auto result = s.analyzeStrings(
        reinterpret_cast<const uint8_t*>(data), std::strlen(data)
    );
    ASSERT_TRUE(result.success);
    bool foundIdentifier = false;
    for (auto& [addr, cat] : result.stringClassifications) {
        if (cat == "identifier") foundIdentifier = true;
    }
    ASSERT_TRUE(foundIdentifier);
    PASS();
}

static void test_strings_classify_path() {
    TEST(strings_classify_path);
    Strings s;
    const char* data = "/data/app/com.example/base.apk\x00";
    auto result = s.analyzeStrings(
        reinterpret_cast<const uint8_t*>(data), std::strlen(data)
    );
    ASSERT_TRUE(result.success);
    bool foundPath = false;
    for (auto& [addr, cat] : result.stringClassifications) {
        if (cat == "path") foundPath = true;
    }
    ASSERT_TRUE(foundPath);
    PASS();
}

// ── Imports ──────────────────────────────────────────────────────────

static void test_imports_empty() {
    TEST(imports_empty_input);
    Imports imp;
    std::vector<uint8_t> empty;
    std::vector<SymbolInfo> noSyms;
    auto result = imp.analyzeImports(0x1000, empty, noSyms);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.totalImports, static_cast<size_t>(0));
    PASS();
}

static void test_imports_name() {
    TEST(imports_name);
    Imports imp;
    ASSERT_EQ(imp.name(), std::string("Imports"));
    PASS();
}

static void test_imports_from_symbols() {
    TEST(imports_from_symbols);
    Imports imp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"strcmp", 0x0, 0, 2, 1, 0},
        {"malloc", 0x0, 0, 2, 1, 0},
        {"strlen", 0x0, 0, 2, 2, 0},
    };
    symbols[0].type = 2; symbols[0].binding = 1;
    symbols[1].type = 2; symbols[1].binding = 1;
    symbols[2].type = 2; symbols[2].binding = 2;

    auto result = imp.analyzeImports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.totalImports >= 2);
    bool foundWeak = false;
    for (auto& i : result.imports) {
        if (i.isWeak) foundWeak = true;
    }
    ASSERT_TRUE(foundWeak);
    PASS();
}

static void test_imports_plt_detection() {
    TEST(imports_plt_detection);
    Imports imp;
    std::vector<SymbolInfo> noSyms;

    uint32_t ldrX16 = 0xF9400210;
    uint32_t brX16  = 0xD61F0200;
    std::vector<uint8_t> code(sizeof(uint32_t) * 2);
    std::memcpy(code.data(), &ldrX16, 4);
    std::memcpy(code.data() + 4, &brX16, 4);

    auto result = imp.analyzeImports(0x2000, code, noSyms);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.totalImports >= 1);
    ASSERT_EQ(result.imports[0].source, ImportDetectionSource::PLTStub);
    ASSERT_EQ(result.imports[0].type, ImportType::Function);
    PASS();
}

// ── Exports ──────────────────────────────────────────────────────────

static void test_exports_empty() {
    TEST(exports_empty_input);
    Exports exp;
    std::vector<uint8_t> empty;
    std::vector<SymbolInfo> noSyms;
    auto result = exp.analyzeExports(0x1000, empty, noSyms);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.totalExports, static_cast<size_t>(0));
    PASS();
}

static void test_exports_name() {
    TEST(exports_name);
    Exports exp;
    ASSERT_EQ(exp.name(), std::string("Exports"));
    PASS();
}

static void test_exports_from_symbols() {
    TEST(exports_from_symbols);
    Exports exp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"main", 0x1000, 0x80, 2, 1, 1},
        {"init", 0x1080, 0x40, 2, 1, 1},
        {"data_obj", 0x2000, 0x10, 1, 1, 1},
        {"_Z7helperv", 0x10C0, 0x30, 2, 1, 1},
    };

    auto result = exp.analyzeExports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.totalExports, static_cast<size_t>(4));
    ASSERT_EQ(result.functionExports, static_cast<size_t>(3));
    ASSERT_EQ(result.objectExports, static_cast<size_t>(1));
    PASS();
}

static void test_exports_filters_internal() {
    TEST(exports_filters_internal_symbols);
    Exports exp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"", 0x1000, 0, 2, 1, 1},
        {".", 0x1080, 0, 2, 1, 1},
        {"..", 0x10C0, 0, 2, 1, 1},
        {"valid_export", 0x1100, 0x40, 2, 1, 1},
    };

    auto result = exp.analyzeExports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.totalExports, static_cast<size_t>(1));
    ASSERT_EQ(result.exports[0].name, std::string("valid_export"));
    PASS();
}

static void test_exports_weak_symbols() {
    TEST(exports_weak_symbols);
    Exports exp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"weak_func", 0x1000, 0x20, 2, 2, 1},
        {"strong_func", 0x1020, 0x20, 2, 1, 1},
    };

    auto result = exp.analyzeExports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.weakExports >= 1);
    bool foundWeak = false;
    for (auto& e : result.exports) {
        if (e.name == "weak_func" && e.isWeak) foundWeak = true;
    }
    ASSERT_TRUE(foundWeak);
    PASS();
}

static void test_exports_demangle() {
    TEST(exports_demangle_cpp);
    Exports exp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"_ZN3foo3barEv", 0x1000, 0x10, 2, 1, 1},
    };

    auto result = exp.analyzeExports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.totalExports, static_cast<size_t>(1));
    ASSERT_EQ(result.exports[0].demangledName, std::string("foo::bar"));
    PASS();
}

static void test_exports_dynamic_detection() {
    TEST(exports_dynamic_adrp_add);
    Exports exp;
    std::vector<SymbolInfo> noSyms;

    uint32_t adrpX0 = 0x90000000 | (0x1000 << 5) | 0x00;
    uint32_t addX0  = 0x91000000 | (0x00 << 5) | 0x00;
    std::vector<uint8_t> code(sizeof(uint32_t) * 2);
    std::memcpy(code.data(), &adrpX0, 4);
    std::memcpy(code.data() + 4, &addX0, 4);

    auto result = exp.analyzeExports(0x1000, code, noSyms);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.totalExports >= 1);
    ASSERT_EQ(result.exports[0].source, ExportDetectionSource::DynamicTag);
    PASS();
}

static void test_exports_address_to_name() {
    TEST(exports_address_to_name_map);
    Exports exp;
    std::vector<uint8_t> empty;

    std::vector<SymbolInfo> symbols = {
        {"func_a", 0x1000, 0x20, 2, 1, 1},
        {"func_b", 0x1020, 0x20, 2, 1, 1},
    };

    auto result = exp.analyzeExports(0x1000, empty, symbols);
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.addressToName.count(0x1000));
    ASSERT_TRUE(result.addressToName.count(0x1020));
    ASSERT_EQ(result.addressToName[0x1000], std::string("func_a"));
    ASSERT_EQ(result.addressToName[0x1020], std::string("func_b"));
    PASS();
}

// ── IAnalysis default stubs ─────────────────────────────────────────

static void test_ianalysis_defaults() {
    TEST(ianalysis_default_stubs);
    Confidences c;
    std::vector<uint8_t> empty;
    std::vector<uint64_t> emptyU64;
    std::vector<std::string> emptyStr;
    std::vector<std::pair<uint64_t, uint64_t>> emptyPairs;
    std::vector<SymbolInfo> noSyms;

    StructureResult sr1 = c.buildTrie(emptyStr);
    ASSERT_FALSE(sr1.success);

    StructureResult sr2 = c.buildBloomFilter(nullptr, 0, 0, 0.01);
    ASSERT_FALSE(sr2.success);

    StructureResult sr3 = c.computeEquivalenceClasses(emptyPairs);
    ASSERT_FALSE(sr3.success);

    StackResult stk = c.constantFold(empty);
    ASSERT_FALSE(stk.success);

    ListResult lr = c.dfsTraversal(0, empty);
    ASSERT_FALSE(lr.success);

    TreeResult tr = c.computeDominatorTree(0, empty);
    ASSERT_FALSE(tr.success);

    PackersResult pr = c.detectPacker(nullptr, 0);
    ASSERT_FALSE(pr.success);

    ObfuscateResult ob = c.analyzeObfuscation(nullptr, 0);
    ASSERT_FALSE(ob.success);

    TaintResult ta = c.analyzeTaint(0, empty, emptyU64);
    ASSERT_FALSE(ta.success);

    FunctionsResult fn = c.analyzeFunctions(0, empty);
    ASSERT_FALSE(fn.success);

    VariablesResult vr = c.analyzeVariables(0, empty);
    ASSERT_FALSE(vr.success);

    ParametersResult pa = c.analyzeParameters(0, empty);
    ASSERT_FALSE(pa.success);

    TypesResult ty = c.analyzeTypes(0, empty);
    ASSERT_FALSE(ty.success);

    ConfidencesResult cr = c.analyzeConfidences(empty);
    ASSERT_FALSE(cr.success);

    StringsResult st = c.analyzeStrings(nullptr, 0);
    ASSERT_FALSE(st.success);

    ExportsResult ex = c.analyzeExports(0, empty, noSyms);
    ASSERT_FALSE(ex.success);

    PASS();
}

// ── Main ─────────────────────────────────────────────────────────────

int main() {
    printf("\n=== Hydra2D Analysis Unit Tests ===\n\n");

    printf("[Confidences]\n");
    test_confidences_empty();
    test_confidences_name();
    test_confidences_code_data();

    printf("\n[Strings]\n");
    test_strings_empty();
    test_strings_name();
    test_strings_extract_basic();
    test_strings_min_length();
    test_strings_classify_numeric();
    test_strings_classify_identifier();
    test_strings_classify_path();

    printf("\n[Imports]\n");
    test_imports_empty();
    test_imports_name();
    test_imports_from_symbols();
    test_imports_plt_detection();

    printf("\n[Exports]\n");
    test_exports_empty();
    test_exports_name();
    test_exports_from_symbols();
    test_exports_filters_internal();
    test_exports_weak_symbols();
    test_exports_demangle();
    test_exports_dynamic_detection();
    test_exports_address_to_name();

    printf("\n[IAnalysis Defaults]\n");
    test_ianalysis_defaults();

    printf("\n---\n");
    printf("Results: %d passed, %d failed, %d total\n\n", passed, failed, passed + failed);

    return failed > 0 ? 1 : 0;
}
