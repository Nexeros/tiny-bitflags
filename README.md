# tiny-bitflags

Lightweight single-header C++20 library for packing up to 64 boolean flags into the smallest unsigned integer (`uint8_t`–`uint64_t`).

## Why `tiny-bitflags`?

Storing multiple `bool` fields in a struct wastes memory (1 byte per `bool`, plus padding). `tiny-bitflags` packs up to 64 flags into a single CPU register-sized unsigned integer while keeping type-safe, readable code.

- **Single header, zero dependencies** — only includes `<cstdint>`.
- **Automatic minimal storage** — picks `uint8_t`, `uint16_t`, `uint32_t`, or `uint64_t` at compile time.
- **Zero runtime overhead** — `constexpr` and `noexcept` bitwise operations (`OR`, `AND`, `XOR`).
- **Easy serialization** — convert back and forth between `Flag::Pack` and a raw `uint` via `Raw()` / `FromRaw()`.

## Storage Size Table

| Number of Flags | Underlying Type | Size in Memory |
|---|---|---|
| `1 – 8` | `uint8_t` | `1 byte` |
| `9 – 16` | `uint16_t` | `2 bytes` |
| `17 – 32` | `uint32_t` | `4 bytes` |
| `33 – 64` | `uint64_t` | `8 bytes` |

---

## Installation

Copy [`flag_locator.hpp`](flag_locator.hpp) into your project and include it.  
Requires **C++20** or newer.

```cpp
#include "flag_locator.hpp"
```

---

## How to Use

### 1. Define your flags as an `enum class`

Add `Count` as the last enumerator so the library automatically knows how many bits you need:

```cpp
#include "flag_locator.hpp"

enum class MyFlag : uint8_t {
    OptionA,
    OptionB,
    OptionC,
    OptionD,
    Count // Keep this last!
};
```

> **Tip:** If your enum cannot have a `Count` entry, pass the flag count explicitly as the second template parameter:
> ```cpp
> Flag::Pack<SomeEnum, 12> flags; // Uses uint16_t (2 bytes)
> ```

### 2. Create and manipulate flags

```cpp
// Default: all flags are 0 (false)
Flag::Pack<MyFlag> empty_flags;

// Or initialize with specific flags turned on:
Flag::Pack<MyFlag> flags{MyFlag::OptionA, MyFlag::OptionC};

// Turn flags on / off / flip
flags.Set(MyFlag::OptionB);          // OptionB = true
flags.Set(MyFlag::OptionA, false);   // OptionA = false
flags.Clear(MyFlag::OptionC);        // OptionC = false
flags.Toggle(MyFlag::OptionD);       // Flips OptionD (0 <-> 1)
flags.ClearAll();                    // Resets all flags to 0
```

### 3. Check flags

```cpp
if (flags.Has(MyFlag::OptionB)) {
    // OptionB is set
}

// Check multiple flags at once (single bitwise instruction)
Flag::Pack<MyFlag> required{MyFlag::OptionA, MyFlag::OptionB};
bool has_all = flags.HasAll(required); // true if BOTH OptionA and OptionB are set
bool has_any = flags.HasAny(required); // true if AT LEAST ONE is set

bool any_set  = flags.Any();  // true if at least one flag is 1
bool all_zero = flags.None(); // true if all flags are 0
```

### 4. Pack & Unpack (save files, network packets, etc.)

```cpp
// Extract the raw packed integer (here: uint8_t)
uint8_t raw = flags.Raw();

// Restore flags from a raw integer
auto restored = Flag::Pack<MyFlag>::FromRaw(raw);

// Compare two packs
bool same = (flags == restored); // true
```

---

## Building and Running the Example (`main.cpp`)

The included [`main.cpp`](main.cpp) is an interactive CLI demo that asks you for flag names and `0`/`1` values, packs them into the smallest `uint`, prints the memory savings and binary bits, unpacks them, and verifies that every bit matches.

### Option A: Quick compile with `g++` or `clang++` (easiest)

From the repository root:

```bash
g++ -std=c++20 -O2 main.cpp -o tiny-bitflags-demo
./tiny-bitflags-demo
```

*(With Clang, replace `g++` with `clang++`.)*

### Option B: Build with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/untitled
```

### Example Run

```text
How many flags do you want to enter (1-64)? 3

Enter flag names:
Flag [0]: flag_a
Flag [1]: flag_b
Flag [2]: flag_c

Enter flag values (0 or 1):
flag_a = 1
flag_b = 0
flag_c = 1

--- PACKED ---
Memory (bool[]): 3 B -> uint8_t: 1 B
Raw uint value:  5
Binary bits:     0b101

--- UNPACKED ---
flag_a: 1 (was: 1)
flag_b: 0 (was: 0)
flag_c: 1 (was: 1)

Do unpacked values match original input? YES
```

---

## License

This project is licensed under the [MIT License](LICENSE).
