// Enhanced/Crypt plugin — Decrypt algorithms detected by FindCrypt3.
// Uses DeCrypt3 engine to decrypt encrypted data in binaries.
// Source: FIPS 197 (AES), FIPS 46-3 (DES), RFC 7914 (ChaCha20).

#include "Plugin/IPlugin.h"
#include "Disassembler/IDisassembler.h"
#include "Plugin/Enhanced/Crypt/DeCrypt3/DeCrypt3.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace omnibyte::hydradis::plugin {

class EnhancedDecryptPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/Crypt/DeCrypt3"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        // Parse configuration from context
        std::string algorithm = ctx.getConfig("algorithm", "AES-ECB");
        std::string keyHex = ctx.getConfig("key", "");
        std::string ivHex = ctx.getConfig("iv", "");
        std::string ciphertextHex = ctx.getConfig("ciphertext", "");

        // If no explicit ciphertext provided, try to extract from disassembly
        if (ciphertextHex.empty() && ctx.disassembly) {
            // Extract raw bytes from the disassembly section
            const auto& instrs = ctx.disassembly->instructions;
            if (!instrs.empty()) {
                std::vector<uint8_t> buffer;
                for (const auto& instr : instrs) {
                    buffer.insert(buffer.end(), instr.bytes.begin(), instr.bytes.end());
                }

                // Convert buffer to hex string for processing
                std::ostringstream hexStream;
                for (uint8_t byte : buffer) {
                    hexStream << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(byte);
                }
                ciphertextHex = hexStream.str();
            }
        }

        if (ciphertextHex.empty()) {
            result.errorMessage = "no ciphertext provided (set 'ciphertext' config or provide disassembly)";
            return result;
        }

        if (keyHex.empty()) {
            result.errorMessage = "no key provided (set 'key' config)";
            return result;
        }

        // Decode hex inputs
        omnibyte::deob::DeCrypt3Engine engine;
        std::vector<uint8_t> ciphertext = engine.hexDecode(ciphertextHex);
        std::vector<uint8_t> key = engine.hexDecode(keyHex);
        std::vector<uint8_t> iv;
        if (!ivHex.empty()) {
            iv = engine.hexDecode(ivHex);
        }

        // Map algorithm name to enum
        omnibyte::deob::CipherAlgorithm algo;
        if (algorithm == "AES-ECB") algo = omnibyte::deob::CipherAlgorithm::AES_ECB;
        else if (algorithm == "AES-CBC") algo = omnibyte::deob::CipherAlgorithm::AES_CBC;
        else if (algorithm == "AES-CTR") algo = omnibyte::deob::CipherAlgorithm::AES_CTR;
        else if (algorithm == "DES-ECB") algo = omnibyte::deob::CipherAlgorithm::DES_ECB;
        else if (algorithm == "DES-CBC") algo = omnibyte::deob::CipherAlgorithm::DES_CBC;
        else if (algorithm == "3DES-ECB") algo = omnibyte::deob::CipherAlgorithm::TRIPLE_DES_ECB;
        else if (algorithm == "3DES-CBC") algo = omnibyte::deob::CipherAlgorithm::TRIPLE_DES_CBC;
        else if (algorithm == "RC4") algo = omnibyte::deob::CipherAlgorithm::RC4;
        else if (algorithm == "ChaCha20") algo = omnibyte::deob::CipherAlgorithm::CHACHA20;
        else if (algorithm == "TEA") algo = omnibyte::deob::CipherAlgorithm::TEA;
        else if (algorithm == "XTEA") algo = omnibyte::deob::CipherAlgorithm::XTEA;
        else if (algorithm == "XXTEA") algo = omnibyte::deob::CipherAlgorithm::XXTEA;
        else if (algorithm == "XOR-SINGLE") algo = omnibyte::deob::CipherAlgorithm::XOR_SINGLE;
        else if (algorithm == "XOR-MULTIPLE") algo = omnibyte::deob::CipherAlgorithm::XOR_MULTIPLE;
        else if (algorithm == "XOR-ROLLING") algo = omnibyte::deob::CipherAlgorithm::XOR_ROLLING;
        else if (algorithm == "Base64") algo = omnibyte::deob::CipherAlgorithm::BASE64_DECODE;
        else if (algorithm == "Hex") algo = omnibyte::deob::CipherAlgorithm::HEX_DECODE;
        else if (algorithm == "Skipjack") algo = omnibyte::deob::CipherAlgorithm::SKIPJACK;
        else if (algorithm == "CAST-128") algo = omnibyte::deob::CipherAlgorithm::CAST128_ECB;
        else if (algorithm == "CAST-256") algo = omnibyte::deob::CipherAlgorithm::CAST256_ECB;
        else {
            result.errorMessage = "unsupported algorithm: " + algorithm +
                                  ". Supported: AES-ECB, AES-CBC, AES-CTR, DES-ECB, DES-CBC, "
                                  "3DES-ECB, 3DES-CBC, RC4, ChaCha20, TEA, XTEA, XXTEA, "
                                  "XOR-SINGLE, XOR-MULTIPLE, XOR-ROLLING, Base64, Hex, "
                                  "Skipjack, CAST-128, CAST-256";
            return result;
        }

        // Build decryption parameters
        omnibyte::deob::DecryptParams params;
        params.ciphertext = ciphertext;
        params.key = key;
        params.iv = iv;
        params.algorithm = algo;

        // Handle XOR-specific parameters
        if (algorithm == "XOR-SINGLE" && !key.empty()) {
            params.xorSingleKey = key[0];
        }
        if (algorithm == "XOR-ROLLING") {
            params.xorRollingKey = key;
        }

        // Execute decryption
        omnibyte::deob::DecryptResult decryptResult = engine.decrypt(params);

        if (!decryptResult.success) {
            result.errorMessage = "decryption failed: " + decryptResult.errorMessage;
            return result;
        }

        // Build output JSON
        std::ostringstream output;
        output << "{"
               << "\"plugin\":\"Enhanced/Crypt/DeCrypt3\","
               << "\"algorithm\":\"" << decryptResult.algorithmName << "\","
               << "\"plaintext_hex\":\"";

        // Convert plaintext to hex
        for (uint8_t byte : decryptResult.plaintext) {
            output << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(byte);
        }

        output << "\","
               << "\"plaintext_size\":" << decryptResult.plaintext.size() << ","
               << "\"ciphertext_size\":" << ciphertext.size() << ","
               << "\"status\":\"decrypt_complete\""
               << "}";

        result.success = true;
        result.output = output.str();
        return result;
    }

    void onUnload() override {}
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_decrypt_plugin() {
    return std::make_unique<EnhancedDecryptPlugin>();
}

extern "C" int enhanced_decrypt_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin
