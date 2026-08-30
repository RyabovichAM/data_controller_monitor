#include "file_collector_directory.h"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace data_storage {

namespace fs = std::filesystem;

FileCollectorDirectory::FileCollectorDirectory(std::string root)
    : root_{std::move(root)} {
}

bool FileCollectorDirectory::HasCollector(const std::string& collector_id) const {
    std::error_code error;
    return fs::is_directory(root_ + "/" + collector_id, error);
}

std::vector<std::string> FileCollectorDirectory::KnownCollectors() const {
    std::vector<std::string> collector_ids;

    std::error_code error;
    for (const auto& entry : fs::directory_iterator{root_, error}) {
        if (entry.is_directory(error)) {
            collector_ids.push_back(entry.path().filename().string());
        }
    }

    std::sort(collector_ids.begin(), collector_ids.end());
    return collector_ids;
}

}   //data_storage
