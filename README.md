# SWIFT Interview Tutor (C++23)

A lightweight console application that teaches the most interview-relevant parts of SWIFT messaging (MT103 / MT202 + MX / ISO 20022 awareness).

## Features

- Clear lessons on message block structure and essential MT fields
- Sample MT103 and MT202 messages with annotated parse output
- Interactive parse mode (paste a real or practice message)
- Guided message builder
- Quiz mode focused on tags, formats, and common interview questions
- Basic structural validation
- **MX / ISO 20022 support** (updated for 2026):
  - Lesson on the MT → MX migration (ended Nov 2025)
  - MT103 ↔ pacs.008 key mapping table
  - Sample pacs.008 message
  - Quiz questions covering the migration and mappings

## Modern C++ usage

- `std::span` for safe non-owning views of message data
- `std::ranges` + views for line splitting, searching, and shuffling
- `std::expected` for parse results
- `std::string_view`, `std::format`, `std::print`
- Header-only design for easy integration into a Visual Studio project

## Building with Visual Studio 2022 / 2025 / 2026

1. Create a new **Empty Project** (Console).
2. Add all `.hpp` and `main.cpp` files.
3. Project Properties → C/C++ → Language → **C++ Language Standard** = **ISO C++23 Standard** (`/std:c++23` or `/std:c++latest`).
4. Build & run.

Alternatively with CMake:

```bash
cmake -B build -S .
cmake --build build
./build/SwiftTutor
```

## Notes for the interview

This tool deliberately stays focused on the knowledge that actually appears in technical interviews:

- Block structure (`{1:}…{5:}`)
- Meaning and format of the most common tags (`:20:`, `:23B:`, `:32A:`, `:50a:`, `:59a:`, `:71A:`, …)
- Difference between MT103 (customer) and MT202 (bank-to-bank / cover)
- Existence of UETR (tag 121 in Block 3)
- High-level awareness that MT103 ≈ pacs.008 in ISO 20022

It is **not** a production SWIFT engine.
