#pragma once

#include <cstdint>
#include <xhash>

namespace ScorchEngine {
    class UUID {
    public:
        UUID();
        UUID(uint64_t override);
        UUID(const UUID&) = default;
        ~UUID();

        operator uint64_t() const { return uuid; }
    private:
        uint64_t uuid;
    };
}

namespace std {
    template<>
    struct hash<ScorchEngine::UUID> {
        size_t operator()(const ScorchEngine::UUID& id) const {
            return static_cast<uint64_t>(id);
        }
    };
}