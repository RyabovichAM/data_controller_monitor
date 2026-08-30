#ifndef COLLECTOR_DIRECTORY_H
#define COLLECTOR_DIRECTORY_H

#include <memory>
#include <string>
#include <vector>

#include "data_storage_domain.h"

namespace data_storage {

// Answers "who has anything stored" — a question that spans every collector at
// once, unlike DataStorageInterface, which is scoped to one. A file backend
// answers it by listing a directory; a database backend by querying a table
// shared across collectors. Split out of StorageRegistry, which used to reach
// into std::filesystem directly and had no way to answer it for anything else.
class CollectorDirectory {
public:
    virtual ~CollectorDirectory() = default;

    // Never creates anything: a lookup for a collector nobody ever heard of
    // must not manufacture a directory or a row for it.
    virtual bool HasCollector(const std::string& collector_id) const = 0;

    virtual std::vector<std::string> KnownCollectors() const = 0;
};

class CollectorDirectoryFactory {
public:
    // Throws std::invalid_argument on a type nobody can build.
    static std::unique_ptr<CollectorDirectory> Create(const Settings& settings);
};

}   //data_storage

#endif // COLLECTOR_DIRECTORY_H
