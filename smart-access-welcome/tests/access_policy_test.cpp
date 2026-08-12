#include "saw/access/access_policy.h"

#include <iostream>
#include <stdexcept>

using saw::access::AccessPolicy;
using saw::access::Decision;
using saw::access::Observation;
using saw::access::PolicyOptions;

namespace {
bool check(bool condition, const char *expression, int line)
{
    if (condition) return true;
    std::cerr << "check failed at line " << line << ": " << expression << '\n';
    return false;
}
}

#define CHECK(expression) do { if (!check((expression), #expression, __LINE__)) return 1; } while (false)

int main()
{
    AccessPolicy policy(PolicyOptions{3, 10000});
    const Observation alice{true, "alice-id", "Alice"};
    CHECK(policy.observe(alice, 1000).decision == Decision::None);
    CHECK(policy.observe(alice, 1010).decision == Decision::None);
    const auto granted = policy.observe(alice, 1020);
    CHECK(granted.decision == Decision::Granted);
    CHECK(granted.personId == "alice-id");

    CHECK(policy.observe(alice, 1030).decision == Decision::None);
    CHECK(policy.observe(alice, 1040).decision == Decision::None);
    CHECK(policy.observe(alice, 1050).decision == Decision::None);
    CHECK(policy.observe(alice, 11020).decision == Decision::None);
    CHECK(policy.observe(alice, 11030).decision == Decision::None);
    CHECK(policy.observe(alice, 11040).decision == Decision::Granted);

    policy.reset();
    const Observation unknown{};
    CHECK(policy.observe(unknown, 20000).decision == Decision::None);
    CHECK(policy.observe(unknown, 20010).decision == Decision::None);
    const auto denied = policy.observe(unknown, 20020);
    CHECK(denied.decision == Decision::Denied);
    CHECK(denied.reason == "unknown_person");

    policy.reset();
    CHECK(policy.observe(alice, 40000).decision == Decision::None);
    CHECK(policy.observe(unknown, 40010).decision == Decision::None);
    CHECK(policy.observe(alice, 40020).decision == Decision::None);

    bool invalidRejected = false;
    try {
        AccessPolicy invalid(PolicyOptions{0, 1000});
    } catch (const std::invalid_argument &) {
        invalidRejected = true;
    }
    CHECK(invalidRejected);
    return 0;
}
