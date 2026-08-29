#ifndef JSON_FRAMER_H
#define JSON_FRAMER_H

#include <functional>
#include <string>

namespace transfer {

// Cuts a byte stream into whole JSON objects.
//
// Controllers send them back to back, with no length and no agreed delimiter,
// so the end of a message can only be found by reading it: braces are counted,
// and the ones inside strings do not count. The Qt version took the first
// closing brace it saw, which meant a nested object was never parsed whole.
class JsonFramer {
public:
    using ObjectHandler = std::function<void(const std::string& json)>;

    explicit JsonFramer(ObjectHandler handler);

    void Feed(const char* data, size_t size);

    // Anything that cannot be the start of an object — line breaks, spaces,
    // rubbish between messages — is dropped rather than kept forever.
    void Reset();

private:
    ObjectHandler handler_;
    std::string buffer_;
    int depth_{0};
    bool in_string_{false};
    bool escaped_{false};
};

}   //transfer

#endif // JSON_FRAMER_H
