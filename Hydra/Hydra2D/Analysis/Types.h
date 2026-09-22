#pragma once
// Types — RTTI and type recovery for C++ binaries.
// Recovers class hierarchy, virtual method tables (vtables), and inheritance
// chains from Itanium ABI symbols (_ZTV*, _ZTI*, _ZTS*) and binary structure.

#include "IAnalysis.h"
#include "Parser/IParser.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// A single virtual method entry in a vtable.
struct VirtualMethod {
    uint64_t offset = 0;            // offset within vtable (bytes from start)
    uint64_t targetAddr = 0;        // address of the method implementation
    std::string name;               // resolved method name (if symbol available)
    std::string mangledName;        // mangled name from symbol
    bool isPureVirtual = false;     // null entry = pure virtual
    bool isDestructor = false;      // destructor marker
};

/// Full recovered type information for a class.
struct TypeInfo {
    std::string className;          // demangled class name
    std::string mangledName;        // mangled name (_ZTV, _ZTI, _ZTS)
    uint64_t vtableAddr = 0;       // vtable address
    uint64_t typeinfoAddr = 0;     // _ZTI* address (type_info pointer)
    uint64_t typeinfoNameAddr = 0; // _ZTS* address (type_info name string)
    std::vector<std::string> baseClasses;  // inheritance chain (ordered)
    std::vector<VirtualMethod> virtualMethods;  // recovered methods
    size_t vtableSize = 0;         // total vtable entries
    size_t methodCount = 0;        // non-null method entries
    bool hasRTTI = false;          // has _ZTI*/_ZTS* symbols
    bool isAbstract = false;       // has pure virtual methods
    bool isPolymorphic = false;    // has vtable at all
};

/// Class hierarchy graph node.
struct ClassNode {
    std::string className;
    std::vector<std::string> baseClasses;   // direct parents
    std::vector<std::string> derivedClasses; // direct children
    bool hasVtable = false;
    bool hasRTTI = false;
};

/// Complete recovery result.
struct RecoveryResult {
    bool success = false;
    std::string errorMessage;
    std::vector<TypeInfo> types;
    std::unordered_map<std::string, ClassNode> hierarchy;  // className → node
    size_t totalClasses = 0;
    size_t totalMethods = 0;
    size_t totalHierarchyLinks = 0;
};

class Types : public IAnalysis {
public:
    std::string name() const override { return "Types"; }

    TypesResult analyzeTypes(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    VtablesResult analyzeVtables(
        const std::vector<SymbolInfo>& symbols,
        const std::vector<SectionInfo>& sections
    ) const;

    /// Full type recovery: hierarchy, virtual methods, RTTI.
    RecoveryResult recoverTypes(
        const std::vector<SymbolInfo>& symbols,
        const std::vector<SectionInfo>& sections
    ) const;

private:
    std::string inferType(uint32_t instruction) const;
    bool isPointerDereference(uint32_t instruction) const;
    bool isStructAccess(uint32_t instruction) const;
    bool isArrayAccess(uint32_t instruction) const;

    /// Recover virtual methods from vtable entries in section data.
    std::vector<VirtualMethod> recoverVirtualMethods(
        uint64_t vtableAddr,
        size_t entryCount,
        const std::vector<uint8_t>& sectionData,
        uint64_t sectionBaseAddr,
        const std::vector<SymbolInfo>& symbols
    ) const;

    /// Resolve inheritance chain from _ZTI* typeinfo symbols.
    std::vector<std::string> resolveInheritanceChain(
        const std::string& mangledTypeinfo,
        const std::vector<SymbolInfo>& symbols
    ) const;

    /// Build class hierarchy graph from recovered types.
    void buildClassHierarchy(
        const std::vector<TypeInfo>& types,
        std::unordered_map<std::string, ClassNode>& hierarchy
    ) const;

    static std::string extractClassNameFromVtable(const std::string& mangled);
    static std::string extractClassNameFromTypeinfo(const std::string& mangled);
    static std::vector<std::string> parseBaseClasses(const std::string& mangled);
    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis
