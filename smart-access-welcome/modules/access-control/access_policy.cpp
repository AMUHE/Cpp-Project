#include "saw/access/access_policy.h"

#include <algorithm>
#include <stdexcept>

namespace saw::access {

AccessPolicy::AccessPolicy(PolicyOptions options) : options_(options)
{
    if (options_.requiredConsecutiveMatches < 1)
        throw std::invalid_argument("requiredConsecutiveMatches must be positive");
    if (options_.cooldownMilliseconds < 0)
        throw std::invalid_argument("cooldownMilliseconds cannot be negative");
}

PolicyResult AccessPolicy::observe(const Observation &observation, std::int64_t nowMilliseconds)
{
    const std::string key = observation.matched && !observation.personId.empty()
        ? observation.personId : std::string("__unknown__");
    if (key != candidateKey_) {
        candidateKey_ = key;
        candidateName_ = observation.displayName;
        consecutive_ = 1;
    } else {
        ++consecutive_;
    }

    if (consecutive_ < options_.requiredConsecutiveMatches)
        return {};

    consecutive_ = 0;
    const auto cooldown = nextDecisionAt_.find(key);
    if (cooldown != nextDecisionAt_.end() && nowMilliseconds < cooldown->second)
        return {};
    nextDecisionAt_[key] = nowMilliseconds + options_.cooldownMilliseconds;

    PolicyResult result;
    if (key == "__unknown__") {
        result.decision = Decision::Denied;
        result.reason = "unknown_person";
    } else {
        result.decision = Decision::Granted;
        result.personId = key;
        result.displayName = candidateName_;
        result.reason = "recognized";
    }
    return result;
}

void AccessPolicy::reset()
{
    candidateKey_.clear();
    candidateName_.clear();
    consecutive_ = 0;
}

} // namespace saw::access
