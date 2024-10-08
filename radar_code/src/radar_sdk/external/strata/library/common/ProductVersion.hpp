#pragma once

#include <array>
#include <cstdint>
#include <string>

///
/// \brief The ProductVersion class represents a version.
/// \details This can be used to compare different versions and also to represent a version as a string.
///
class ProductVersion
{
public:
    explicit ProductVersion(uint16_t major, uint16_t minor, uint16_t patch);

    ///
    /// \param build is interpreted as development phase, see DevelopmentPhase
    ///
    explicit ProductVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build);

    explicit ProductVersion(const std::array<uint16_t, 8> &version);

    /// The development phase of the software, e.g. Alpha.
    ///
    /// Using this leads to the addition of a e.g. "-rc" to the version string.
    /// You can also number the development phases, e.g. "-beta2" can be achieved
    /// by setting the build parameter in ProductVersion to `ProductVersion::Beta + 2`
    ///
    enum DevelopmentPhase : uint16_t
    {
        Alpha            = 0xA000,
        Beta             = 0xB000,
        ReleaseCandidate = 0xC000,
    };

    friend std::ostream &operator<<(std::ostream &os, const ProductVersion &v);
    std::string toString() const;

    bool hasSameMajorAndMinor(const ProductVersion &other) const;

    bool operator==(const ProductVersion &rhs) const;
    bool operator!=(const ProductVersion &rhs) const;
    bool operator<(const ProductVersion &rhs) const;
    bool operator<=(const ProductVersion &rhs) const;
    bool operator>=(const ProductVersion &rhs) const;
    bool operator>(const ProductVersion &rhs) const;

private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
    uint16_t m_build;
};
