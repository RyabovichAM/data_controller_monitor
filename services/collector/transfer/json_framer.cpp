#include "json_framer.h"

#include <utility>

namespace transfer {

JsonFramer::JsonFramer(ObjectHandler handler)
    : handler_{std::move(handler)} {
}

void JsonFramer::Feed(const char* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        const char symbol = data[i];

        if (depth_ == 0) {
            if (symbol != '{') {
                continue;   // between messages: whitespace, newlines, rubbish
            }
            buffer_.clear();
        }

        buffer_.push_back(symbol);

        if (in_string_) {
            if (escaped_) {
                escaped_ = false;
            } else if (symbol == '\\') {
                escaped_ = true;
            } else if (symbol == '"') {
                in_string_ = false;
            }
            continue;
        }

        switch (symbol) {
        case '"':
            in_string_ = true;
            break;
        case '{':
            ++depth_;
            break;
        case '}':
            --depth_;
            if (depth_ == 0 && handler_) {
                handler_(buffer_);
                buffer_.clear();
            }
            break;
        default:
            break;
        }
    }
}

void JsonFramer::Reset() {
    buffer_.clear();
    depth_ = 0;
    in_string_ = false;
    escaped_ = false;
}

}   //transfer
