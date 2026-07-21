// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Task 41: IntPtr/UIntPtr, DaylightTime, DigitShapes, SortVersion,
// DecoderFallback, EncoderFallback, Win32Exception, DataAnnotations,
// JsonSerializerOptions, Json enums, JsonValueKind, JsonSerializationAttributes,
// JsonSerializer, DictionaryEntry, PropertyDescriptorCollection, Prop macros.
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "System/IntPtr.hpp"
#include "System/UIntPtr.hpp"
#include "System/Globalization/DaylightTime.hpp"
#include "System/Globalization/DigitShapes.hpp"
#include "System/Globalization/SortVersion.hpp"
#include "System/Text/DecoderFallback.hpp"
#include "System/Text/EncoderFallback.hpp"
#include "System/ComponentModel/Win32Exception.hpp"
#include "System/ComponentModel/DataAnnotations/DataAnnotationAttributes.hpp"
#include "System/Text/Json/JsonSerializerOptions.hpp"
#include "System/Text/Json/JsonValueKind.hpp"
#include "System/Text/Json/Serialization/JsonSerializationAttributes.hpp"
#include "System/Text/Json/JsonSerializer.hpp"
#include "System/Collections/DictionaryEntry.hpp"
#include "System/ComponentModel/PropertyDescriptorCollection.hpp"
#include "SharpRuntime/Prop.hpp"

// ===========================================================================
// IntPtr
// ===========================================================================

TEST(IntPtrTests, DefaultCtor_IsZero) {
    System::IntPtr p;
    EXPECT_EQ(p.value, intptr_t(0));
    EXPECT_TRUE(p.IsZero());
}

TEST(IntPtrTests, Zero_IsZero) {
    EXPECT_TRUE(System::IntPtr::Zero.IsZero());
}

TEST(IntPtrTests, ValueCtor_Stored) {
    System::IntPtr p(42);
    EXPECT_EQ(p.value, intptr_t(42));
    EXPECT_FALSE(p.IsZero());
}

TEST(IntPtrTests, Equality) {
    System::IntPtr a(10), b(10), c(20);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(IntPtrTests, VoidPtrCtor) {
    int x = 0;
    System::IntPtr p(&x);
    EXPECT_FALSE(p.IsZero());
    EXPECT_EQ(static_cast<void*>(p), &x);
}

TEST(IntPtrTests, Size_MatchesSizeofIntptr) {
    EXPECT_EQ(System::IntPtr::Size, static_cast<int>(sizeof(intptr_t)));
}

TEST(IntPtrTests, MaxValue_MinValue) {
    EXPECT_EQ(System::IntPtr::MaxValue.value, std::numeric_limits<intptr_t>::max());
    EXPECT_EQ(System::IntPtr::MinValue.value, std::numeric_limits<intptr_t>::min());
}

TEST(IntPtrTests, ToInt32_FitsInRange) {
    System::IntPtr p(42);
    EXPECT_EQ(p.ToInt32(), 42);
}

TEST(IntPtrTests, ToInt32_Overflow_Throws) {
    if (sizeof(intptr_t) > sizeof(int32_t)) {
        System::IntPtr p(static_cast<intptr_t>(std::numeric_limits<int64_t>::max()));
        EXPECT_THROW(p.ToInt32(), System::OverflowException);
    }
}

TEST(IntPtrTests, ToInt64_ReturnsValue) {
    System::IntPtr p(12345);
    EXPECT_EQ(p.ToInt64(), int64_t(12345));
}

TEST(IntPtrTests, ToPointer_RoundTrips) {
    int x = 7;
    System::IntPtr p(&x);
    EXPECT_EQ(p.ToPointer(), static_cast<void*>(&x));
}

TEST(IntPtrTests, CompareTo_Ordering) {
    EXPECT_LT(System::IntPtr(1).CompareTo(System::IntPtr(2)), 0);
    EXPECT_EQ(System::IntPtr(2).CompareTo(System::IntPtr(2)), 0);
    EXPECT_GT(System::IntPtr(3).CompareTo(System::IntPtr(2)), 0);
}

TEST(IntPtrTests, Equals_Method) {
    EXPECT_TRUE(System::IntPtr(5).Equals(System::IntPtr(5)));
    EXPECT_FALSE(System::IntPtr(5).Equals(System::IntPtr(6)));
}

TEST(IntPtrTests, GetHashCode_SameValueSameHash) {
    EXPECT_EQ(System::IntPtr(9).GetHashCode(), System::IntPtr(9).GetHashCode());
}

TEST(IntPtrTests, ToString_Decimal) {
    EXPECT_EQ(System::IntPtr(123).ToString(), "123");
    EXPECT_EQ(System::IntPtr(-5).ToString(), "-5");
}

TEST(IntPtrTests, Add_Subtract_Static) {
    System::IntPtr p(100);
    EXPECT_EQ(System::IntPtr::Add(p, 5).value, intptr_t(105));
    EXPECT_EQ(System::IntPtr::Subtract(p, 5).value, intptr_t(95));
}

TEST(IntPtrTests, Add_Subtract_Operators) {
    System::IntPtr p(100);
    EXPECT_EQ((p + 5).value, intptr_t(105));
    EXPECT_EQ((p - 5).value, intptr_t(95));
}

// ===========================================================================
// UIntPtr
// ===========================================================================

TEST(UIntPtrTests, DefaultCtor_IsZero) {
    System::UIntPtr p;
    EXPECT_EQ(p.value, uintptr_t(0));
    EXPECT_TRUE(p.IsZero());
}

TEST(UIntPtrTests, Zero_IsZero) {
    EXPECT_TRUE(System::UIntPtr::Zero.IsZero());
}

TEST(UIntPtrTests, ValueCtor_Stored) {
    System::UIntPtr p(99u);
    EXPECT_EQ(p.value, uintptr_t(99));
    EXPECT_FALSE(p.IsZero());
}

TEST(UIntPtrTests, Equality) {
    System::UIntPtr a(5u), b(5u), c(6u);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ===========================================================================
// DaylightTime
// ===========================================================================

using System::DateTime;
using System::TimeSpan;
using System::Globalization::DaylightTime;

TEST(DaylightTimeTests, Ctor_StoresValues) {
    DateTime start, end;
    TimeSpan delta = TimeSpan::FromHours(1);
    DaylightTime dt(start, end, delta);
    EXPECT_EQ(dt.getStartProperty(), start);
    EXPECT_EQ(dt.getEndProperty(), end);
    EXPECT_NEAR(dt.getDeltaProperty().getTotalHoursProperty(), 1.0, 1e-9);
}

// ===========================================================================
// DigitShapes
// ===========================================================================

using System::Globalization::DigitShapes;

TEST(DigitShapesTests, Values) {
    EXPECT_EQ(static_cast<int>(DigitShapes::Context),        0);
    EXPECT_EQ(static_cast<int>(DigitShapes::None),           1);
    EXPECT_EQ(static_cast<int>(DigitShapes::NativeNational), 2);
}

// ===========================================================================
// SortVersion
// ===========================================================================

using System::Globalization::SortVersion;

TEST(SortVersionTests, FullVersion_Stored) {
    SortVersion sv(1234);
    EXPECT_EQ(sv.getFullVersionProperty(), 1234);
}

TEST(SortVersionTests, Equality_SameFullVersion) {
    SortVersion a(42), b(42);
    EXPECT_EQ(a, b);
}

TEST(SortVersionTests, Inequality_DifferentVersion) {
    SortVersion a(1), b(2);
    EXPECT_NE(a, b);
}

TEST(SortVersionTests, SortId_DefaultAllZero) {
    SortVersion sv(0);
    for (auto byte : sv.getSortIdProperty()) {
        EXPECT_EQ(byte, 0u);
    }
}

// ===========================================================================
// DecoderFallback
// ===========================================================================

using System::Text::DecoderFallback;
using System::Text::DecoderReplacementFallback;
using System::Text::DecoderExceptionFallback;
using System::Text::DecoderFallbackException;

TEST(DecoderFallbackTests, ReplacementFallback_ReturnsQuestionMark) {
    auto fb = DecoderFallback::ReplacementFallback();
    EXPECT_EQ(fb->GetFallbackString(nullptr, 0), "?");
}

TEST(DecoderFallbackTests, ReplacementFallback_MaxCharCount_One) {
    auto fb = DecoderFallback::ReplacementFallback();
    EXPECT_EQ(fb->getMaxCharCountProperty(), 1);
}

TEST(DecoderFallbackTests, ExceptionFallback_GetFallbackString_Throws) {
    auto fb = DecoderFallback::ExceptionFallback();
    EXPECT_THROW(fb->GetFallbackString(nullptr, 0), DecoderFallbackException);
}

TEST(DecoderFallbackTests, ExceptionFallback_MaxCharCount_Zero) {
    auto fb = DecoderFallback::ExceptionFallback();
    EXPECT_EQ(fb->getMaxCharCountProperty(), 0);
}

TEST(DecoderFallbackTests, ReplacementFallback_Singleton) {
    auto a = DecoderFallback::ReplacementFallback();
    auto b = DecoderFallback::ReplacementFallback();
    EXPECT_EQ(a.get(), b.get());
}

TEST(DecoderReplacementFallbackTests, CustomReplacement) {
    DecoderReplacementFallback fb("__");
    EXPECT_EQ(fb.GetFallbackString(nullptr, 0), "__");
    EXPECT_EQ(fb.getMaxCharCountProperty(), 2);
    EXPECT_EQ(fb.getDefaultStringProperty(), "__");
}

// ===========================================================================
// EncoderFallback
// ===========================================================================

using System::Text::EncoderFallback;
using System::Text::EncoderReplacementFallback;
using System::Text::EncoderExceptionFallback;
using System::Text::EncoderFallbackException;

TEST(EncoderFallbackTests, ReplacementFallback_ReturnsQuestionMarkBytes) {
    auto fb = EncoderFallback::ReplacementFallback();
    auto bytes = fb->GetFallbackBytes('?');
    ASSERT_EQ(static_cast<int>(bytes.size()), 1);
    EXPECT_EQ(bytes[0], uint8_t('?'));
}

TEST(EncoderFallbackTests, ReplacementFallback_MaxByteCount_One) {
    auto fb = EncoderFallback::ReplacementFallback();
    EXPECT_EQ(fb->getMaxByteCountProperty(), 1);
}

TEST(EncoderFallbackTests, ExceptionFallback_GetFallbackBytes_Throws) {
    auto fb = EncoderFallback::ExceptionFallback();
    EXPECT_THROW(fb->GetFallbackBytes('x'), EncoderFallbackException);
}

TEST(EncoderFallbackTests, ExceptionFallback_MaxByteCount_Zero) {
    auto fb = EncoderFallback::ExceptionFallback();
    EXPECT_EQ(fb->getMaxByteCountProperty(), 0);
}

TEST(EncoderReplacementFallbackTests, CustomReplacement) {
    EncoderReplacementFallback fb("XX");
    auto bytes = fb.GetFallbackBytes('a');
    ASSERT_EQ(static_cast<int>(bytes.size()), 2);
    EXPECT_EQ(fb.getMaxByteCountProperty(), 2);
    EXPECT_EQ(fb.getDefaultStringProperty(), "XX");
}

// ===========================================================================
// Win32Exception
// ===========================================================================

using System::ComponentModel::Win32Exception;

TEST(Win32ExceptionTests, ErrorCode_Stored) {
    Win32Exception ex(5);
    EXPECT_EQ(ex.getNativeErrorCodeProperty(), 5);
}

TEST(Win32ExceptionTests, DefaultMsg_ContainsErrorCode) {
    Win32Exception ex(42);
    EXPECT_NE(std::string(ex.what()).find("42"), std::string::npos);
}

TEST(Win32ExceptionTests, CustomMsg_Used) {
    Win32Exception ex(0, "access denied");
    EXPECT_NE(std::string(ex.what()).find("access denied"), std::string::npos);
}

TEST(Win32ExceptionTests, IsA_Exception) {
    EXPECT_THROW(throw Win32Exception(1), System::Exception);
}

// ===========================================================================
// DataAnnotations
// ===========================================================================

using System::ComponentModel::DataAnnotations::ValidationAttribute;
using System::ComponentModel::DataAnnotations::RequiredAttribute;
using System::ComponentModel::DataAnnotations::RangeAttribute;
using System::ComponentModel::DataAnnotations::StringLengthAttribute;
using System::ComponentModel::DataAnnotations::MaxLengthAttribute;
using System::ComponentModel::DataAnnotations::MinLengthAttribute;
using System::ComponentModel::DataAnnotations::RegularExpressionAttribute;

TEST(ValidationAttributeTests, DefaultCtor_EmptyMessage) {
    ValidationAttribute a;
    EXPECT_TRUE(a.getErrorMessageProperty().empty());
}

TEST(ValidationAttributeTests, MessageCtor_StoresMessage) {
    ValidationAttribute a("required");
    EXPECT_EQ(a.getErrorMessageProperty(), "required");
}

TEST(RequiredAttributeTests, AllowEmptyStrings_DefaultFalse) {
    RequiredAttribute a;
    EXPECT_FALSE(a.AllowEmptyStrings);
}

TEST(RangeAttributeTests, MinMax_Stored_Double) {
    RangeAttribute a(1.0, 100.0);
    EXPECT_DOUBLE_EQ(a.getMinimumProperty(), 1.0);
    EXPECT_DOUBLE_EQ(a.getMaximumProperty(), 100.0);
}

TEST(RangeAttributeTests, MinMax_Stored_Int) {
    RangeAttribute a(0, 255);
    EXPECT_DOUBLE_EQ(a.getMinimumProperty(), 0.0);
    EXPECT_DOUBLE_EQ(a.getMaximumProperty(), 255.0);
}

TEST(StringLengthAttributeTests, MaxLength_Stored) {
    StringLengthAttribute a(50);
    EXPECT_EQ(a.getMaximumLengthProperty(), 50);
    EXPECT_EQ(a.getMinimumLengthProperty(), 0);
}

TEST(StringLengthAttributeTests, MinLength_Settable) {
    StringLengthAttribute a(20);
    a.setMinimumLengthProperty(5);
    EXPECT_EQ(a.getMinimumLengthProperty(), 5);
}

TEST(MaxLengthAttributeTests, Length_Stored) {
    MaxLengthAttribute a(100);
    EXPECT_EQ(a.getLengthProperty(), 100);
}

TEST(MinLengthAttributeTests, Length_Stored) {
    MinLengthAttribute a(3);
    EXPECT_EQ(a.getLengthProperty(), 3);
}

TEST(RegularExpressionAttributeTests, Pattern_Stored) {
    RegularExpressionAttribute a("[A-Z]+");
    EXPECT_EQ(a.getPatternProperty(), "[A-Z]+");
}

// ===========================================================================
// JsonSerializerOptions + Json enums
// ===========================================================================

using System::Text::Json::JsonSerializerOptions;
using System::Text::Json::JsonNamingPolicy;
using System::Text::Json::JsonCommentHandling;
using System::Text::Json::JsonValueKind;
using System::Text::Json::Serialization::JsonNumberHandling;

TEST(JsonSerializerOptionsTests, DefaultCtor_AllDefaults) {
    JsonSerializerOptions opts;
    EXPECT_FALSE(opts.getWriteIndentedProperty());
    EXPECT_FALSE(opts.getAllowTrailingCommasProperty());
    EXPECT_FALSE(opts.getPropertyNameCaseInsensitiveProperty());
    EXPECT_EQ(opts.getMaxDepthProperty(), 0);
}

TEST(JsonSerializerOptionsTests, WriteIndented_SetGet) {
    JsonSerializerOptions opts;
    opts.setWriteIndentedProperty(true);
    EXPECT_TRUE(opts.getWriteIndentedProperty());
}

TEST(JsonSerializerOptionsTests, AllowTrailingCommas_SetGet) {
    JsonSerializerOptions opts;
    opts.setAllowTrailingCommasProperty(true);
    EXPECT_TRUE(opts.getAllowTrailingCommasProperty());
}

TEST(JsonNamingPolicyTests, CamelCase_ConvertsFirstLetterToLowercase) {
    EXPECT_EQ(JsonNamingPolicy::CamelCase()->ConvertName("PropertyName"), "propertyName");
}

TEST(JsonNamingPolicyTests, SnakeCaseLower_InsertsUnderscoresAtWordBoundaries) {
    EXPECT_EQ(JsonNamingPolicy::SnakeCaseLower()->ConvertName("PropertyName"), "property_name");
}

TEST(JsonNamingPolicyTests, KebabCaseLower_InsertsHyphensAtWordBoundaries) {
    EXPECT_EQ(JsonNamingPolicy::KebabCaseLower()->ConvertName("PropertyName"), "property-name");
}

TEST(JsonCommentHandlingTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonCommentHandling::Disallow), 0);
    EXPECT_EQ(static_cast<int>(JsonCommentHandling::Skip),     1);
    EXPECT_EQ(static_cast<int>(JsonCommentHandling::Allow),    2);
}

TEST(JsonNumberHandlingTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonNumberHandling::Strict),                      0);
    EXPECT_EQ(static_cast<int>(JsonNumberHandling::AllowReadingFromString),      1);
    EXPECT_EQ(static_cast<int>(JsonNumberHandling::WriteAsString),               2);
    EXPECT_EQ(static_cast<int>(JsonNumberHandling::AllowNamedFloatingPointLiterals), 4);
}

TEST(JsonValueKindTests, Values) {
    EXPECT_EQ(static_cast<int>(JsonValueKind::Undefined), 0);
    EXPECT_EQ(static_cast<int>(JsonValueKind::Object),    1);
    EXPECT_EQ(static_cast<int>(JsonValueKind::Array),     2);
    EXPECT_EQ(static_cast<int>(JsonValueKind::String),    3);
    EXPECT_EQ(static_cast<int>(JsonValueKind::Number),    4);
    EXPECT_EQ(static_cast<int>(JsonValueKind::True),      5);
    EXPECT_EQ(static_cast<int>(JsonValueKind::False),     6);
    EXPECT_EQ(static_cast<int>(JsonValueKind::Null),      7);
}

// ===========================================================================
// JsonSerializationAttributes
// ===========================================================================

using System::Text::Json::Serialization::JsonPropertyNameAttribute;
using System::Text::Json::Serialization::JsonIgnoreAttribute;
using System::Text::Json::Serialization::JsonIgnoreCondition;
using System::Text::Json::Serialization::JsonConverterAttribute;
using System::Text::Json::Serialization::JsonPropertyOrderAttribute;

TEST(JsonPropertyNameAttributeTests, Name_Stored) {
    JsonPropertyNameAttribute a("my_name");
    EXPECT_EQ(a.getNameProperty(), "my_name");
}

TEST(JsonIgnoreAttributeTests, DefaultCtor_Always) {
    JsonIgnoreAttribute a;
    EXPECT_EQ(a.getConditionProperty(), JsonIgnoreCondition::Always);
}

TEST(JsonIgnoreAttributeTests, Condition_Stored) {
    JsonIgnoreAttribute a(JsonIgnoreCondition::WhenWritingNull);
    EXPECT_EQ(a.getConditionProperty(), JsonIgnoreCondition::WhenWritingNull);
}

TEST(JsonConverterAttributeTests, TypeName_Stored) {
    JsonConverterAttribute a("MyConverter");
    EXPECT_EQ(a.getConverterTypeNameProperty(), "MyConverter");
}

TEST(JsonPropertyOrderAttributeTests, Order_Stored) {
    JsonPropertyOrderAttribute a(3);
    EXPECT_EQ(a.getOrderProperty(), 3);
}

// ===========================================================================
// JsonSerializer
// ===========================================================================

using System::Text::Json::JsonSerializer;

TEST(JsonSerializerTests, Serialize_Int) {
    EXPECT_EQ(JsonSerializer::Serialize(42), "42");
}

TEST(JsonSerializerTests, Serialize_String) {
    EXPECT_EQ(JsonSerializer::Serialize(std::string("hello")), "\"hello\"");
}

TEST(JsonSerializerTests, Serialize_VectorOfInt) {
    EXPECT_EQ(JsonSerializer::Serialize(std::vector<int>{1, 2, 3}), "[1,2,3]");
}

TEST(JsonSerializerTests, Serialize_WriteIndented) {
    JsonSerializerOptions opts;
    opts.setWriteIndentedProperty(true);
    EXPECT_EQ(JsonSerializer::Serialize(std::vector<int>{1, 2}, opts), "[\n  1,\n  2\n]");
}

TEST(JsonSerializerTests, Deserialize_Int) {
    EXPECT_EQ(JsonSerializer::Deserialize<int>("42"), 42);
}

TEST(JsonSerializerTests, Deserialize_VectorOfInt) {
    auto v = JsonSerializer::Deserialize<std::vector<int>>("[1,2,3]");
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[2], 3);
}

TEST(JsonSerializerTests, Deserialize_ParsesJson) {
    auto doc = JsonSerializer::Deserialize("{\"x\":1}");
    EXPECT_NE(doc, nullptr);
}

// ===========================================================================
// DictionaryEntry
// ===========================================================================

using System::Collections::DictionaryEntry;

TEST(DictionaryEntryTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(DictionaryEntry de);
}

TEST(DictionaryEntryTests, KeyValue_Stored) {
    DictionaryEntry de(std::any(42), std::any(std::string("hello")));
    EXPECT_EQ(std::any_cast<int>(de.getKeyProperty()), 42);
    EXPECT_EQ(std::any_cast<std::string>(de.getValueProperty()), "hello");
}

// ===========================================================================
// PropertyDescriptorCollection
// ===========================================================================

using System::ComponentModel::PropertyDescriptorCollection;

TEST(PropertyDescriptorCollectionTests, DefaultCount_Zero) {
    PropertyDescriptorCollection col;
    EXPECT_EQ(col.getCountProperty(), 0);
}

// ===========================================================================
// SharpRuntime::Prop macros (DDATA / DGETTER / IDATA / IGETTER)
// ===========================================================================

// A minimal test class using the Prop macros — declared in this .cpp.
class PropBox {
    DDATA(int, Width)
    DGETTER(std::string, Label)
public:
    PropBox() = default;
    PropBox(int w, std::string lbl) : Width_(w), Label_(std::move(lbl)) {}
};

// Implement the declared properties at namespace scope (as IDATA/IGETTER require)
IDATA(int, Width, PropBox)
IGETTER(std::string, Label, PropBox)

TEST(PropMacroTests, DDATA_Getter_ReturnsValue) {
    PropBox box(42, "hello");
    EXPECT_EQ(box.getWidthProperty(), 42);
}

TEST(PropMacroTests, DDATA_Setter_UpdatesValue) {
    PropBox box;
    box.setWidthProperty(99);
    EXPECT_EQ(box.getWidthProperty(), 99);
}

TEST(PropMacroTests, DGETTER_ReturnsValue) {
    PropBox box(0, "world");
    EXPECT_EQ(box.getLabelProperty(), "world");
}

TEST(PropMacroTests, DDATA_MoveSet) {
    PropBox box;
    box.setWidthProperty(7);
    EXPECT_EQ(box.getWidthProperty(), 7);
}
