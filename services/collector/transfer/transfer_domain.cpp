#include "transfer_domain.h"

namespace transfer {

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

}   //transfer
