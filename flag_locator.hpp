#pragma once

#include <cstdint>

namespace Flag {

namespace detail {
// Selects the smallest unsigned integer type that fits `Bits` at compile time.
template <unsigned Bits>
consteval auto PickStorage() noexcept {
    if constexpr (Bits <= 8)       return uint8_t{};
    else if constexpr (Bits <= 16) return uint16_t{};
    else if constexpr (Bits <= 32) return uint32_t{};
    else                           return uint64_t{};
}
} // namespace detail

// Compact bit-packed container for 1..64 boolean flags stored in a single uint8/16/32/64_t.
// `Enum` should define `Count` as its last enumerator, or pass the flag count explicitly.
template <typename Enum, unsigned Count = static_cast<unsigned>(Enum::Count)>
class Pack {
    static_assert(Count >= 1 && Count <= 64, "Flag::Pack supports 1..64 flags");

public:
    using Storage = decltype(detail::PickStorage<Count>());

    constexpr Pack() noexcept = default; // Initializes all flags to 0

    // Initializes with one or more flags enabled, e.g. Pack{F::A, F::B}
    template <typename... Ts>
    constexpr explicit Pack(Enum first, Ts... rest) noexcept
        : bits_(static_cast<Storage>((Mask(first) | ... | Mask(rest)))) {}

    // Unpacks flags from a raw unsigned integer
    [[nodiscard]] static constexpr Pack FromRaw(Storage raw) noexcept {
        Pack p;
        p.bits_ = raw;
        return p;
    }

    // Sets the given flag to 1
    constexpr void Set(Enum flag) noexcept {
        bits_ = static_cast<Storage>(bits_ | Mask(flag));
    }

    // Sets the given flag to `value` (true/false)
    constexpr void Set(Enum flag, bool value) noexcept {
        const Storage m = Mask(flag);
        bits_ = value ? static_cast<Storage>(bits_ | m)
                      : static_cast<Storage>(bits_ & static_cast<Storage>(~m));
    }

    // Clears the given flag (sets to 0)
    constexpr void Clear(Enum flag) noexcept {
        bits_ = static_cast<Storage>(bits_ & static_cast<Storage>(~Mask(flag)));
    }

    // Flips the given flag (0 <-> 1)
    constexpr void Toggle(Enum flag) noexcept {
        bits_ = static_cast<Storage>(bits_ ^ Mask(flag));
    }

    // Clears all flags (sets all bits to 0)
    constexpr void ClearAll() noexcept { bits_ = 0; }

    // Returns true if the given flag is set
    [[nodiscard]] constexpr bool Has(Enum flag) const noexcept {
        return (bits_ & Mask(flag)) != 0;
    }

    // Returns true if at least one flag from `other` is set
    [[nodiscard]] constexpr bool HasAny(Pack other) const noexcept {
        return (bits_ & other.bits_) != 0;
    }

    // Returns true if all flags from `other` are set
    [[nodiscard]] constexpr bool HasAll(Pack other) const noexcept {
        return (bits_ & other.bits_) == other.bits_;
    }

    [[nodiscard]] constexpr bool Any() const noexcept { return bits_ != 0; }  // True if any bit is set
    [[nodiscard]] constexpr bool None() const noexcept { return bits_ == 0; } // True if all bits are 0
    [[nodiscard]] constexpr Storage Raw() const noexcept { return bits_; }    // Returns raw packed uint

    constexpr bool operator==(const Pack&) const noexcept = default;

private:
    static constexpr Storage Mask(Enum flag) noexcept {
        return static_cast<Storage>(Storage{1} << static_cast<unsigned>(flag));
    }

    Storage bits_ = 0; // Single underlying integer (1, 2, 4, or 8 bytes)
};

} // namespace Flag

/*
=== QUICK USAGE ===

1. Define flags as an enum with `Count` at the end:
    enum class MyFlag : uint8_t {
        OptionA,
        OptionB,
        OptionC,
        Count
    };

2. Store and manipulate flags (packed into the smallest uint):
    Flag::Pack<MyFlag> flags{MyFlag::OptionA}; // sizeof(flags) == 1 byte

    flags.Set(MyFlag::OptionB);
    flags.Set(MyFlag::OptionA, false);
    flags.Toggle(MyFlag::OptionC);

    if (flags.Has(MyFlag::OptionB)) { ... }

3. Pack / unpack raw integer (e.g. for storage or serialization):
    uint8_t raw = flags.Raw();
    auto restored = Flag::Pack<MyFlag>::FromRaw(raw);

4. Without `Count`, pass the bit count explicitly:
    Flag::Pack<SomeEnum, 12> flags12; // automatically uses uint16_t (2 bytes)
*/
