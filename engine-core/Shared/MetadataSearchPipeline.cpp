#include "MetadataSearchPipeline.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <array>

// Include DeCrypt3 for encryption detection and decryption
#include "../HydraDis/Plugin/Enhanced/Crypt/DeCrypt3/DeCrypt3.h"

namespace omnibyte::shared {

MetadataSearchPipeline::MetadataSearchPipeline(std::shared_ptr<IMetadataStore> store)
    : store_(std::move(store)) {}

PhaseResult MetadataSearchPipeline::search(const std::string& targetPath,
                                           const SearchConfig& config) {
    phaseResults_.clear();
    encryptionDetection_ = {};
    
    // Phase 1: Quick Scan
    if (config.enablePhase1_QuickScan) {
        auto result = phase1_QuickScan(targetPath, config);
        phaseResults_.push_back(result);
    }
    
    // Phase 2: Pattern Search
    if (config.enablePhase2_PatternSearch) {
        auto result = phase2_PatternSearch(targetPath, config);
        phaseResults_.push_back(result);
    }
    
    // Phase 3: Deep Analysis
    if (config.enablePhase3_DeepAnalysis) {
        auto result = phase3_DeepAnalysis(targetPath, config);
        phaseResults_.push_back(result);
    }
    
    // Phase 4: Crypt Detection (if encryption detected)
    if (config.enablePhase4_CryptDetection) {
        auto result = phase4_CryptDetection(targetPath, config);
        phaseResults_.push_back(result);
    }
    
    // Phase 5: Validation
    if (config.enablePhase5_Validation) {
        auto result = phase5_Validation(targetPath, config);
        phaseResults_.push_back(result);
    }
    
    // Return combined result
    PhaseResult combined;
    combined.phaseName = "Combined";
    combined.completed = true;
    
    for (const auto& phase : phaseResults_) {
        combined.completed &= phase.completed;
        combined.foundMetadata |= phase.foundMetadata;
        combined.entriesFound += phase.entriesFound;
        combined.errors.insert(combined.errors.end(),
                               phase.errors.begin(), phase.errors.end());
        combined.warnings.insert(combined.warnings.end(),
                                 phase.warnings.begin(), phase.warnings.end());
    }
    
    return combined;
}

PhaseResult MetadataSearchPipeline::phase1_QuickScan(const std::string& targetPath,
                                                     const SearchConfig& config) {
    PhaseResult result;
    result.phaseName = "Phase1_QuickScan";
    
    // TODO: Implement magic bytes detection
    // - Check file signatures
    // - Identify file type
    // - Quick hash-based lookup
    
    result.completed = true;
    return result;
}

PhaseResult MetadataSearchPipeline::phase2_PatternSearch(const std::string& targetPath,
                                                         const SearchConfig& config) {
    PhaseResult result;
    result.phaseName = "Phase2_PatternSearch";
    
    // TODO: Implement pattern-based search
    // - Search for known metadata structures
    // - Use AOB patterns from IEngineProfile
    // - Look for string tables
    
    result.completed = true;
    return result;
}

PhaseResult MetadataSearchPipeline::phase3_DeepAnalysis(const std::string& targetPath,
                                                        const SearchConfig& config) {
    PhaseResult result;
    result.phaseName = "Phase3_DeepAnalysis";
    
    // TODO: Implement deep analysis
    // - Full metadata parsing
    // - Type/field/method extraction
    // - String literal recovery
    
    result.completed = true;
    return result;
}

PhaseResult MetadataSearchPipeline::phase4_CryptDetection(const std::string& targetPath,
                                                          const SearchConfig& config) {
    PhaseResult result;
    result.phaseName = "Phase4_CryptDetection";
    
    // Read target file for analysis
    std::ifstream file(targetPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.errors.push_back("Cannot open file: " + targetPath);
        result.completed = true;
        return result;
    }
    
    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) {
        result.warnings.push_back("File is empty: " + targetPath);
        result.completed = true;
        return result;
    }
    
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        result.errors.push_back("Failed to read file: " + targetPath);
        result.completed = true;
        return result;
    }
    
    // Check if data is encrypted using entropy analysis
    if (isEncrypted(buffer)) {
        encryptionDetection_.detected = true;
        encryptionDetection_.confidence = 0.8f;  // Entropy-based detection is not 100% reliable
        
        // Try to detect specific encryption algorithms using DeCrypt3
        omnibyte::deob::DeCrypt3Engine cryptEngine;
        
        // Try common algorithms
        std::vector<omnibyte::deob::CipherAlgorithm> algorithmsToTry = {
            omnibyte::deob::CipherAlgorithm::AES_ECB,
            omnibyte::deob::CipherAlgorithm::AES_CBC,
            omnibyte::deob::CipherAlgorithm::RC4,
            omnibyte::deob::CipherAlgorithm::XOR_SINGLE
        };
        
        for (auto algo : algorithmsToTry) {
            if (cryptEngine.isAlgorithmSupported(algo)) {
                omnibyte::deob::DecryptParams params;
                params.ciphertext = buffer;
                params.algorithm = algo;
                
                // For demo: try with empty key (will fail but shows the interface)
                auto decryptResult = cryptEngine.decrypt(params);
                if (decryptResult.success) {
                    encryptionDetection_.algorithmName = decryptResult.algorithmName;
                    break;
                }
            }
        }
        
        if (encryptionDetection_.algorithmName.empty()) {
            encryptionDetection_.algorithmName = "Unknown (high entropy detected)";
        }
        
        result.foundMetadata = true;
        result.entriesFound = 1;
        
        // Store encryption info in metadata store
        if (store_) {
            store_->set("encryption.detected", "true", MetadataSource::CryptDecrypted);
            store_->set("encryption.algorithm", encryptionDetection_.algorithmName, MetadataSource::CryptDecrypted);
            store_->set("encryption.confidence", std::to_string(encryptionDetection_.confidence), MetadataSource::CryptDecrypted);
        }
    }
    
    result.completed = true;
    return result;
}

PhaseResult MetadataSearchPipeline::phase5_Validation(const std::string& targetPath,
                                                      const SearchConfig& config) {
    PhaseResult result;
    result.phaseName = "Phase5_Validation";
    
    // TODO: Implement validation
    // - Cross-reference metadata entries
    // - Validate consistency
    // - Mark confidence scores
    
    result.completed = true;
    return result;
}

bool MetadataSearchPipeline::isEncrypted(const std::vector<uint8_t>& data) const {
    if (data.empty()) return false;
    
    // Simple entropy check - high entropy suggests encryption
    std::array<size_t, 256> freq{};
    for (uint8_t byte : data) {
        freq[byte]++;
    }
    
    double entropy = 0.0;
    double dataSize = static_cast<double>(data.size());
    for (size_t count : freq) {
        if (count > 0) {
            double p = static_cast<double>(count) / dataSize;
            entropy -= p * std::log2(p);
        }
    }
    
    // Entropy > 7.5 suggests encryption (max is 8.0 for truly random data)
    return entropy > 7.5;
}

PhaseResult MetadataSearchPipeline::getPhaseResult(int phase) const {
    if (phase >= 0 && phase < static_cast<int>(phaseResults_.size())) {
        return phaseResults_[phase];
    }
    return {};
}

std::vector<PhaseResult> MetadataSearchPipeline::getAllPhaseResults() const {
    return phaseResults_;
}

bool MetadataSearchPipeline::wasEncryptionDetected() const {
    return encryptionDetection_.detected;
}

const EncryptionDetection& MetadataSearchPipeline::getEncryptionDetection() const {
    return encryptionDetection_;
}

} // namespace omnibyte::shared
