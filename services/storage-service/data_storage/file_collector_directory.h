#ifndef FILE_COLLECTOR_DIRECTORY_H
#define FILE_COLLECTOR_DIRECTORY_H

#include <string>

#include "collector_directory.h"

namespace data_storage {

// One subdirectory of root per collector — the same layout FileDataStorage
// writes into. Read straight off the disk rather than off any in-memory list,
// so a restart does not lose the collectors that are quiet right now.
class FileCollectorDirectory : public CollectorDirectory {
public:
    explicit FileCollectorDirectory(std::string root);

    bool HasCollector(const std::string& collector_id) const override;
    std::vector<std::string> KnownCollectors() const override;

private:
    std::string root_;
};

}   //data_storage

#endif // FILE_COLLECTOR_DIRECTORY_H
