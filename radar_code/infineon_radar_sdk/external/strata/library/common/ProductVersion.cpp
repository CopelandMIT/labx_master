#include "ProductVersion.hpp"

#include <sstream>
#include <tuple>

ProductVersion::ProductVersion(uint16_t major, uint16_t minor, uint16_t patch) :
    m_major(major), m_minor(minor), m_patch(patch), m_build(0)
{
}

ProductVersion::ProductVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build) :
    m_major(major), m_minor(minor), m_patch(patch), m_build(build)
{
}

ProductVersion::ProductVersion(const std::array<uint16_t, 8> &version) :
    m_major(version[0]), m_minor(version[1]), m_patch(version[2]), m_build(version[3])
{
}

std::ostream &operator<<(std::ostream &os, const ProductVersion &v)
{
    os << v.m_major << '.' << v.m_minor << '.' << v.m_patch;

    const auto developmentPhase = v.m_build & 0xF000;
    const auto buildVersion     = v.m_build & 0x0FFF;
    switch (developmentPhase)
    {
        case ProductVersion::Alpha:
            os << "-alpha";
            if (buildVersion)
                os << buildVersion;
            break;
        case ProductVersion::Beta:
            os << "-beta";
            if (buildVersion)
                os << buildVersion;
            break;
        case ProductVersion::ReleaseCandidate:
            os << "-rc";
            if (buildVersion)
                os << buildVersion;
            break;
        default:
            break;
    }

    return os;
}

std::string ProductVersion::toString() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

bool ProductVersion::hasSameMajorAndMinor(const ProductVersion &other) const
{
    return std::tie(m_major, m_minor) == std::tie(other.m_major, other.m_minor);
}

bool ProductVersion::operator<(const ProductVersion &rhs) const
{
    return std::tie(m_major, m_minor, m_patch) < std::tie(rhs.m_major, rhs.m_minor, rhs.m_patch);
}

bool ProductVersion::operator<=(const ProductVersion &rhs) const
{
    return !(rhs < *this);
}

bool ProductVersion::operator>=(const ProductVersion &rhs) const
{
    return !(*this < rhs);
}

bool ProductVersion::operator>(const ProductVersion &rhs) const
{
    return rhs < *this;
}

bool ProductVersion::operator==(const ProductVersion &rhs) const
{
    return std::tie(m_major, m_minor, m_patch) == std::tie(rhs.m_major, rhs.m_minor, rhs.m_patch);
}

bool ProductVersion::operator!=(const ProductVersion &rhs) const
{
    return !(*this == rhs);
}
