#include "Dumper.h"
#include "DumperCore/SharedUtils/SharedUtils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Engines/UnrealEngine/UnrealEngineEngine.h"
#include "Engines/UnityIL2CPP/UnityIL2CPPEngine.h"
#include "Engines/UnityMono/UnityMonoEngine.h"
#include "Engines/Source2/Source2Engine.h"
#include "Engines/Godot/GodotEngine.h"
#include "Engines/GameMaker/GameMakerEngine.h"
#include "Engines/Cocos2d/Cocos2dEngine.h"

namespace omnibyte::dumper {

namespace {

using json = nlohmann::json;

class EngineAnalyzerAdapter : public IEngineAnalyzer {
public:
    explicit EngineAnalyzerAdapter(std::shared_ptr<IDumperEngine> e) : engine_(std::move(e)) {}
    std::string engineName() const override { return engine_->name(); }
    DumpData analyze(const AnalysisTarget& t, const std::shared_ptr<IEngineProfile>& p) override {
        return engine_->analyze(t, p);
    }
private:
    std::shared_ptr<IDumperEngine> engine_;
};

class EngineResolverAdapter : public IEngineResolver {
public:
    explicit EngineResolverAdapter(std::shared_ptr<IDumperEngine> e) : engine_(std::move(e)) {}
    std::string engineName() const override { return engine_->name(); }
    DumpData resolveSymbols(const AnalysisTarget& t, const std::shared_ptr<IEngineProfile>& p) override {
        return engine_->resolveSymbols(t, p);
    }
private:
    std::shared_ptr<IDumperEngine> engine_;
};

json dumpDataToJson(const DumpData& d) {
    json j;
    j["engine"] = d.engineName;
    j["version"] = d.detectedVersion;
    j["success"] = d.success;

    json types = json::array();
    for (const auto& t : d.typeTable) {
        json tj;
        tj["name"] = t.name;
        tj["address"] = t.address;
        tj["size"] = t.size;
        tj["typeId"] = t.typeId;
        tj["parentType"] = t.parentType;
        tj["interfaces"] = t.interfaces;
        types.push_back(std::move(tj));
    }
    j["types"] = std::move(types);

    json methods = json::array();
    for (const auto& m : d.methodTable) {
        json mj;
        mj["name"] = m.name;
        mj["declaringType"] = m.declaringType;
        mj["address"] = m.address;
        mj["methodIndex"] = m.methodIndex;
        mj["isVirtual"] = m.isVirtual;
        mj["isStatic"] = m.isStatic;
        methods.push_back(std::move(mj));
    }
    j["methods"] = std::move(methods);

    json fields = json::array();
    for (const auto& f : d.fieldTable) {
        json fj;
        fj["name"] = f.name;
        fj["declaringType"] = f.declaringType;
        fj["offset"] = f.offset;
        fj["typeName"] = f.typeName;
        fj["fieldSize"] = f.fieldSize;
        fields.push_back(std::move(fj));
    }
    j["fields"] = std::move(fields);

    json strings = json::array();
    for (const auto& s : d.stringTable) {
        json sj;
        sj["value"] = s.value;
        sj["address"] = s.address;
        strings.push_back(std::move(sj));
    }
    j["strings"] = std::move(strings);
    j["metadata"] = d.metadata;

    return j;
}

} // anon

DumperOutcome Dumper::execute(const DumpRequest& request) {
    DumperOutcome out;

    DumpResult vr = validateTarget(request);
    if (vr != DumpResult::Success) {
        out.status = vr;
        return out;
    }

    if (request.progressCb) request.progressCb("detecting", 0.1f);

    auto outcome = detectEngine(request);
    if (!outcome || !outcome->engine) {
        out.status = DumpResult::EngineNotDetected;
        return out;
    }

    AnalysisTarget target;
    if (request.pid > 0) {
        target = AnalysisTarget::fromProcess(request.pid, 0);
    } else {
        target = AnalysisTarget::fromFile(request.apkPath);
    }

    auto [analyzer, resolver] = createEnginePair(outcome->engine->type());
    if (!analyzer) {
        out.status = DumpResult::InvalidRequest;
        return out;
    }

    if (request.progressCb) request.progressCb("analyzing", 0.3f);

    DumpData analyzeResult = analyzer->analyze(target, outcome->profile);
    if (!analyzeResult.success) {
        out.status = DumpResult::AnalyzerFailed;
        out.data = std::move(analyzeResult);
        return out;
    }

    if (request.progressCb) request.progressCb("resolving", 0.5f);

    DumpData resolveResult;
    if (resolver) {
        resolveResult = resolver->resolveSymbols(target, outcome->profile);
    }

    if (request.progressCb) request.progressCb("normalizing", 0.7f);

    DumpData merged;
    if (resolveResult.success) {
        merged = ResultNormalizer::mergeResults({analyzeResult, resolveResult});
    } else {
        merged = analyzeResult;
    }

    merged.typeTable   = ResultNormalizer::normalizeTypes(merged.typeTable);
    merged.methodTable = ResultNormalizer::normalizeMethods(merged.methodTable);
    merged.fieldTable  = ResultNormalizer::normalizeFields(merged.fieldTable);
    merged.stringTable = ResultNormalizer::normalizeStrings(merged.stringTable);

    if (request.progressCb) request.progressCb("exporting", 0.9f);

    if (!request.exportPath.empty()) {
        DumpResult er = DumpResult::Success;
        if (request.exportFormat == "json") {
            er = exportJson(merged, request.exportPath);
        } else {
            auto& reg = export_core::ExportRegistry::instance();
            if (!reg.hasExporter(request.exportFormat)) {
                er = DumpResult::ExportFailed;
            } else if (!reg.exportTo(request.exportFormat, merged, request.exportPath)) {
                er = DumpResult::ExportFailed;
            }
        }
        if (er != DumpResult::Success) {
            out.status = er;
            out.data = std::move(merged);
            return out;
        }
    }

    out.status = DumpResult::Success;
    out.data = std::move(merged);
    out.engineName = outcome->engine->name();
    out.detectedVersion = outcome->detection.detectedVersion;
    out.detectionConfidence = outcome->detection.confidence;

    if (!resolveResult.success) {
        out.status = DumpResult::ResolverFailed;
    }

    if (request.progressCb) request.progressCb("done", 1.0f);
    return out;
}

DumperOutcome Dumper::dumpFile(const std::string& apkPath,
                               const std::optional<EngineType>& override) {
    DumpRequest req;
    req.apkPath = apkPath;
    req.manualOverride = override;
    return execute(req);
}

DumperOutcome Dumper::dumpProcess(pid_t pid,
                                  const std::optional<EngineType>& override) {
    DumpRequest req;
    req.pid = pid;
    req.manualOverride = override;
    return execute(req);
}

std::vector<EngineType> Dumper::availableEngines() const {
    return {
        EngineType::UnrealEngine,
        EngineType::UnityIL2CPP,
        EngineType::UnityMono,
        EngineType::Source2,
        EngineType::Godot,
        EngineType::GameMaker,
        EngineType::Cocos2d
    };
}

void Dumper::loadConfig(const config::EngineDetectionConfig& detection,
                        const config::FileLimitsConfig& fileLimits,
                        const config::LoggingConfig& logging) {
    detectionCfg_ = detection;
    fileLimitsCfg_ = fileLimits;
    loggingCfg_ = logging;
}

std::optional<DetectionOutcome> Dumper::detectEngine(const DumpRequest& request) {
    AnalysisTarget target;
    if (request.pid > 0) {
        target = AnalysisTarget::fromProcess(request.pid, 0);
    } else {
        target = AnalysisTarget::fromFile(request.apkPath);
    }

    if (request.manualOverride.has_value()) {
        std::shared_ptr<IDumperEngine> engine;
        switch (*request.manualOverride) {
            case EngineType::UnrealEngine:
                engine = std::make_shared<unrealengine::UnrealEngineDumper>(); break;
            case EngineType::UnityIL2CPP:
                engine = std::make_shared<unityil2cpp::UnityIL2CPPDumper>(); break;
            case EngineType::UnityMono:
                engine = std::make_shared<unitymono::UnityMonoDumper>(); break;
            case EngineType::Source2:
                engine = std::make_shared<source2::Source2Dumper>(); break;
            case EngineType::Godot:
                engine = std::make_shared<godot::GodotDumper>(); break;
            case EngineType::GameMaker:
                engine = std::make_shared<gamemaker::GameMakerDumper>(); break;
            case EngineType::Cocos2d:
                engine = std::make_shared<cocos2d::Cocos2dDumper>(); break;
            case EngineType::Unknown:
                return std::nullopt;
        }
        if (!engine) return std::nullopt;

        DetectionOutcome outcome;
        outcome.engine = engine;
        outcome.detection.matched = true;
        outcome.detection.confidence = 1.0f;
        outcome.detection.detectedVersion = "manual";

        auto detection = engine->detect(target);
        if (detection.detectedVersion.empty()) {
            outcome.detection.detectedVersion = engine->supportedVersions().empty()
                ? "latest" : engine->supportedVersions().back();
        } else {
            outcome.detection.detectedVersion = detection.detectedVersion;
        }

        if (!outcome.detection.detectedVersion.empty()) {
            outcome.profile = engine->resolveProfile(outcome.detection.detectedVersion);
        }

        return outcome;
    }

    return Detector::detect(target);
}

DumpResult Dumper::validateTarget(const DumpRequest& request) {
    if (request.apkPath.empty() && request.pid <= 0) {
        return DumpResult::InvalidRequest;
    }

    if (!request.apkPath.empty()) {
        if (!utils::fileExists(request.apkPath)) {
            return DumpResult::FileNotFound;
        }

        auto fileData = utils::readFileBytes(request.apkPath);
        if (fileData.empty()) {
            return DumpResult::FileNotFound;
        }

        if (fileData.size() > fileLimitsCfg_.maxFileSizeBytes) {
            return DumpResult::InvalidRequest;
        }

        if (!fileLimitsCfg_.allowedExtensions.empty()) {
            bool extOk = false;
            for (const auto& ext : fileLimitsCfg_.allowedExtensions) {
                if (utils::endsWith(request.apkPath, ext)) {
                    extOk = true;
                    break;
                }
            }
            if (!extOk) return DumpResult::InvalidRequest;
        }
    }

    return DumpResult::Success;
}

std::pair<std::shared_ptr<IEngineAnalyzer>, std::shared_ptr<IEngineResolver>>
Dumper::createEnginePair(EngineType type) {
    std::shared_ptr<IDumperEngine> engine;

    switch (type) {
        case EngineType::UnrealEngine:
            engine = std::make_shared<unrealengine::UnrealEngineDumper>();
            break;
        case EngineType::UnityIL2CPP:
            engine = std::make_shared<unityil2cpp::UnityIL2CPPDumper>();
            break;
        case EngineType::UnityMono:
            engine = std::make_shared<unitymono::UnityMonoDumper>();
            break;
        case EngineType::Source2:
            engine = std::make_shared<source2::Source2Dumper>();
            break;
        case EngineType::Godot:
            engine = std::make_shared<godot::GodotDumper>();
            break;
        case EngineType::GameMaker:
            engine = std::make_shared<gamemaker::GameMakerDumper>();
            break;
        case EngineType::Cocos2d:
            engine = std::make_shared<cocos2d::Cocos2dDumper>();
            break;
        case EngineType::Unknown:
            return {nullptr, nullptr};
    }

    if (!engine) return {nullptr, nullptr};

    auto analyzer = std::make_shared<EngineAnalyzerAdapter>(engine);
    auto resolver = std::make_shared<EngineResolverAdapter>(engine);
    return {std::move(analyzer), std::move(resolver)};
}

DumpResult Dumper::exportJson(const DumpData& data, const std::string& path) {
    json j = dumpDataToJson(data);
    std::string content = j.dump(2);

    size_t maxBytes = fileLimitsCfg_.maxFileSizeBytes;
    size_t maxLines = fileLimitsCfg_.exportSplitMaxLines;

    if (content.size() <= maxBytes) {
        std::ofstream file(path);
        if (!file.is_open()) return DumpResult::ExportFailed;
        file << content;
        return file.good() ? DumpResult::Success : DumpResult::ExportFailed;
    }

    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(std::move(line));
    }

    size_t chunk = maxLines > 0 ? maxLines : 100000;
    size_t partIdx = 0;

    auto basePos = path.rfind('.');
    std::string base = (basePos != std::string::npos) ? path.substr(0, basePos) : path;
    std::string ext = (basePos != std::string::npos) ? path.substr(basePos) : ".json";

    for (size_t i = 0; i < lines.size(); i += chunk) {
        size_t end = std::min(i + chunk, lines.size());
        std::string partPath = base + "_part" + std::to_string(partIdx) + ext;

        std::ofstream partFile(partPath);
        if (!partFile.is_open()) return DumpResult::ExportFailed;

        for (size_t j = i; j < end; ++j) {
            partFile << lines[j] << "\n";
        }

        if (!partFile.good()) return DumpResult::ExportFailed;
        partIdx++;
    }

    return DumpResult::Success;
}

} // namespace omnibyte::dumper
