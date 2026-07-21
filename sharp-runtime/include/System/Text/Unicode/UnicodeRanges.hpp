// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Text/Unicode/UnicodeRange.hpp"

namespace System::Text::Unicode {

    /**
     * @brief Provides static properties that return well-known UnicodeRange instances.
     *
     * C++ counterpart of .NET System.Text.Unicode.UnicodeRanges. All 160 Unicode BMP blocks
     * from the .NET reference source's generated block list are reproduced (mechanically
     * generated from that source, like TlsCipherSuite elsewhere in this runtime), plus the
     * hand-written `All`/`None` special cases.
     */
    class UnicodeRanges {
    public:
        UnicodeRanges() = delete;

        /** @return A UnicodeRange containing every code point in the BMP (U+0000–U+FFFF). */
        static UnicodeRange getAllProperty() { return UnicodeRange(0x0000, 0x10000); }
        /** @return A UnicodeRange with no contents. */
        static UnicodeRange getNoneProperty() { return UnicodeRange(0x0000, 0); }

        /** @return The BasicLatin block (U+0000–U+007F). */
        static UnicodeRange getBasicLatinProperty() { return UnicodeRange(0x0000, 128); }
        /** @return The Latin1Supplement block (U+0080–U+00FF). */
        static UnicodeRange getLatin1SupplementProperty() { return UnicodeRange(0x0080, 128); }
        /** @return The LatinExtendedA block (U+0100–U+017F). */
        static UnicodeRange getLatinExtendedAProperty() { return UnicodeRange(0x0100, 128); }
        /** @return The LatinExtendedB block (U+0180–U+024F). */
        static UnicodeRange getLatinExtendedBProperty() { return UnicodeRange(0x0180, 208); }
        /** @return The IpaExtensions block (U+0250–U+02AF). */
        static UnicodeRange getIpaExtensionsProperty() { return UnicodeRange(0x0250, 96); }
        /** @return The SpacingModifierLetters block (U+02B0–U+02FF). */
        static UnicodeRange getSpacingModifierLettersProperty() { return UnicodeRange(0x02B0, 80); }
        /** @return The CombiningDiacriticalMarks block (U+0300–U+036F). */
        static UnicodeRange getCombiningDiacriticalMarksProperty() { return UnicodeRange(0x0300, 112); }
        /** @return The GreekandCoptic block (U+0370–U+03FF). */
        static UnicodeRange getGreekandCopticProperty() { return UnicodeRange(0x0370, 144); }
        /** @return The Cyrillic block (U+0400–U+04FF). */
        static UnicodeRange getCyrillicProperty() { return UnicodeRange(0x0400, 256); }
        /** @return The CyrillicSupplement block (U+0500–U+052F). */
        static UnicodeRange getCyrillicSupplementProperty() { return UnicodeRange(0x0500, 48); }
        /** @return The Armenian block (U+0530–U+058F). */
        static UnicodeRange getArmenianProperty() { return UnicodeRange(0x0530, 96); }
        /** @return The Hebrew block (U+0590–U+05FF). */
        static UnicodeRange getHebrewProperty() { return UnicodeRange(0x0590, 112); }
        /** @return The Arabic block (U+0600–U+06FF). */
        static UnicodeRange getArabicProperty() { return UnicodeRange(0x0600, 256); }
        /** @return The Syriac block (U+0700–U+074F). */
        static UnicodeRange getSyriacProperty() { return UnicodeRange(0x0700, 80); }
        /** @return The ArabicSupplement block (U+0750–U+077F). */
        static UnicodeRange getArabicSupplementProperty() { return UnicodeRange(0x0750, 48); }
        /** @return The Thaana block (U+0780–U+07BF). */
        static UnicodeRange getThaanaProperty() { return UnicodeRange(0x0780, 64); }
        /** @return The NKo block (U+07C0–U+07FF). */
        static UnicodeRange getNKoProperty() { return UnicodeRange(0x07C0, 64); }
        /** @return The Samaritan block (U+0800–U+083F). */
        static UnicodeRange getSamaritanProperty() { return UnicodeRange(0x0800, 64); }
        /** @return The Mandaic block (U+0840–U+085F). */
        static UnicodeRange getMandaicProperty() { return UnicodeRange(0x0840, 32); }
        /** @return The SyriacSupplement block (U+0860–U+086F). */
        static UnicodeRange getSyriacSupplementProperty() { return UnicodeRange(0x0860, 16); }
        /** @return The ArabicExtendedB block (U+0870–U+089F). */
        static UnicodeRange getArabicExtendedBProperty() { return UnicodeRange(0x0870, 48); }
        /** @return The ArabicExtendedA block (U+08A0–U+08FF). */
        static UnicodeRange getArabicExtendedAProperty() { return UnicodeRange(0x08A0, 96); }
        /** @return The Devanagari block (U+0900–U+097F). */
        static UnicodeRange getDevanagariProperty() { return UnicodeRange(0x0900, 128); }
        /** @return The Bengali block (U+0980–U+09FF). */
        static UnicodeRange getBengaliProperty() { return UnicodeRange(0x0980, 128); }
        /** @return The Gurmukhi block (U+0A00–U+0A7F). */
        static UnicodeRange getGurmukhiProperty() { return UnicodeRange(0x0A00, 128); }
        /** @return The Gujarati block (U+0A80–U+0AFF). */
        static UnicodeRange getGujaratiProperty() { return UnicodeRange(0x0A80, 128); }
        /** @return The Oriya block (U+0B00–U+0B7F). */
        static UnicodeRange getOriyaProperty() { return UnicodeRange(0x0B00, 128); }
        /** @return The Tamil block (U+0B80–U+0BFF). */
        static UnicodeRange getTamilProperty() { return UnicodeRange(0x0B80, 128); }
        /** @return The Telugu block (U+0C00–U+0C7F). */
        static UnicodeRange getTeluguProperty() { return UnicodeRange(0x0C00, 128); }
        /** @return The Kannada block (U+0C80–U+0CFF). */
        static UnicodeRange getKannadaProperty() { return UnicodeRange(0x0C80, 128); }
        /** @return The Malayalam block (U+0D00–U+0D7F). */
        static UnicodeRange getMalayalamProperty() { return UnicodeRange(0x0D00, 128); }
        /** @return The Sinhala block (U+0D80–U+0DFF). */
        static UnicodeRange getSinhalaProperty() { return UnicodeRange(0x0D80, 128); }
        /** @return The Thai block (U+0E00–U+0E7F). */
        static UnicodeRange getThaiProperty() { return UnicodeRange(0x0E00, 128); }
        /** @return The Lao block (U+0E80–U+0EFF). */
        static UnicodeRange getLaoProperty() { return UnicodeRange(0x0E80, 128); }
        /** @return The Tibetan block (U+0F00–U+0FFF). */
        static UnicodeRange getTibetanProperty() { return UnicodeRange(0x0F00, 256); }
        /** @return The Myanmar block (U+1000–U+109F). */
        static UnicodeRange getMyanmarProperty() { return UnicodeRange(0x1000, 160); }
        /** @return The Georgian block (U+10A0–U+10FF). */
        static UnicodeRange getGeorgianProperty() { return UnicodeRange(0x10A0, 96); }
        /** @return The HangulJamo block (U+1100–U+11FF). */
        static UnicodeRange getHangulJamoProperty() { return UnicodeRange(0x1100, 256); }
        /** @return The Ethiopic block (U+1200–U+137F). */
        static UnicodeRange getEthiopicProperty() { return UnicodeRange(0x1200, 384); }
        /** @return The EthiopicSupplement block (U+1380–U+139F). */
        static UnicodeRange getEthiopicSupplementProperty() { return UnicodeRange(0x1380, 32); }
        /** @return The Cherokee block (U+13A0–U+13FF). */
        static UnicodeRange getCherokeeProperty() { return UnicodeRange(0x13A0, 96); }
        /** @return The UnifiedCanadianAboriginalSyllabics block (U+1400–U+167F). */
        static UnicodeRange getUnifiedCanadianAboriginalSyllabicsProperty() { return UnicodeRange(0x1400, 640); }
        /** @return The Ogham block (U+1680–U+169F). */
        static UnicodeRange getOghamProperty() { return UnicodeRange(0x1680, 32); }
        /** @return The Runic block (U+16A0–U+16FF). */
        static UnicodeRange getRunicProperty() { return UnicodeRange(0x16A0, 96); }
        /** @return The Tagalog block (U+1700–U+171F). */
        static UnicodeRange getTagalogProperty() { return UnicodeRange(0x1700, 32); }
        /** @return The Hanunoo block (U+1720–U+173F). */
        static UnicodeRange getHanunooProperty() { return UnicodeRange(0x1720, 32); }
        /** @return The Buhid block (U+1740–U+175F). */
        static UnicodeRange getBuhidProperty() { return UnicodeRange(0x1740, 32); }
        /** @return The Tagbanwa block (U+1760–U+177F). */
        static UnicodeRange getTagbanwaProperty() { return UnicodeRange(0x1760, 32); }
        /** @return The Khmer block (U+1780–U+17FF). */
        static UnicodeRange getKhmerProperty() { return UnicodeRange(0x1780, 128); }
        /** @return The Mongolian block (U+1800–U+18AF). */
        static UnicodeRange getMongolianProperty() { return UnicodeRange(0x1800, 176); }
        /** @return The UnifiedCanadianAboriginalSyllabicsExtended block (U+18B0–U+18FF). */
        static UnicodeRange getUnifiedCanadianAboriginalSyllabicsExtendedProperty() { return UnicodeRange(0x18B0, 80); }
        /** @return The Limbu block (U+1900–U+194F). */
        static UnicodeRange getLimbuProperty() { return UnicodeRange(0x1900, 80); }
        /** @return The TaiLe block (U+1950–U+197F). */
        static UnicodeRange getTaiLeProperty() { return UnicodeRange(0x1950, 48); }
        /** @return The NewTaiLue block (U+1980–U+19DF). */
        static UnicodeRange getNewTaiLueProperty() { return UnicodeRange(0x1980, 96); }
        /** @return The KhmerSymbols block (U+19E0–U+19FF). */
        static UnicodeRange getKhmerSymbolsProperty() { return UnicodeRange(0x19E0, 32); }
        /** @return The Buginese block (U+1A00–U+1A1F). */
        static UnicodeRange getBugineseProperty() { return UnicodeRange(0x1A00, 32); }
        /** @return The TaiTham block (U+1A20–U+1AAF). */
        static UnicodeRange getTaiThamProperty() { return UnicodeRange(0x1A20, 144); }
        /** @return The CombiningDiacriticalMarksExtended block (U+1AB0–U+1AFF). */
        static UnicodeRange getCombiningDiacriticalMarksExtendedProperty() { return UnicodeRange(0x1AB0, 80); }
        /** @return The Balinese block (U+1B00–U+1B7F). */
        static UnicodeRange getBalineseProperty() { return UnicodeRange(0x1B00, 128); }
        /** @return The Sundanese block (U+1B80–U+1BBF). */
        static UnicodeRange getSundaneseProperty() { return UnicodeRange(0x1B80, 64); }
        /** @return The Batak block (U+1BC0–U+1BFF). */
        static UnicodeRange getBatakProperty() { return UnicodeRange(0x1BC0, 64); }
        /** @return The Lepcha block (U+1C00–U+1C4F). */
        static UnicodeRange getLepchaProperty() { return UnicodeRange(0x1C00, 80); }
        /** @return The OlChiki block (U+1C50–U+1C7F). */
        static UnicodeRange getOlChikiProperty() { return UnicodeRange(0x1C50, 48); }
        /** @return The CyrillicExtendedC block (U+1C80–U+1C8F). */
        static UnicodeRange getCyrillicExtendedCProperty() { return UnicodeRange(0x1C80, 16); }
        /** @return The GeorgianExtended block (U+1C90–U+1CBF). */
        static UnicodeRange getGeorgianExtendedProperty() { return UnicodeRange(0x1C90, 48); }
        /** @return The SundaneseSupplement block (U+1CC0–U+1CCF). */
        static UnicodeRange getSundaneseSupplementProperty() { return UnicodeRange(0x1CC0, 16); }
        /** @return The VedicExtensions block (U+1CD0–U+1CFF). */
        static UnicodeRange getVedicExtensionsProperty() { return UnicodeRange(0x1CD0, 48); }
        /** @return The PhoneticExtensions block (U+1D00–U+1D7F). */
        static UnicodeRange getPhoneticExtensionsProperty() { return UnicodeRange(0x1D00, 128); }
        /** @return The PhoneticExtensionsSupplement block (U+1D80–U+1DBF). */
        static UnicodeRange getPhoneticExtensionsSupplementProperty() { return UnicodeRange(0x1D80, 64); }
        /** @return The CombiningDiacriticalMarksSupplement block (U+1DC0–U+1DFF). */
        static UnicodeRange getCombiningDiacriticalMarksSupplementProperty() { return UnicodeRange(0x1DC0, 64); }
        /** @return The LatinExtendedAdditional block (U+1E00–U+1EFF). */
        static UnicodeRange getLatinExtendedAdditionalProperty() { return UnicodeRange(0x1E00, 256); }
        /** @return The GreekExtended block (U+1F00–U+1FFF). */
        static UnicodeRange getGreekExtendedProperty() { return UnicodeRange(0x1F00, 256); }
        /** @return The GeneralPunctuation block (U+2000–U+206F). */
        static UnicodeRange getGeneralPunctuationProperty() { return UnicodeRange(0x2000, 112); }
        /** @return The SuperscriptsandSubscripts block (U+2070–U+209F). */
        static UnicodeRange getSuperscriptsandSubscriptsProperty() { return UnicodeRange(0x2070, 48); }
        /** @return The CurrencySymbols block (U+20A0–U+20CF). */
        static UnicodeRange getCurrencySymbolsProperty() { return UnicodeRange(0x20A0, 48); }
        /** @return The CombiningDiacriticalMarksforSymbols block (U+20D0–U+20FF). */
        static UnicodeRange getCombiningDiacriticalMarksforSymbolsProperty() { return UnicodeRange(0x20D0, 48); }
        /** @return The LetterlikeSymbols block (U+2100–U+214F). */
        static UnicodeRange getLetterlikeSymbolsProperty() { return UnicodeRange(0x2100, 80); }
        /** @return The NumberForms block (U+2150–U+218F). */
        static UnicodeRange getNumberFormsProperty() { return UnicodeRange(0x2150, 64); }
        /** @return The Arrows block (U+2190–U+21FF). */
        static UnicodeRange getArrowsProperty() { return UnicodeRange(0x2190, 112); }
        /** @return The MathematicalOperators block (U+2200–U+22FF). */
        static UnicodeRange getMathematicalOperatorsProperty() { return UnicodeRange(0x2200, 256); }
        /** @return The MiscellaneousTechnical block (U+2300–U+23FF). */
        static UnicodeRange getMiscellaneousTechnicalProperty() { return UnicodeRange(0x2300, 256); }
        /** @return The ControlPictures block (U+2400–U+243F). */
        static UnicodeRange getControlPicturesProperty() { return UnicodeRange(0x2400, 64); }
        /** @return The OpticalCharacterRecognition block (U+2440–U+245F). */
        static UnicodeRange getOpticalCharacterRecognitionProperty() { return UnicodeRange(0x2440, 32); }
        /** @return The EnclosedAlphanumerics block (U+2460–U+24FF). */
        static UnicodeRange getEnclosedAlphanumericsProperty() { return UnicodeRange(0x2460, 160); }
        /** @return The BoxDrawing block (U+2500–U+257F). */
        static UnicodeRange getBoxDrawingProperty() { return UnicodeRange(0x2500, 128); }
        /** @return The BlockElements block (U+2580–U+259F). */
        static UnicodeRange getBlockElementsProperty() { return UnicodeRange(0x2580, 32); }
        /** @return The GeometricShapes block (U+25A0–U+25FF). */
        static UnicodeRange getGeometricShapesProperty() { return UnicodeRange(0x25A0, 96); }
        /** @return The MiscellaneousSymbols block (U+2600–U+26FF). */
        static UnicodeRange getMiscellaneousSymbolsProperty() { return UnicodeRange(0x2600, 256); }
        /** @return The Dingbats block (U+2700–U+27BF). */
        static UnicodeRange getDingbatsProperty() { return UnicodeRange(0x2700, 192); }
        /** @return The MiscellaneousMathematicalSymbolsA block (U+27C0–U+27EF). */
        static UnicodeRange getMiscellaneousMathematicalSymbolsAProperty() { return UnicodeRange(0x27C0, 48); }
        /** @return The SupplementalArrowsA block (U+27F0–U+27FF). */
        static UnicodeRange getSupplementalArrowsAProperty() { return UnicodeRange(0x27F0, 16); }
        /** @return The BraillePatterns block (U+2800–U+28FF). */
        static UnicodeRange getBraillePatternsProperty() { return UnicodeRange(0x2800, 256); }
        /** @return The SupplementalArrowsB block (U+2900–U+297F). */
        static UnicodeRange getSupplementalArrowsBProperty() { return UnicodeRange(0x2900, 128); }
        /** @return The MiscellaneousMathematicalSymbolsB block (U+2980–U+29FF). */
        static UnicodeRange getMiscellaneousMathematicalSymbolsBProperty() { return UnicodeRange(0x2980, 128); }
        /** @return The SupplementalMathematicalOperators block (U+2A00–U+2AFF). */
        static UnicodeRange getSupplementalMathematicalOperatorsProperty() { return UnicodeRange(0x2A00, 256); }
        /** @return The MiscellaneousSymbolsandArrows block (U+2B00–U+2BFF). */
        static UnicodeRange getMiscellaneousSymbolsandArrowsProperty() { return UnicodeRange(0x2B00, 256); }
        /** @return The Glagolitic block (U+2C00–U+2C5F). */
        static UnicodeRange getGlagoliticProperty() { return UnicodeRange(0x2C00, 96); }
        /** @return The LatinExtendedC block (U+2C60–U+2C7F). */
        static UnicodeRange getLatinExtendedCProperty() { return UnicodeRange(0x2C60, 32); }
        /** @return The Coptic block (U+2C80–U+2CFF). */
        static UnicodeRange getCopticProperty() { return UnicodeRange(0x2C80, 128); }
        /** @return The GeorgianSupplement block (U+2D00–U+2D2F). */
        static UnicodeRange getGeorgianSupplementProperty() { return UnicodeRange(0x2D00, 48); }
        /** @return The Tifinagh block (U+2D30–U+2D7F). */
        static UnicodeRange getTifinaghProperty() { return UnicodeRange(0x2D30, 80); }
        /** @return The EthiopicExtended block (U+2D80–U+2DDF). */
        static UnicodeRange getEthiopicExtendedProperty() { return UnicodeRange(0x2D80, 96); }
        /** @return The CyrillicExtendedA block (U+2DE0–U+2DFF). */
        static UnicodeRange getCyrillicExtendedAProperty() { return UnicodeRange(0x2DE0, 32); }
        /** @return The SupplementalPunctuation block (U+2E00–U+2E7F). */
        static UnicodeRange getSupplementalPunctuationProperty() { return UnicodeRange(0x2E00, 128); }
        /** @return The CjkRadicalsSupplement block (U+2E80–U+2EFF). */
        static UnicodeRange getCjkRadicalsSupplementProperty() { return UnicodeRange(0x2E80, 128); }
        /** @return The KangxiRadicals block (U+2F00–U+2FDF). */
        static UnicodeRange getKangxiRadicalsProperty() { return UnicodeRange(0x2F00, 224); }
        /** @return The IdeographicDescriptionCharacters block (U+2FF0–U+2FFF). */
        static UnicodeRange getIdeographicDescriptionCharactersProperty() { return UnicodeRange(0x2FF0, 16); }
        /** @return The CjkSymbolsandPunctuation block (U+3000–U+303F). */
        static UnicodeRange getCjkSymbolsandPunctuationProperty() { return UnicodeRange(0x3000, 64); }
        /** @return The Hiragana block (U+3040–U+309F). */
        static UnicodeRange getHiraganaProperty() { return UnicodeRange(0x3040, 96); }
        /** @return The Katakana block (U+30A0–U+30FF). */
        static UnicodeRange getKatakanaProperty() { return UnicodeRange(0x30A0, 96); }
        /** @return The Bopomofo block (U+3100–U+312F). */
        static UnicodeRange getBopomofoProperty() { return UnicodeRange(0x3100, 48); }
        /** @return The HangulCompatibilityJamo block (U+3130–U+318F). */
        static UnicodeRange getHangulCompatibilityJamoProperty() { return UnicodeRange(0x3130, 96); }
        /** @return The Kanbun block (U+3190–U+319F). */
        static UnicodeRange getKanbunProperty() { return UnicodeRange(0x3190, 16); }
        /** @return The BopomofoExtended block (U+31A0–U+31BF). */
        static UnicodeRange getBopomofoExtendedProperty() { return UnicodeRange(0x31A0, 32); }
        /** @return The CjkStrokes block (U+31C0–U+31EF). */
        static UnicodeRange getCjkStrokesProperty() { return UnicodeRange(0x31C0, 48); }
        /** @return The KatakanaPhoneticExtensions block (U+31F0–U+31FF). */
        static UnicodeRange getKatakanaPhoneticExtensionsProperty() { return UnicodeRange(0x31F0, 16); }
        /** @return The EnclosedCjkLettersandMonths block (U+3200–U+32FF). */
        static UnicodeRange getEnclosedCjkLettersandMonthsProperty() { return UnicodeRange(0x3200, 256); }
        /** @return The CjkCompatibility block (U+3300–U+33FF). */
        static UnicodeRange getCjkCompatibilityProperty() { return UnicodeRange(0x3300, 256); }
        /** @return The CjkUnifiedIdeographsExtensionA block (U+3400–U+4DBF). */
        static UnicodeRange getCjkUnifiedIdeographsExtensionAProperty() { return UnicodeRange(0x3400, 6592); }
        /** @return The YijingHexagramSymbols block (U+4DC0–U+4DFF). */
        static UnicodeRange getYijingHexagramSymbolsProperty() { return UnicodeRange(0x4DC0, 64); }
        /** @return The CjkUnifiedIdeographs block (U+4E00–U+9FFF). */
        static UnicodeRange getCjkUnifiedIdeographsProperty() { return UnicodeRange(0x4E00, 20992); }
        /** @return The YiSyllables block (U+A000–U+A48F). */
        static UnicodeRange getYiSyllablesProperty() { return UnicodeRange(0xA000, 1168); }
        /** @return The YiRadicals block (U+A490–U+A4CF). */
        static UnicodeRange getYiRadicalsProperty() { return UnicodeRange(0xA490, 64); }
        /** @return The Lisu block (U+A4D0–U+A4FF). */
        static UnicodeRange getLisuProperty() { return UnicodeRange(0xA4D0, 48); }
        /** @return The Vai block (U+A500–U+A63F). */
        static UnicodeRange getVaiProperty() { return UnicodeRange(0xA500, 320); }
        /** @return The CyrillicExtendedB block (U+A640–U+A69F). */
        static UnicodeRange getCyrillicExtendedBProperty() { return UnicodeRange(0xA640, 96); }
        /** @return The Bamum block (U+A6A0–U+A6FF). */
        static UnicodeRange getBamumProperty() { return UnicodeRange(0xA6A0, 96); }
        /** @return The ModifierToneLetters block (U+A700–U+A71F). */
        static UnicodeRange getModifierToneLettersProperty() { return UnicodeRange(0xA700, 32); }
        /** @return The LatinExtendedD block (U+A720–U+A7FF). */
        static UnicodeRange getLatinExtendedDProperty() { return UnicodeRange(0xA720, 224); }
        /** @return The SylotiNagri block (U+A800–U+A82F). */
        static UnicodeRange getSylotiNagriProperty() { return UnicodeRange(0xA800, 48); }
        /** @return The CommonIndicNumberForms block (U+A830–U+A83F). */
        static UnicodeRange getCommonIndicNumberFormsProperty() { return UnicodeRange(0xA830, 16); }
        /** @return The Phagspa block (U+A840–U+A87F). */
        static UnicodeRange getPhagspaProperty() { return UnicodeRange(0xA840, 64); }
        /** @return The Saurashtra block (U+A880–U+A8DF). */
        static UnicodeRange getSaurashtraProperty() { return UnicodeRange(0xA880, 96); }
        /** @return The DevanagariExtended block (U+A8E0–U+A8FF). */
        static UnicodeRange getDevanagariExtendedProperty() { return UnicodeRange(0xA8E0, 32); }
        /** @return The KayahLi block (U+A900–U+A92F). */
        static UnicodeRange getKayahLiProperty() { return UnicodeRange(0xA900, 48); }
        /** @return The Rejang block (U+A930–U+A95F). */
        static UnicodeRange getRejangProperty() { return UnicodeRange(0xA930, 48); }
        /** @return The HangulJamoExtendedA block (U+A960–U+A97F). */
        static UnicodeRange getHangulJamoExtendedAProperty() { return UnicodeRange(0xA960, 32); }
        /** @return The Javanese block (U+A980–U+A9DF). */
        static UnicodeRange getJavaneseProperty() { return UnicodeRange(0xA980, 96); }
        /** @return The MyanmarExtendedB block (U+A9E0–U+A9FF). */
        static UnicodeRange getMyanmarExtendedBProperty() { return UnicodeRange(0xA9E0, 32); }
        /** @return The Cham block (U+AA00–U+AA5F). */
        static UnicodeRange getChamProperty() { return UnicodeRange(0xAA00, 96); }
        /** @return The MyanmarExtendedA block (U+AA60–U+AA7F). */
        static UnicodeRange getMyanmarExtendedAProperty() { return UnicodeRange(0xAA60, 32); }
        /** @return The TaiViet block (U+AA80–U+AADF). */
        static UnicodeRange getTaiVietProperty() { return UnicodeRange(0xAA80, 96); }
        /** @return The MeeteiMayekExtensions block (U+AAE0–U+AAFF). */
        static UnicodeRange getMeeteiMayekExtensionsProperty() { return UnicodeRange(0xAAE0, 32); }
        /** @return The EthiopicExtendedA block (U+AB00–U+AB2F). */
        static UnicodeRange getEthiopicExtendedAProperty() { return UnicodeRange(0xAB00, 48); }
        /** @return The LatinExtendedE block (U+AB30–U+AB6F). */
        static UnicodeRange getLatinExtendedEProperty() { return UnicodeRange(0xAB30, 64); }
        /** @return The CherokeeSupplement block (U+AB70–U+ABBF). */
        static UnicodeRange getCherokeeSupplementProperty() { return UnicodeRange(0xAB70, 80); }
        /** @return The MeeteiMayek block (U+ABC0–U+ABFF). */
        static UnicodeRange getMeeteiMayekProperty() { return UnicodeRange(0xABC0, 64); }
        /** @return The HangulSyllables block (U+AC00–U+D7AF). */
        static UnicodeRange getHangulSyllablesProperty() { return UnicodeRange(0xAC00, 11184); }
        /** @return The HangulJamoExtendedB block (U+D7B0–U+D7FF). */
        static UnicodeRange getHangulJamoExtendedBProperty() { return UnicodeRange(0xD7B0, 80); }
        /** @return The CjkCompatibilityIdeographs block (U+F900–U+FAFF). */
        static UnicodeRange getCjkCompatibilityIdeographsProperty() { return UnicodeRange(0xF900, 512); }
        /** @return The AlphabeticPresentationForms block (U+FB00–U+FB4F). */
        static UnicodeRange getAlphabeticPresentationFormsProperty() { return UnicodeRange(0xFB00, 80); }
        /** @return The ArabicPresentationFormsA block (U+FB50–U+FDFF). */
        static UnicodeRange getArabicPresentationFormsAProperty() { return UnicodeRange(0xFB50, 688); }
        /** @return The VariationSelectors block (U+FE00–U+FE0F). */
        static UnicodeRange getVariationSelectorsProperty() { return UnicodeRange(0xFE00, 16); }
        /** @return The VerticalForms block (U+FE10–U+FE1F). */
        static UnicodeRange getVerticalFormsProperty() { return UnicodeRange(0xFE10, 16); }
        /** @return The CombiningHalfMarks block (U+FE20–U+FE2F). */
        static UnicodeRange getCombiningHalfMarksProperty() { return UnicodeRange(0xFE20, 16); }
        /** @return The CjkCompatibilityForms block (U+FE30–U+FE4F). */
        static UnicodeRange getCjkCompatibilityFormsProperty() { return UnicodeRange(0xFE30, 32); }
        /** @return The SmallFormVariants block (U+FE50–U+FE6F). */
        static UnicodeRange getSmallFormVariantsProperty() { return UnicodeRange(0xFE50, 32); }
        /** @return The ArabicPresentationFormsB block (U+FE70–U+FEFF). */
        static UnicodeRange getArabicPresentationFormsBProperty() { return UnicodeRange(0xFE70, 144); }
        /** @return The HalfwidthandFullwidthForms block (U+FF00–U+FFEF). */
        static UnicodeRange getHalfwidthandFullwidthFormsProperty() { return UnicodeRange(0xFF00, 240); }
        /** @return The Specials block (U+FFF0–U+FFFF). */
        static UnicodeRange getSpecialsProperty() { return UnicodeRange(0xFFF0, 16); }
    };

} // namespace System::Text::Unicode
