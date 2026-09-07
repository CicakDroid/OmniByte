#pragma once
#include "DumperCore/DumpResult.h"
#include "DumperCore/AnalysisTarget.h"
#include "DumperCore/IEngineProfile.h"
#include <memory>
#include <string>

namespace omnibyte::dumper {

class IEngineAnalyzer {
public:
    virtual ~IEngineAnalyzer() = default;
    virtual std::string engineName() const = 0;
    virtual DumpData analyze(const AnalysisTarget& target,
                             const std::shared_ptr<IEngineProfile>& profile) = 0;
};

class IEngineResolver {
public:
    virtual ~IEngineResolver() = default;
    virtual std::string engineName() const = 0;
    virtual DumpData resolveSymbols(const AnalysisTarget& target,
                                    const std::shared_ptr<IEngineProfile>& profile) = 0;
};

} // namespace omnibyte::dumper
