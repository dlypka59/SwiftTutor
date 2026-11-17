#pragma once

#include "SwiftMessage.hpp"
#include "FieldDefs.hpp"   // NOLINT(unused-includes)
#include "Samples.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <ranges>   // NOLINT(unused-includes)
#include <format>

namespace swift {
namespace tutor {

inline void print_header(std::string_view title) {
    std::cout << "\n==================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "==================================================\n\n";
}

inline void lesson_blocks() {
    print_header("Lesson 1 - What is SWIFT MT and the Message Structure");

    std::cout << R"(
================================================================
1. Important acronyms you must know
================================================================

SWIFT  = Society for Worldwide Interbank Financial Telecommunication
         The global network banks use to send payment instructions.

MT     = Message Type
         The old (but still very important) format of SWIFT messages.
         Example: MT103, MT202.

MX     = The newer XML-based format (ISO 20022).
         Example: pacs.008 is the modern replacement for MT103.

FIN    = The traditional SWIFT messaging service that carries MT messages.

UETR   = Unique End-to-end Transaction Reference
         A UUID that stays with the payment from start to finish.
         It lives in Block 3 (tag 121).

================================================================
2. The two message types you must know for interviews
================================================================

MT103  = Customer Credit Transfer
         Used when a bank is sending money on behalf of a customer
         (the most common payment message).

MT202  = General Financial Institution Transfer
         Used for bank-to-bank transfers (often the "cover" payment
         that moves the money between banks for an MT103).

================================================================
3. The 5 blocks of an MT message
================================================================

Every classic SWIFT MT message is made of up to 5 blocks:

  {1: ...}   Basic Header          - who is sending the message
  {2: ...}   Application Header    - message type + receiver + priority
  {3: ...}   User Header           - optional but almost always present today
                                     (contains the UETR)
  {4: ...}   Text Block            - the real business information (the fields)
  {5: ...}   Trailer               - checksum and control information

================================================================
4. Our running example (we will use these same values everywhere)
================================================================

We will always use this simple realistic payment:

  Sender's Reference : REF20260816-001
  Value Date         : 16 August 2026
  Currency + Amount  : EUR 250,000.00
  Ordering Customer  : ACME TRADING BV
                       Account: BE68539007547034
                       Address: RUE DE LA LOI 100, 1000 BRUSSELS
  Beneficiary        : JOHN DOE GMBH
                       Account: DE89370400440532013000
                       Address: FRIEDRICHSTRASSE 123, 10117 BERLIN
  Charges            : SHA (shared)
  Remittance Info    : /INV/2026-0842

This exact payment appears in the sample MT103 you can view from the menu.
)" << "\n";
}

inline void lesson_mt103_fields() {
    print_header("Lesson 2 - Essential MT103 Fields Explained in Plain English");

    std::cout << R"(
We use the same running example payment from Lesson 1.

================================================================
How to read the format codes (this is the cryptic part explained)
================================================================

SWIFT uses short codes to describe the allowed format of each field:

  16x     = up to 16 characters of any allowed SWIFT character
  4!c     = exactly 4 uppercase letters (the ! means "exactly")
  6!n     = exactly 6 digits
  3!a     = exactly 3 letters
  15d     = up to 15 digits (the amount part)
  3!a15d  = exactly 3 letters + up to 15 digits

So the famous format for field 32A is written as:

  6!n3!a15d

This means:
  - Exactly 6 digits        (the date YYMMDD)
  - Exactly 3 letters       (the currency code, e.g. EUR)
  - Up to 15 digits         (the amount, using a COMMA as the decimal point)

Example from our payment:
  :32A:260816EUR250000,00
        ^^^^^^ ^^^ ^^^^^^^^
        date   cur  amount

================================================================
The most important fields (with our running example)
================================================================

:20:  Sender's Reference
      Format: 16x  (up to 16 characters)
      Meaning: A unique reference the sending bank gives this payment.
      Our example: REF20260816-001

:23B: Bank Operation Code
      Format: 4!c  (exactly 4 letters)
      Meaning: What kind of transfer this is.
      Almost always: CRED  (normal credit transfer)
      Our example: CRED

:32A: Value Date / Currency / Amount
      Format: 6!n3!a15d
      Meaning: When the money should be valued + which currency + how much.
      Our example: 260816EUR250000,00
                   (16 Aug 2026, EUR 250,000.00)
      Important: The decimal separator is a COMMA, not a dot.

:50K: Ordering Customer (Name & Address)
      This is the customer who is sending the money.
      Our example:
        /BE68539007547034          <- account number
        ACME TRADING BV
        RUE DE LA LOI 100
        1000 BRUSSELS

:59:  Beneficiary Customer
      This is the customer who will receive the money.
      Our example:
        /DE89370400440532013000    <- account number
        JOHN DOE GMBH
        FRIEDRICHSTRASSE 123
        10117 BERLIN

:71A: Details of Charges
      Format: 3!a
      Meaning: Who pays the bank charges.
        OUR  = sender pays all charges
        BEN  = beneficiary pays all charges
        SHA  = charges are shared (most common for international payments)
      Our example: SHA

:70:  Remittance Information
      Free text that tells the beneficiary what the payment is for.
      Our example: /INV/2026-0842   (invoice number)

:72:  Sender to Receiver Information
      Bank-to-bank information (instructions between banks).
      Our example: /ACC/BERLIN BRANCH

================================================================
Quick memory list for interviews
================================================================

:20:   Your unique reference
:23B:  Almost always CRED
:32A:  Date + Currency + Amount (comma!)
:50a:  Who is sending the money (Ordering Customer)
:59a:  Who is receiving the money (Beneficiary)
:71A:  OUR / BEN / SHA
)" << "\n";
}

inline void show_sample(std::string_view name, std::string_view raw) {
    print_header(std::format("Sample: {}", name));
    auto result = parse_message(raw);
    if (!result) {
        std::cout << "Parse error: " << result.error() << "\n";
        return;
    }
    std::cout << pretty_print(*result) << "\n";

    auto issues = validate(*result);
    if (issues.empty()) {
        std::cout << "[OK] Basic validation passed\n";
    } else {
        std::cout << "Validation notes:\n";
        for (const auto& i : issues) std::cout << "  * " << i << "\n";
    }
}

inline void interactive_parse() {
    print_header("Interactive Parse Mode");
    std::cout << "Paste a full SWIFT message (or just Block 4) and press Enter twice:\n\n";

    std::string line, full;
    while (std::getline(std::cin, line)) {
        if (line.empty() && !full.empty()) break;
        full += line + "\n";
    }

    auto result = parse_message(full);
    if (!result) {
        std::cout << "\nParse failed: " << result.error() << "\n";
        return;
    }

    std::cout << "\n" << pretty_print(*result) << "\n";
    auto issues = validate(*result);
    if (!issues.empty()) {
        std::cout << "\nIssues found:\n";
        for (const auto& i : issues) std::cout << "  * " << i << "\n";
    } else {
        std::cout << "\n[OK] Looks structurally sound\n";
    }
}

inline void quiz() {
    print_header("Quiz Mode - SWIFT Interview Drill");

    struct Question {
        std::string prompt;
        std::string answer;
        std::string explanation;
    };

    std::vector<Question> bank = {
        {"What is the tag for Sender's Reference?", "20", "Always present; unique per sender."},
        {"What does field 23B usually contain for a normal credit transfer?", "CRED", "CRED = no special SWIFT service level."},
        {"Which field holds Value Date + Currency + Amount?", "32A", "Format: 6!n3!a15d  (e.g. 260816USD1000,00)"},
        {"Who pays the charges when :71A:OUR is used?", "Sender", "OUR = ordering customer pays all charges."},
        {"Where does the UETR live in an MT message?", "121", "In Block 3 (User Header), tag 121."},
        {"What is the ISO 20022 / MX equivalent of MT103?", "pacs.008", "FIToFICustomerCreditTransfer"},
        {"What is the ISO 20022 / MX equivalent of MT202?", "pacs.009", "FinancialInstitutionCreditTransfer"},
        {"What character is used as decimal separator in MT amounts?", ",", "European style: 1000,50 not 1000.50"},
        {"Which block contains the business fields (:20:, :32A:, ...)?", "4", "Text / Body block"},
        {"What does SHA mean in :71A:?", "Shared", "Charges shared between ordering and beneficiary sides."},
        {"In MX, what does the charge code SHAR correspond to in MT?", "SHA", "SHAR = SHA (shared charges)"},
        {"When did the MT/MX coexistence period for payment instructions end?", "2025", "22 November 2025"},
        {"Is Block 3 mandatory for modern MT103?", "Yes (practically)", "Because of the mandatory UETR (tag 121)."}
    };

    std::ranges::shuffle(bank, std::mt19937{std::random_device{}()});

    int score = 0;
    const int total = std::min(8, static_cast<int>(bank.size()));

    for (int i = 0; i < total; ++i) {
        const auto& q = bank[i];
        std::cout << "\nQ" << (i + 1) << ": " << q.prompt << "\n";
        std::cout << "Your answer: ";
        std::string ans;
        std::getline(std::cin, ans);

        auto lower = [](std::string s) {
            std::ranges::transform(s, s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return s;
        };
        if (lower(ans).find(lower(q.answer)) != std::string::npos ||
            lower(q.answer).find(lower(ans)) != std::string::npos) {
            std::cout << "[OK] Correct\n";
            ++score;
        } else {
            std::cout << "[X] Expected something like: " << q.answer << "\n";
        }
        std::cout << "  -> " << q.explanation << "\n";
    }

    std::cout << "\nScore: " << score << " / " << total << "\n";
    if (score == total) std::cout << "Excellent - interview ready on these points.\n";
    else if (score >= total * 0.7) std::cout << "Solid. Review the ones you missed.\n";
    else std::cout << "Keep drilling - focus on the core tags.\n";
}

inline void guided_build() {
    print_header("Guided MT103 Builder (simplified)");

    std::string ref, amount, currency, ordering, beneficiary, charges;

    std::cout << "Sender's Reference (:20:): ";
    std::getline(std::cin, ref);

    std::cout << "Currency (e.g. USD): ";
    std::getline(std::cin, currency);

    std::cout << "Amount (use comma, e.g. 12500,00): ";
    std::getline(std::cin, amount);

    std::cout << "Ordering Customer name: ";
    std::getline(std::cin, ordering);

    std::cout << "Beneficiary name: ";
    std::getline(std::cin, beneficiary);

    std::cout << "Charges (OUR/BEN/SHA): ";
    std::getline(std::cin, charges);
    if (charges.empty()) charges = "SHA";

    // Build a minimal realistic message
    std::string msg = std::format(
R"({{1:F01YOURBANKXXXX0000000000}}{{2:I103BENEBANKXXXXN}}{{3:{{108:MUR{0}}}{{121:00000000-0000-4000-8000-000000000000}}}}{{4:
:20:{0}
:23B:CRED
:32A:260816{1}{2}
:50K:{3}
:59:{4}
:71A:{5}
-}}{{5:{{CHK:000000000000}}}})",
        ref, currency, amount, ordering, beneficiary, charges);

    std::cout << "\nGenerated message:\n" << msg << "\n";

    auto result = parse_message(msg);
    if (result) {
        std::cout << "\n" << pretty_print(*result) << "\n";
    }
}

// ---------------------------------------------------------------
// MX / ISO 20022 support (interview-focused)
// ---------------------------------------------------------------

inline void lesson_mx() {
    print_header("Lesson - SWIFT MX (ISO 20022)");

    std::cout << R"(
MX = the modern XML-based message format based on the ISO 20022 standard.

Key facts for interviews (as of 2026):

* Coexistence of MT and MX for cross-border payments ended 22 Nov 2025.
* Since then, FI-to-FI payment instructions must be sent as MX on FINplus.
* Native MX adoption quickly reached ~97%.
* Remaining MT payment traffic uses chargeable contingency conversion.

Main mappings you should know:

  MT103  ->  pacs.008   (FI-to-FI Customer Credit Transfer)
  MT202  ->  pacs.009   (Financial Institution Credit Transfer)
  MT202 COV -> pacs.009 COV
  MT940/950 -> camt.053 (Account Statement)  - migration slower

Important differences:
* MT uses flat :TAG:value fields
* MX uses hierarchical, structured XML (much richer data)
* UETR moves from Block 3 {121:} into <PmtId><UETR>
* Charges: OUR->DEBT, BEN->CRED, SHA->SHAR
* Structured postal addresses are strongly preferred / becoming mandatory

In an interview it is impressive to say:
  "I know the industry completed the move of core payment flows to ISO 20022
   MX in November 2025. MT103 maps to pacs.008 and MT202 maps to pacs.009."
)" << "\n";
}

inline void show_mx_mapping() {
    print_header("MT103 <-> pacs.008 Key Mapping");

    std::cout << R"(
MT103 Field          ->  pacs.008 Element
----------------------------------------------------------
:20:  Sender's Ref   ->  GrpHdr/MsgId  or  PmtId/InstrId
:23B: Bank Op Code   ->  PmtTpInf / LclInstrm
:32A: Value Date/Amt ->  IntrBkSttlmDt + IntrBkSttlmAmt
:33B: Instructed Amt ->  InstdAmt
:50a: Ordering Cust  ->  Dbtr + DbtrAcct
:52a: Ordering Inst  ->  DbtrAgt
:56a: Intermediary   ->  IntrmyAgt1
:57a: Account With   ->  CdtrAgt
:59a: Beneficiary    ->  Cdtr + CdtrAcct
:70:  Remittance     ->  RmtInf/Ustrd  (or structured)
:71A: Charges        ->  ChrgBr   (OUR->DEBT, BEN->CRED, SHA->SHAR)
:72:  Sender-to-Rec  ->  InstrForCdtrAgt / InstrForNxtAgt
Block 3 {121:} UETR  ->  PmtId/UETR

Note: MX also has EndToEndId (no direct MT equivalent).
)" << "\n";
}

inline void show_pacs008_sample() {
    print_header("Sample pacs.008 (MX equivalent of MT103)");
    std::cout << samples::pacs008_sample << "\n\n";
    std::cout << "This is a simplified but realistic pacs.008.001.08.\n";
    std::cout << "Real CBPR+ messages also include a Business Application Header (BAH).\n";
}

inline void run_menu() {
    while (true) {
        print_header("SWIFT Interview Tutor (C++23)");
        std::cout << "1. Lesson - Message Blocks\n";
        std::cout << "2. Lesson - Essential MT103 Fields\n";
        std::cout << "3. Show sample MT103\n";
        std::cout << "4. Show sample MT202\n";
        std::cout << "5. Interactive Parse (MT)\n";
        std::cout << "6. Guided Build (MT103)\n";
        std::cout << "7. Quiz\n";
        std::cout << "8. Lesson - SWIFT MX / ISO 20022\n";
        std::cout << "9. MT103 <-> pacs.008 Mapping\n";
        std::cout << "A. Show sample pacs.008 (MX)\n";
        std::cout << "0. Exit\n";
        std::cout << "\nChoice: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") lesson_blocks();
        else if (choice == "2") lesson_mt103_fields();
        else if (choice == "3") show_sample("Canonical MT103", samples::mt103_good);
        else if (choice == "4") show_sample("Canonical MT202", samples::mt202_good);
        else if (choice == "5") interactive_parse();
        else if (choice == "6") guided_build();
        else if (choice == "7") quiz();
        else if (choice == "8") lesson_mx();
        else if (choice == "9") show_mx_mapping();
        else if (choice == "A" || choice == "a") show_pacs008_sample();
        else if (choice == "0") break;
        else std::cout << "Unknown option\n";
    }
}

} // namespace tutor
} // namespace swift
