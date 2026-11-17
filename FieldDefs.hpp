#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>
#include <span>

namespace swift {

enum class MessageType {
    Unknown,
    MT103,
    MT202
};

struct FieldInfo {
    std::string_view tag;           // e.g. "20", "32A"
    std::string_view name;          // human readable
    std::string_view format;        // e.g. "16x", "6!n3!a15d"
    bool mandatory;                 // for the primary message types we care about
    std::string_view description;
};

// Core knowledge base for the tutor (interview-focused)
inline const std::vector<FieldInfo>& common_fields() {
    static const std::vector<FieldInfo> fields = {
        {"20",  "Sender's Reference",               "16x",          true,  "Unique reference assigned by the sender"},
        {"23B", "Bank Operation Code",              "4!c",          true,  "CRED = normal credit transfer (most common)"},
        {"32A", "Value Date/Currency/Amount",       "6!n3!a15d",    true,  "YYMMDD + Currency + Amount (comma as decimal)"},
        {"50A", "Ordering Customer (BIC)",          "options A/F/K",true,  "Ordering customer identification"},
        {"50F", "Ordering Customer (structured)",   "options A/F/K",true,  "Structured name & address"},
        {"50K", "Ordering Customer (name/addr)",    "options A/F/K",true,  "Name and address of ordering customer"},
        {"59",  "Beneficiary Customer",             "options",      true,  "Beneficiary account + name/address"},
        {"59A", "Beneficiary Customer (BIC)",       "options",      true,  "Beneficiary with BIC"},
        {"59F", "Beneficiary Customer (structured)","options",      true,  "Structured beneficiary data"},
        {"71A", "Details of Charges",               "3!a",          true,  "OUR / BEN / SHA"},
        {"70",  "Remittance Information",           "4*35x",        false, "Payment purpose / invoice refs"},
        {"72",  "Sender to Receiver Information",   "6*35x",        false, "Bank-to-bank free text / codes"},
        {"21",  "Related Reference",                "16x",          false, "Reference of related message (MT202)"},
        {"13C", "Time Indication",                  "/8c/...",      false, "Time related to processing"},
        {"23E", "Instruction Code",                 "4!c[/30x]",    false, "Additional instructions"},
        {"26T", "Transaction Type Code",            "3!c",          false, "Nature of transaction"},
        {"33B", "Currency/Instructed Amount",       "3!a15d",       false, "Original instructed amount"},
        {"36",  "Exchange Rate",                    "12d",          false, "Rate used"},
        {"52A", "Ordering Institution",             "A or D",       false, "Ordering bank"},
        {"53A", "Sender's Correspondent",           "A/B/D",        false, "Correspondent of sender"},
        {"54A", "Receiver's Correspondent",         "A/B/D",        false, "Correspondent of receiver"},
        {"56A", "Intermediary Institution",         "A/C/D",        false, "Intermediary bank"},
        {"57A", "Account With Institution",         "A/B/C/D",      false, "Account with bank"},
        {"71F", "Sender's Charges",                 "3!a15d",       false, "Charges deducted by sender"},
        {"71G", "Receiver's Charges",               "3!a15d",       false, "Charges of receiver"},
        {"77B", "Regulatory Reporting",             "3*35x",        false, "Regulatory information"}
    };
    return fields;
}

inline std::optional<FieldInfo> find_field(std::string_view tag) {
    for (const auto& f : common_fields()) {
        if (f.tag == tag) return f;
    }
    // also try without letter option
    if (tag.size() > 2) {
        auto base = tag.substr(0, 2);
        for (const auto& f : common_fields()) {
            if (f.tag.starts_with(base)) return f;
        }
    }
    return std::nullopt;
}

inline MessageType detect_message_type(std::string_view block2) {
    // very lightweight detection from Application Header
    if (block2.contains("103")) return MessageType::MT103;
    if (block2.contains("202")) return MessageType::MT202;
    return MessageType::Unknown;
}

} // namespace swift
