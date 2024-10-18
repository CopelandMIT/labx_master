#include <cstdint>

namespace Strata
{
    void getVersion(uint16_t &major, uint16_t &minor, uint16_t &patch, uint16_t &build)
    {
        major = STRATA_VERSION_MAJOR;
        minor = STRATA_VERSION_MINOR;
        patch = STRATA_VERSION_PATCH;
        build = STRATA_VERSION_BUILD;
    }
}