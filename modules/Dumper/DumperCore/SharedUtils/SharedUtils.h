#pragma once
// DumperCore/SharedUtils — shared utility functions for all engine modules.
// File I/O, memory read, hex encoding, base64, AOB pattern scanning.
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace omnibyte::dumper::utils {

// ── File I/O ──

// Read entire file into byte vector. Returns empty vector on failure.
std::vector<uint8_t> readFileBytes(const std::string& path);

// Read N bytes from file at offset. Returns nullopt on failure.
std::optional<std::vector<uint8_t>> readFileAt(const std::string& path,
                                                size_t offset, size_t count);

// Check if file exists and is readable.
bool fileExists(const std::string& path);

// ── Process Memory ──

// Read process memory via /proc/pid/mem. Returns false on failure.
bool readProcessMemory(int pid, uintptr_t address, void* out, size_t size);

// Read N bytes from process at address. Returns nullopt on failure.
std::optional<std::vector<uint8_t>> readProcessBytes(int pid, uintptr_t address, size_t count);

// ── Hex / Base64 ──

// Convert byte buffer to hex string (lowercase, no prefix).
std::string hexEncode(const uint8_t* data, size_t len);

// Convert hex string to byte vector. Returns nullopt if invalid hex.
std::optional<std::vector<uint8_t>> hexDecode(const std::string& hex);

// Standard base64 encode (RFC 4648).
std::string base64Encode(const uint8_t* data, size_t len);

// Standard base64 decode (RFC 4648). Returns nullopt if invalid.
std::optional<std::vector<uint8_t>> base64Decode(const std::string& str);

// ── String Utils ──

// Trim whitespace from both ends.
std::string trim(const std::string& s);

// Check if string starts with prefix.
bool startsWith(const std::string& str, const std::string& prefix);

// Check if string ends with suffix.
bool endsWith(const std::string& str, const std::string& suffix);

// Case-insensitive substring search. Returns position or nullopt.
std::optional<size_t> findCaseInsensitive(const std::string& haystack,
                                           const std::string& needle);

// ── AOB Pattern Scanner ──

// Scan a memory region for an AOB pattern with mask.
// Returns address of first match, or nullopt if not found.
// mask: 'x' = exact match, '?' = wildcard
std::optional<uintptr_t> scanPattern(const uint8_t* base, size_t regionSize,
                                      const std::string& bytePattern,
                                      const std::string& mask);

// Scan for pattern and extract RIP-relative address at offset.
// Common UE pattern: mov rax, [rip+disp32] → extract disp32 at match+3.
std::optional<uintptr_t> scanAndExtractRIP(const uint8_t* base, size_t regionSize,
                                             const std::string& bytePattern,
                                             const std::string& mask,
                                             int addressOffset, int instructionLen);

// ── Memory Utilities ──

// Check if address is page-aligned (typically 0x1000 on ARM64).
bool isPageAligned(uintptr_t addr, size_t pageSize = 0x1000);

// Round up to next page boundary.
uintptr_t pageAlignUp(uintptr_t addr, size_t pageSize = 0x1000);

} // namespace omnibyte::dumper::utils
