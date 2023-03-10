#include "uuid.h"

#include <random>
namespace ScorchEngine {
    static std::random_device UUID_randomDevice;
    static std::mt19937_64 UUID_engine(UUID_randomDevice());
    static std::uniform_int_distribution<uint64_t> UUID_uniformDistribution;

    UUID::UUID() : uuid(UUID_uniformDistribution(UUID_engine)) {}
    UUID::UUID(uint64_t override) : uuid(override) {}
    UUID::~UUID() {}
}