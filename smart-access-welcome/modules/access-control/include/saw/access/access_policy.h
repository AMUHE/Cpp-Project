#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace saw::access {

enum class Decision { None, Granted, Denied };

struct Observation {
    bool matched{false};
    std::string personId;
    std::string displayName;
};

struct PolicyResult {
    Decision decision{Decision::None};
    std::string personId;
    std::string displayName;
    std::string reason;
};

struct PolicyOptions {
    int requiredConsecutiveMatches{3};
    std::int64_t cooldownMilliseconds{10000};
};

class AccessPolicy {
public:
    explicit AccessPolicy(PolicyOptions options = {});
    PolicyResult observe(const Observation &observation, std::int64_t nowMilliseconds);
    void reset();

private:
    PolicyOptions options_;
    std::string candidateKey_;
    std::string candidateName_;
    int consecutive_{0};
    std::unordered_map<std::string, std::int64_t> nextDecisionAt_;
};

} // namespace saw::access
