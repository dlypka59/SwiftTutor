#pragma once

#include <string_view>

namespace swift {
namespace samples {

// Clean, interview-friendly MT103 example
inline constexpr std::string_view mt103_good = R"({1:F01BANKBEBBAXXX0000000000}{2:I103BANKDEFFXXXXN}{3:{108:MUR20260816}{121:f47ac10b-58cc-4372-a567-0e02b2c3d479}}{4:
:20:REF20260816-001
:23B:CRED
:32A:260816EUR250000,00
:50K:/BE68539007547034
ACME TRADING BV
RUE DE LA LOI 100
1000 BRUSSELS
:59:/DE89370400440532013000
JOHN DOE GMBH
FRIEDRICHSTRASSE 123
10117 BERLIN
:70:/INV/2026-0842
:71A:SHA
:72:/ACC/BERLIN BRANCH
-}{5:{CHK:123456789ABC}})";

// Clean MT202 example
inline constexpr std::string_view mt202_good = R"({1:F01BANKBEBBAXXX0000000000}{2:I202BANKDEFFXXXXN}{3:{108:MUR20260816B}{121:a1b2c3d4-e5f6-7890-abcd-ef1234567890}}{4:
:20:COV20260816-001
:21:REF20260816-001
:32A:260816EUR250000,00
:52A:BANKBEBB
:58A:BANKDEFF
:72:/BNF/COVER FOR CUSTOMER TRANSFER
-}{5:{CHK:ABCDEF123456}})";

// Deliberately incomplete for validation practice
inline constexpr std::string_view mt103_missing_fields = R"({1:F01BANKBEBBAXXX0000000000}{2:I103BANKDEFFXXXXN}{4:
:20:TESTREF
:32A:260816USD1000,00
:59:BENEFICIARY ONLY
-}{5:{CHK:000000000000}})";

// Simplified but realistic pacs.008.001.08 (MX equivalent of MT103)
// Note: Real CBPR+ messages also carry a Business Application Header (BAH)
inline constexpr std::string_view pacs008_sample = R"(<?xml version="1.0" encoding="UTF-8"?>
<Document xmlns="urn:iso:std:iso:20022:tech:xsd:pacs.008.001.08">
  <FIToFICstmrCdtTrf>
    <GrpHdr>
      <MsgId>REF20260816-001</MsgId>
      <CreDtTm>2026-08-16T10:15:00+00:00</CreDtTm>
      <NbOfTxs>1</NbOfTxs>
      <SttlmInf>
        <SttlmMtd>INDA</SttlmMtd>
      </SttlmInf>
    </GrpHdr>
    <CdtTrfTxInf>
      <PmtId>
        <InstrId>REF20260816-001</InstrId>
        <EndToEndId>E2E-INV-2026-0842</EndToEndId>
        <UETR>f47ac10b-58cc-4372-a567-0e02b2c3d479</UETR>
      </PmtId>
      <IntrBkSttlmAmt Ccy="EUR">250000.00</IntrBkSttlmAmt>
      <IntrBkSttlmDt>2026-08-16</IntrBkSttlmDt>
      <ChrgBr>SHAR</ChrgBr>
      <Dbtr>
        <Nm>ACME TRADING BV</Nm>
        <PstlAdr>
          <StrtNm>RUE DE LA LOI</StrtNm>
          <BldgNb>100</BldgNb>
          <TwnNm>BRUSSELS</TwnNm>
          <Ctry>BE</Ctry>
        </PstlAdr>
      </Dbtr>
      <DbtrAcct>
        <Id>
          <IBAN>BE68539007547034</IBAN>
        </Id>
      </DbtrAcct>
      <DbtrAgt>
        <FinInstnId>
          <BICFI>BANKBEBB</BICFI>
        </FinInstnId>
      </DbtrAgt>
      <CdtrAgt>
        <FinInstnId>
          <BICFI>BANKDEFF</BICFI>
        </FinInstnId>
      </CdtrAgt>
      <Cdtr>
        <Nm>JOHN DOE GMBH</Nm>
        <PstlAdr>
          <StrtNm>FRIEDRICHSTRASSE</StrtNm>
          <BldgNb>123</BldgNb>
          <TwnNm>BERLIN</TwnNm>
          <Ctry>DE</Ctry>
        </PstlAdr>
      </Cdtr>
      <CdtrAcct>
        <Id>
          <IBAN>DE89370400440532013000</IBAN>
        </Id>
      </CdtrAcct>
      <RmtInf>
        <Ustrd>/INV/2026-0842</Ustrd>
      </RmtInf>
    </CdtTrfTxInf>
  </FIToFICstmrCdtTrf>
</Document>)";

} // namespace samples
} // namespace swift
