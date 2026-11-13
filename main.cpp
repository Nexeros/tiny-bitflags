#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "flag_locator.hpp"

// Demonstrates packing, inspecting, unpacking, and verifying `names.size()` flags
// using a `Flag::Pack` backed by an integer of `Bits` width (8, 16, 32, or 64).
template <unsigned Bits>
void RunDemo(const std::vector<std::string>& names, const std::vector<uint8_t>& values) {
    // Since flag indices come from user input at runtime, we pass `uint8_t` as the index type
    // and specify the bit capacity explicitly. (In static code, prefer an `enum class`.)
    using PackT = Flag::Pack<uint8_t, Bits>;

    // 1. Pack each boolean value into its corresponding bit inside `PackT`.
    PackT packed;
    for (uint8_t i = 0; i < names.size(); ++i) {
        packed.Set(i, values[i] != 0);
    }

    // 2. Extract the underlying packed unsigned integer (`uint8_t`, `uint16_t`, `uint32_t`, or `uint64_t`).
    const auto raw = packed.Raw();

    std::cout << "\n--- PACKED ---\n";
    std::cout << "Memory (bool[]): " << values.size() * sizeof(bool) << " B -> "
              << "uint" << Bits << "_t: " << sizeof(raw) << " B\n";
    // Cast `raw` to `uint64_t` so `uint8_t` prints as a number rather than an ASCII character.
    std::cout << "Raw uint value:  " << static_cast<uint64_t>(raw) << "\n";
    std::cout << "Binary bits:     0b";
    for (int b = static_cast<int>(names.size()) - 1; b >= 0; --b) {
        std::cout << ((raw >> b) & 1U);
    }
    std::cout << "\n";

    // 3. Unpack the raw integer back into a `Flag::Pack` instance.
    const PackT unpacked = PackT::FromRaw(raw);

    // 4. Read each bit via `Has(index)` and compare it against the original input.
    std::cout << "\n--- UNPACKED ---\n";
    bool all_equal = true;
    for (uint8_t i = 0; i < names.size(); ++i) {
        const bool before = values[i] != 0;
        const bool after = unpacked.Has(i);
        if (before != after) {
            all_equal = false;
        }
        std::cout << names[i] << ": " << after << " (was: " << before << ")\n";
    }

    std::cout << "\nDo unpacked values match original input? " << (all_equal ? "YES" : "NO") << "\n";
}

int main() {
    // Ask the user how many flags to create (up to 64, the max supported by `Flag::Pack`).
    unsigned count = 0;
    std::cout << "How many flags do you want to enter (1-64)? ";
    if (!(std::cin >> count) || count < 1 || count > 64) {
        std::cerr << "Invalid flag count.\n";
        return 1;
    }

    // Store names and initial boolean values temporarily for the interactive comparison.
    std::vector<std::string> names(count);
    std::vector<uint8_t> values(count);

    std::cout << "\nEnter flag names:\n";
    for (unsigned i = 0; i < count; ++i) {
        std::cout << "Flag [" << i << "]: ";
        std::cin >> names[i];
    }

    std::cout << "\nEnter flag values (0 or 1):\n";
    for (unsigned i = 0; i < count; ++i) {
        int v = 0;
        std::cout << names[i] << " = ";
        std::cin >> v;
        values[i] = (v != 0) ? 1 : 0;
    }

    // Dispatch to the smallest `Flag::Pack` size that fits `count` flags.
    if (count <= 8)       RunDemo<8>(names, values);
    else if (count <= 16) RunDemo<16>(names, values);
    else if (count <= 32) RunDemo<32>(names, values);
    else                  RunDemo<64>(names, values);

    return 0;
}
