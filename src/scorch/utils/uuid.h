#pragma once

#include <cstdint>
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
