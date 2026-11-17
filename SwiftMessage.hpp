#pragma once

#include "FieldDefs.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <ranges>
#include <optional>
#include <expected>
#include <format>
#include <algorithm>
#include <cctype>

namespace swift {

struct Field {
    std::string tag;          // "20", "32A", ...
    std::string value;        // content (may contain newlines for multi-line)
    std::string_view raw;     // original view if available
};

struct Block {
    int id = 0;               // 1..5
    std::string content;      // full content inside the braces
};

struct SwiftMessage {
    std::vector<Block> blocks;
    std::vector<Field> fields;          // only from Block 4
    MessageType type = MessageType::Unknown;
    std::string original;

    // Convenience accessors
    std::optional<std::string_view> get_field(std::string_view tag) const {
        auto it = std::ranges::find_if(fields, [&](const Field& f) {
            return f.tag == tag;
        });
        if (it != fields.end()) return it->value;
        return std::nullopt;
    }

    bool has_field(std::string_view tag) const {
        return get_field(tag).has_value();
    }
};

// ---------------------------------------------------------------
// Parsing helpers using std::span + std::ranges
// ---------------------------------------------------------------

inline std::string_view trim(std::string_view sv) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    auto first = std::ranges::find_if(sv, not_space);
    if (first == sv.end()) return {};
    auto last = std::ranges::find_if(sv | std::views::reverse, not_space).base();
    return std::string_view{first, last};
}

// Split a string_view into lines (CRLF or LF) using ranges
inline auto split_lines(std::string_view text) {
    // Normalize: treat both \r\n and \n as separators
    return text
        | std::views::split('\n')
        | std::views::transform([](auto&& r) {
            std::string_view line{r.begin(), r.end()};
            if (!line.empty() && line.back() == '\r')
                line.remove_suffix(1);
            return line;
        });
}

// Extract blocks {n:...} from a full message using a simple state machine on span
inline std::expected<std::vector<Block>, std::string>
extract_blocks(std::string_view msg) {
    std::vector<Block> result;
    std::span<const char> data{msg.data(), msg.size()};

    size_t i = 0;
    while (i < data.size()) {
        // skip whitespace
        while (i < data.size() && std::isspace(static_cast<unsigned char>(data[i]))) ++i;
        if (i >= data.size()) break;

        if (data[i] != '{') {
            return std::unexpected(std::format("Expected '{{' at position {}", i));
        }
        ++i; // skip '{'

        if (i >= data.size() || !std::isdigit(static_cast<unsigned char>(data[i]))) {
            return std::unexpected("Invalid block identifier");
        }
        int block_id = data[i] - '0';
        ++i;

        if (i >= data.size() || data[i] != ':') {
            return std::unexpected("Expected ':' after block id");
        }
        ++i; // skip ':'

        // find matching '}'
        size_t start = i;
        int depth = 1;
        while (i < data.size() && depth > 0) {
            if (data[i] == '{') ++depth;
            else if (data[i] == '}') --depth;
            ++i;
        }
        if (depth != 0) {
            return std::unexpected(std::format("Unbalanced braces for block {}", block_id));
        }

        // content is [start, i-1)
        std::string content{data.begin() + start, data.begin() + (i - 1)};
        result.push_back(Block{block_id, std::move(content)});
    }

    return result;
}

// Parse fields inside Block 4 using ranges
inline std::expected<std::vector<Field>, std::string>
parse_text_block(std::string_view text) {
    std::vector<Field> fields;

    // Remove leading/trailing whitespace and the optional trailing "-"
    text = trim(text);
    if (text.ends_with('-')) {
        text.remove_suffix(1);
        text = trim(text);
    }

    // Split into candidate field lines
    auto lines = split_lines(text);

    std::string current_tag;
    std::string current_value;

    auto flush = [&]() {
        if (!current_tag.empty()) {
            fields.push_back(Field{
                .tag = current_tag,
                .value = current_value,
                .raw = {}
            });
            current_tag.clear();
            current_value.clear();
        }
    };

    for (std::string_view line : lines) {
        line = trim(line);
        if (line.empty()) continue;

        // Field start: :TAG:value
        if (line.starts_with(':') && line.size() > 2) {
            auto second_colon = line.find(':', 1);
            if (second_colon != std::string_view::npos && second_colon > 1) {
                flush();
                current_tag = std::string(line.substr(1, second_colon - 1));
                current_value = std::string(line.substr(second_colon + 1));
                continue;
            }
        }

        // Continuation line for multi-line fields
        if (!current_tag.empty()) {
            if (!current_value.empty()) current_value += '\n';
            current_value += line;
        }
    }
    flush();

    return fields;
}

// Main entry point
inline std::expected<SwiftMessage, std::string>
parse_message(std::string_view raw) {
    SwiftMessage msg;
    msg.original = std::string(raw);

    auto blocks_result = extract_blocks(raw);
    if (!blocks_result) return std::unexpected(blocks_result.error());

    msg.blocks = std::move(*blocks_result);

    // Detect type from block 2 if present
    auto it2 = std::ranges::find_if(msg.blocks, [](const Block& b) { return b.id == 2; });
    if (it2 != msg.blocks.end()) {
        msg.type = detect_message_type(it2->content);
    }

    // Parse text block (id 4)
    auto it4 = std::ranges::find_if(msg.blocks, [](const Block& b) { return b.id == 4; });
    if (it4 != msg.blocks.end()) {
        auto fields_result = parse_text_block(it4->content);
        if (!fields_result) return std::unexpected(fields_result.error());
        msg.fields = std::move(*fields_result);
    }

    return msg;
}

// Pretty print using ranges
inline std::string pretty_print(const SwiftMessage& msg) {
    std::string out;

    out += std::format("=== SWIFT Message (detected: {}) ===\n",
        msg.type == MessageType::MT103 ? "MT103" :
        msg.type == MessageType::MT202 ? "MT202" : "Unknown");

    for (const auto& b : msg.blocks) {
        out += std::format("\n--- Block {} ---\n{}\n", b.id, b.content);
    }

    if (!msg.fields.empty()) {
        out += "\n--- Parsed Fields (Block 4) ---\n";
        for (const auto& f : msg.fields) {
            auto info = find_field(f.tag);
            if (info) {
                out += std::format(":{:4}  {}  [{}]\n", f.tag, f.value, info->name);
            } else {
                out += std::format(":{:4}  {}\n", f.tag, f.value);
            }
        }
    }
    return out;
}

// Basic validation focused on interview-relevant rules
inline std::vector<std::string> validate(const SwiftMessage& msg) {
    std::vector<std::string> issues;

    if (msg.type == MessageType::Unknown) {
        issues.push_back("Could not detect message type (looked for 103/202 in Block 2)");
    }

    // Mandatory fields for MT103 / MT202 (simplified)
    const std::vector<std::string_view> mandatory_mt103 = {
        "20", "23B", "32A", "50K", "59", "71A"   // 50K is one common option; real world is more complex
    };

    if (msg.type == MessageType::MT103) {
        for (auto tag : mandatory_mt103) {
            bool found = std::ranges::any_of(msg.fields, [&](const Field& f) {
                return f.tag == tag || f.tag.starts_with(tag);
            });
            if (!found) {
                issues.push_back(std::format("Missing mandatory field :{}: for MT103", tag));
            }
        }
    }

    // Basic format sanity for :32A:
    if (auto amt = msg.get_field("32A")) {
        if (amt->size() < 9) {
            issues.push_back(":32A: looks too short (expected YYMMDD + CCY + amount)");
        }
    }

    // :20: should be present and not empty
    if (auto ref = msg.get_field("20")) {
        if (ref->empty()) issues.push_back(":20: is empty");
    } else {
        issues.push_back("Missing :20: Sender's Reference");
    }

    return issues;
}

} // namespace swift
