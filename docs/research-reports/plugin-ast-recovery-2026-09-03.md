# Research Report: Abstract Syntax Tree Recovery from Binary

**Date**: 2026-09-03
**Plugin**: Enhanced/AST (enhanced_ast.cpp)
**Status**: STUB — returns false, "not yet implemented"

---

## 1. Current State

The `Enhanced/AST` plugin is a 29-line stub that returns `result.success = false`. It implements the `IPlugin` interface but performs no analysis.

### What AST Recovery Means

AST recovery from binary = reconstructing a tree representation of the program's logic from disassembled instructions. This is NOT full decompilation to C source — it's building an intermediate tree structure that captures:
- Control flow constructs (loops, if/else, switch)
- Expression trees (arithmetic, logical operations)
- Function call graphs
- Variable assignments

### Data Available from PluginContext

```
PluginContext provides:
- binary: ParsedBinary (sections, symbols, header)
- disassemblyResults: vector<DisassemblyResult> (instructions with address, size, mnemonic, opStr, bytes)
- decompilationResults: vector<DecompiledFunction> (pseudocode strings)
```

The plugin has access to disassembled instructions (mnemonic + operands) and optionally decompiled pseudocode.

---

## 2. Research Sources

### Source 1: SALT4Decompile (2025)
- **Paper**: "SALT4Decompile: Decompile with Source code-level Abstract Logic Tree"
- **URL**: https://arxiv.org/pdf/2509.14646
- **Key Insight**: Builds a "Source code-level Abstract Logic Tree" (SALT) from assembly
- **Algorithm**:
  1. CFG extraction from binary
  2. Instruction normalization (absolute→relative addresses)
  3. Jumping unit identification (loop detection via SCC in CFG)
  4. Tree construction (recursive, following CFG edges)
- **Implementation**: Python, ~2000 LOC, uses angr for CFG extraction
- **Relevance**: HIGH — the SALT tree structure is exactly what our AST plugin should produce

### Source 2: Ahoy SAILR (USENIX Security 2024)
- **Paper**: "Ahoy SAILR! There is No Need to DREAM of C"
- **URL**: https://www.usenix.org/system/files/usenixsecurity24-basque.pdf
- **Key Insight**: Compiler-aware structuring algorithm that inverts GCC's goto-inducing transformations
- **Algorithm**:
  1. Lift binary to VEX IR (via angr)
  2. Convert to AIL (ANGR INTERMEDIATE LANGUAGE)
  3. Region identification (SESE regions, reverse topological order)
  4. Schema matching (match against known CFG patterns: if/else, while, for)
  5. C pseudocode emission
- **Relevance**: HIGH — the schema matching approach is implementable without full angr dependency

### Source 3: BRIDGE (ACL 2026)
- **Paper**: "Lifting Optimized Binaries to Canonical Compiler IR"
- **URL**: https://aclanthology.org/2026.acl-long.527.pdf
- **Key Insight**: Uses pseudo-probe instrumentation + RAG for IR reconstruction
- **Relevance**: MEDIUM — too complex for our use case, but confirms CFG→AST pipeline

### Source 4: angr CFG Recovery
- **Documentation**: https://docs.angr.io/en/v9.2.81/analyses/cfg.html
- **Key Insight**: CFGFast is the standard approach — linear sweep + recursive traversal + indirect jump resolution
- **Relevance**: MEDIUM — we can simplify since we already have disassembled instructions

---

## 3. Feasible Implementation Approach

### Minimal Viable AST (without external dependencies)

Since we only have Capstone-based disassembly (no angr/VEX), we can implement:

1. **Basic Block Detection** (from instructions):
   - Block ends at: branch, call, return
   - Block starts at: branch targets, function entries

2. **Simple Expression Trees**:
   - Parse `opStr` into expression nodes
   - Handle: `mov reg, imm`, `add reg, reg/imm`, `ldr/str` patterns

3. **Control Flow Structures**:
   - Detect back-edges → loops
   - Detect diamond patterns → if/else

### What We CANNOT Do (without decompiler backend)

- Full expression recovery (need data flow analysis)
- Variable type inference
- Complex expression simplification
- Function boundary detection (needs CFG)

### Recommended Implementation

```cpp
struct AstNode {
    enum Type { 
        BLOCK, FUNCTION, LOOP, IF_ELSE, 
        ASSIGN, BINOP, CALL, RETURN, UNKNOWN 
    };
    Type type;
    uint64_t address;
    std::string label;
    std::vector<AstNode*> children;
};
```

The plugin should:
1. Take disassembly results
2. Build basic blocks
3. Construct a simple AST with BLOCK, LOOP, IF_ELSE nodes
4. Return the AST as JSON in `result.output`

---

## 4. Dependencies

| Dependency | Required | Notes |
|------------|----------|-------|
| Capstone (disassembler) | YES | Already available via IDisassembler |
| angr | NO | Too heavy, Python-based |
| VEX IR | NO | angr-specific |
| Z3/CVC5 | NO | Not needed for AST construction |

**Conclusion**: AST recovery is feasible with pure C++ using existing disassembly output. The result will be a simplified AST (not full decompilation), suitable for pattern matching and analysis.

---

## 5. Expected Output Format

```json
{
  "plugin": "Enhanced/AST",
  "functions": [
    {
      "address": "0x1000",
      "name": "main",
      "ast": {
        "type": "BLOCK",
        "children": [
          {"type": "ASSIGN", "op": "mov", "dst": "x0", "src": "#0"},
          {"type": "CALL", "target": "printf"},
          {"type": "RETURN"}
        ]
      }
    }
  ]
}
```

---

## 6. Citations

1. Wang et al. "SALT4Decompile: Decompile with Source code-level Abstract Logic Tree" (2025)
2. Basque et al. "Ahoy SAILR! There is No Need to DREAM of C" (USENIX Security 2024)
3. Zhu et al. "Lifting Optimized Binaries to Canonical Compiler IR" (ACL 2026)
4. angr documentation: CFG Recovery (https://docs.angr.io/en/v9.2.81/analyses/cfg.html)

---

## 7. Recommendation

**Implement**: Simplified AST construction from disassembled instructions
- **Scope**: Basic blocks → simple control flow tree
- **Limitations**: No full expression recovery, no type inference
- **Effort**: ~200-300 LOC C++
- **Value**: Enables pattern matching, code structure visualization
