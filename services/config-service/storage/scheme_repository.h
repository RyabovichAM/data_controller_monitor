#ifndef SCHEME_REPOSITORY_H
#define SCHEME_REPOSITORY_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "scheme_service.pb.h"

namespace config {

using dcm::scheme::v1::Scheme;
using dcm::scheme::v1::SchemeSummary;

// Storage of mnemonic schemes. Same shape as ConfigRepository, and for the
// same reason: the gRPC layer talks to this interface only, so SQL stays out
// of the service implementation.
class SchemeRepository {
public:
    struct SaveResult {
        bool conflict{false};
        int64_t version{0};   // version after the save, the stored one on conflict
    };

    virtual ~SchemeRepository() = default;

    virtual std::optional<Scheme> Get(const std::string& scheme_id) = 0;

    // Summaries, not whole schemes: a menu of schemes has no use for every
    // shape of every one of them.
    virtual std::vector<SchemeSummary> List() = 0;

    virtual SaveResult Save(const Scheme& scheme, int64_t expected_version) = 0;
    virtual bool Delete(const std::string& scheme_id) = 0;
};

}   //config

#endif // SCHEME_REPOSITORY_H
