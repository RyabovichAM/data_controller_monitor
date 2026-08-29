#ifndef TRANSFER_DOMAIN_H
#define TRANSFER_DOMAIN_H

#include <functional>
#include <string>
#include <unordered_map>

namespace transfer {

// The flat map the configuration arrives in — the same vocabulary the contract
// is mapped into, "type" telling the factory which transport to build.
using Settings = std::unordered_map<std::string, std::string>;

using JsonHandler = std::function<void(const std::string& json)>;
using ErrorHandler = std::function<void(const std::string& error)>;

std::string Value(const Settings& settings, const std::string& key);

}   //transfer

#endif // TRANSFER_DOMAIN_H
