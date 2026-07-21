// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <sstream>
#include <string>
#include <tuple>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace detail {
    inline intcs tupleHashCombine(intcs h1, intcs h2) noexcept
    {
        return ((h1 << 5) + h1) ^ h2;
    }
    /**
     * Masks to the low 31 bits before narrowing std::hash<T>'s platform-dependent
     * (often 64-bit) result down to intcs, so the truncation is a defined, positive
     * value rather than implementation-defined narrowing of a possibly-negative bit
     * pattern. Combined via tupleHashCombine exactly as .NET's Tuple.CombineHashCodes does.
     */
    template<typename T>
    intcs tupleHash(const T& val)
    {
        return static_cast<intcs>(std::hash<T>{}(val) & 0x7fffffff);
    }
    template<typename T>
    std::string tupleItemStr(const T& val)
    {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << val;
        return oss.str();
    }
    /** @brief Three-way lexicographic comparison of a single element pair, matching .NET's Comparer<T>.Default.Compare. */
    template<typename T>
    intcs tupleCompare(const T& a, const T& b)
    {
        if (a < b) return -1;
        if (b < a) return 1;
        return 0;
    }
} // namespace detail

// ---------------------------------------------------------------------------
// Tuple1
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 1-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1>.
 * Wraps a single value with an Item1 field.
 */
template<typename T1>
struct Tuple1 {
    /** @brief The first element of the tuple. */
    T1 Item1;

    /** @brief Constructs a Tuple1 from one value. */
    explicit Tuple1(T1 i1) : Item1(std::move(i1)) {}

    /** @brief Deconstructs this tuple into its element. */
    void Deconstruct(T1& item1) const { item1 = Item1; }

    /** @brief Converts this tuple to an equivalent std::tuple. */
    [[nodiscard]] std::tuple<T1> ToStdTuple() const { return {Item1}; }

    /** @brief Returns true if both tuples have equal elements. */
    bool operator==(const Tuple1& o) const { return Item1 == o.Item1; }

    /** @brief Returns true if any element differs. */
    bool operator!=(const Tuple1& o) const { return !(*this == o); }

    /**
     * @brief Compares this tuple to @p o element-by-element.
     *
     * C++ counterpart of .NET Tuple<T1>.CompareTo (via IStructuralComparable).
     */
    [[nodiscard]] intcs CompareTo(const Tuple1& o) const
    {
        return detail::tupleCompare(Item1, o.Item1);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple1& o) const { return CompareTo(o) < 0; }

    /**
     * @brief Returns a string representation in the form "(item1)".
     *
     * C++ counterpart of .NET Tuple<T1>.ToString().
     */
    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ")";
    }

    /**
     * @brief Returns a hash code for this tuple.
     *
     * C++ counterpart of .NET Tuple<T1>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHash(Item1);
    }
};

// ---------------------------------------------------------------------------
// Tuple2
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 2-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2>.
 */
template<typename T1, typename T2>
struct Tuple2 {
    /** @brief The first element of the tuple. */
    T1 Item1;
    /** @brief The second element of the tuple. */
    T2 Item2;

    /** @brief Constructs a Tuple2 from two values. */
    Tuple2(T1 i1, T2 i2) : Item1(std::move(i1)), Item2(std::move(i2)) {}

    /** @brief Deconstructs this tuple into its elements. */
    void Deconstruct(T1& item1, T2& item2) const { item1 = Item1; item2 = Item2; }

    /** @brief Converts this tuple to an equivalent std::tuple. */
    [[nodiscard]] std::tuple<T1, T2> ToStdTuple() const { return {Item1, Item2}; }

    /** @brief Returns true if all elements compare equal. */
    bool operator==(const Tuple2& o) const { return Item1 == o.Item1 && Item2 == o.Item2; }

    /** @brief Returns true if any element differs. */
    bool operator!=(const Tuple2& o) const { return !(*this == o); }

    /**
     * @brief Compares this tuple to @p o element-by-element.
     *
     * C++ counterpart of .NET Tuple<T1, T2>.CompareTo (via IStructuralComparable).
     */
    [[nodiscard]] intcs CompareTo(const Tuple2& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        return detail::tupleCompare(Item2, o.Item2);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple2& o) const { return CompareTo(o) < 0; }

    /**
     * @brief Returns a string representation in the form "(item1, item2)".
     *
     * C++ counterpart of .NET Tuple<T1, T2>.ToString().
     */
    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", " + detail::tupleItemStr(Item2) + ")";
    }

    /**
     * @brief Returns a hash code for this tuple.
     *
     * C++ counterpart of .NET Tuple<T1, T2>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2));
    }
};

// ---------------------------------------------------------------------------
// Tuple3
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 3-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3>.
 */
template<typename T1, typename T2, typename T3>
struct Tuple3 {
    /** @brief The first element of the tuple. */
    T1 Item1;
    /** @brief The second element of the tuple. */
    T2 Item2;
    /** @brief The third element of the tuple. */
    T3 Item3;

    /** @brief Constructs a Tuple3 from three values. */
    Tuple3(T1 i1, T2 i2, T3 i3)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)) {}

    /** @brief Deconstructs this tuple into its elements. */
    void Deconstruct(T1& item1, T2& item2, T3& item3) const
    {
        item1 = Item1; item2 = Item2; item3 = Item3;
    }

    /** @brief Converts this tuple to an equivalent std::tuple. */
    [[nodiscard]] std::tuple<T1, T2, T3> ToStdTuple() const { return {Item1, Item2, Item3}; }

    /** @brief Returns true if all elements compare equal. */
    bool operator==(const Tuple3& o) const
    {
        return Item1 == o.Item1 && Item2 == o.Item2 && Item3 == o.Item3;
    }

    /** @brief Returns true if any element differs. */
    bool operator!=(const Tuple3& o) const { return !(*this == o); }

    /**
     * @brief Compares this tuple to @p o element-by-element.
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3>.CompareTo (via IStructuralComparable).
     */
    [[nodiscard]] intcs CompareTo(const Tuple3& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        return detail::tupleCompare(Item3, o.Item3);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple3& o) const { return CompareTo(o) < 0; }

    /**
     * @brief Returns a string representation in the form "(item1, item2, item3)".
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3>.ToString().
     */
    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ")";
    }

    /**
     * @brief Returns a hash code for this tuple.
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
            detail::tupleHash(Item3));
    }
};

// ---------------------------------------------------------------------------
// Tuple4
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 4-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3, T4>.
 */
template<typename T1, typename T2, typename T3, typename T4>
struct Tuple4 {
    /** @brief The first element of the tuple. */
    T1 Item1;
    /** @brief The second element of the tuple. */
    T2 Item2;
    /** @brief The third element of the tuple. */
    T3 Item3;
    /** @brief The fourth element of the tuple. */
    T4 Item4;

    /** @brief Constructs a Tuple4 from four values. */
    Tuple4(T1 i1, T2 i2, T3 i3, T4 i4)
        : Item1(std::move(i1)), Item2(std::move(i2)),
          Item3(std::move(i3)), Item4(std::move(i4)) {}

    /** @brief Deconstructs this tuple into its elements. */
    void Deconstruct(T1& item1, T2& item2, T3& item3, T4& item4) const
    {
        item1 = Item1; item2 = Item2; item3 = Item3; item4 = Item4;
    }

    /** @brief Converts this tuple to an equivalent std::tuple. */
    [[nodiscard]] std::tuple<T1, T2, T3, T4> ToStdTuple() const
    {
        return {Item1, Item2, Item3, Item4};
    }

    /** @brief Returns true if all elements compare equal. */
    bool operator==(const Tuple4& o) const
    {
        return Item1 == o.Item1 && Item2 == o.Item2 && Item3 == o.Item3 && Item4 == o.Item4;
    }

    /** @brief Returns true if any element differs. */
    bool operator!=(const Tuple4& o) const { return !(*this == o); }

    /**
     * @brief Compares this tuple to @p o element-by-element.
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3, T4>.CompareTo (via IStructuralComparable).
     */
    [[nodiscard]] intcs CompareTo(const Tuple4& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        c = detail::tupleCompare(Item3, o.Item3);
        if (c != 0) return c;
        return detail::tupleCompare(Item4, o.Item4);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple4& o) const { return CompareTo(o) < 0; }

    /**
     * @brief Returns a string representation in the form "(item1, item2, item3, item4)".
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3, T4>.ToString().
     */
    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ", "
                   + detail::tupleItemStr(Item4) + ")";
    }

    /**
     * @brief Returns a hash code for this tuple.
     *
     * C++ counterpart of .NET Tuple<T1, T2, T3, T4>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
            detail::tupleHashCombine(detail::tupleHash(Item3), detail::tupleHash(Item4)));
    }
};

// ---------------------------------------------------------------------------
// Tuple5
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 5-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3, T4, T5>.
 */
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Tuple5 {
    T1 Item1; /**< @brief The first element. */
    T2 Item2; /**< @brief The second element. */
    T3 Item3; /**< @brief The third element. */
    T4 Item4; /**< @brief The fourth element. */
    T5 Item5; /**< @brief The fifth element. */

    /** @brief Constructs a Tuple5 from five values. */
    Tuple5(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)) {}

    /** @brief Deconstructs this tuple into its elements. */
    void Deconstruct(T1& i1, T2& i2, T3& i3, T4& i4, T5& i5) const
    {
        i1 = Item1; i2 = Item2; i3 = Item3; i4 = Item4; i5 = Item5;
    }

    /** @brief Converts this tuple to an equivalent std::tuple. */
    [[nodiscard]] std::tuple<T1, T2, T3, T4, T5> ToStdTuple() const
    {
        return {Item1, Item2, Item3, Item4, Item5};
    }

    bool operator==(const Tuple5& o) const
    {
        return Item1 == o.Item1 && Item2 == o.Item2 && Item3 == o.Item3
            && Item4 == o.Item4 && Item5 == o.Item5;
    }
    bool operator!=(const Tuple5& o) const { return !(*this == o); }

    /** @brief Compares this tuple to @p o element-by-element (matches .NET's IStructuralComparable). */
    [[nodiscard]] intcs CompareTo(const Tuple5& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        c = detail::tupleCompare(Item3, o.Item3);
        if (c != 0) return c;
        c = detail::tupleCompare(Item4, o.Item4);
        if (c != 0) return c;
        return detail::tupleCompare(Item5, o.Item5);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple5& o) const { return CompareTo(o) < 0; }

    /** @brief Returns a string in the form "(item1, …, item5)". */
    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ", "
                   + detail::tupleItemStr(Item4) + ", "
                   + detail::tupleItemStr(Item5) + ")";
    }

    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
                detail::tupleHashCombine(detail::tupleHash(Item3), detail::tupleHash(Item4))),
            detail::tupleHash(Item5));
    }
};

// ---------------------------------------------------------------------------
// Tuple6
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 6-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3, T4, T5, T6>.
 */
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Tuple6 {
    T1 Item1; T2 Item2; T3 Item3; T4 Item4; T5 Item5; T6 Item6;

    Tuple6(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)) {}

    void Deconstruct(T1& i1, T2& i2, T3& i3, T4& i4, T5& i5, T6& i6) const
    {
        i1=Item1; i2=Item2; i3=Item3; i4=Item4; i5=Item5; i6=Item6;
    }

    [[nodiscard]] std::tuple<T1,T2,T3,T4,T5,T6> ToStdTuple() const
    {
        return {Item1, Item2, Item3, Item4, Item5, Item6};
    }

    bool operator==(const Tuple6& o) const
    {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3
            && Item4==o.Item4 && Item5==o.Item5 && Item6==o.Item6;
    }
    bool operator!=(const Tuple6& o) const { return !(*this == o); }

    /** @brief Compares this tuple to @p o element-by-element (matches .NET's IStructuralComparable). */
    [[nodiscard]] intcs CompareTo(const Tuple6& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        c = detail::tupleCompare(Item3, o.Item3);
        if (c != 0) return c;
        c = detail::tupleCompare(Item4, o.Item4);
        if (c != 0) return c;
        c = detail::tupleCompare(Item5, o.Item5);
        if (c != 0) return c;
        return detail::tupleCompare(Item6, o.Item6);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple6& o) const { return CompareTo(o) < 0; }

    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ", "
                   + detail::tupleItemStr(Item4) + ", "
                   + detail::tupleItemStr(Item5) + ", "
                   + detail::tupleItemStr(Item6) + ")";
    }

    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
                detail::tupleHashCombine(detail::tupleHash(Item3), detail::tupleHash(Item4))),
            detail::tupleHashCombine(detail::tupleHash(Item5), detail::tupleHash(Item6)));
    }
};

// ---------------------------------------------------------------------------
// Tuple7
// ---------------------------------------------------------------------------

/**
 * @brief Represents a 7-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3, T4, T5, T6, T7>.
 */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7>
struct Tuple7 {
    T1 Item1; T2 Item2; T3 Item3; T4 Item4; T5 Item5; T6 Item6; T7 Item7;

    Tuple7(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)),
          Item7(std::move(i7)) {}

    void Deconstruct(T1& i1, T2& i2, T3& i3, T4& i4, T5& i5, T6& i6, T7& i7) const
    {
        i1=Item1; i2=Item2; i3=Item3; i4=Item4; i5=Item5; i6=Item6; i7=Item7;
    }

    [[nodiscard]] std::tuple<T1,T2,T3,T4,T5,T6,T7> ToStdTuple() const
    {
        return {Item1, Item2, Item3, Item4, Item5, Item6, Item7};
    }

    bool operator==(const Tuple7& o) const
    {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 && Item4==o.Item4
            && Item5==o.Item5 && Item6==o.Item6 && Item7==o.Item7;
    }
    bool operator!=(const Tuple7& o) const { return !(*this == o); }

    /** @brief Compares this tuple to @p o element-by-element (matches .NET's IStructuralComparable). */
    [[nodiscard]] intcs CompareTo(const Tuple7& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        c = detail::tupleCompare(Item3, o.Item3);
        if (c != 0) return c;
        c = detail::tupleCompare(Item4, o.Item4);
        if (c != 0) return c;
        c = detail::tupleCompare(Item5, o.Item5);
        if (c != 0) return c;
        c = detail::tupleCompare(Item6, o.Item6);
        if (c != 0) return c;
        return detail::tupleCompare(Item7, o.Item7);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple7& o) const { return CompareTo(o) < 0; }

    [[nodiscard]] std::string ToString() const
    {
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ", "
                   + detail::tupleItemStr(Item4) + ", "
                   + detail::tupleItemStr(Item5) + ", "
                   + detail::tupleItemStr(Item6) + ", "
                   + detail::tupleItemStr(Item7) + ")";
    }

    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
                detail::tupleHashCombine(detail::tupleHash(Item3), detail::tupleHash(Item4))),
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item5), detail::tupleHash(Item6)),
                detail::tupleHash(Item7)));
    }
};

// ---------------------------------------------------------------------------
// Tuple8
// ---------------------------------------------------------------------------

/**
 * @brief Represents an eight-or-more-element tuple.
 *
 * C++ counterpart of .NET System.Tuple<T1, T2, T3, T4, T5, T6, T7, TRest>.
 * @p TRest should be another TupleN to represent additional elements, matching
 * .NET's Tuple.Create(T1..T8) convention (which wraps item8 in a Tuple1<T8>).
 */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7, typename TRest>
struct Tuple8 {
    /** @brief The first element of the tuple. */
    T1 Item1;
    /** @brief The second element of the tuple. */
    T2 Item2;
    /** @brief The third element of the tuple. */
    T3 Item3;
    /** @brief The fourth element of the tuple. */
    T4 Item4;
    /** @brief The fifth element of the tuple. */
    T5 Item5;
    /** @brief The sixth element of the tuple. */
    T6 Item6;
    /** @brief The seventh element of the tuple. */
    T7 Item7;
    /** @brief The remaining elements (another TupleN). */
    TRest Rest;

    /** @brief Constructs a Tuple8 from seven values and a Rest tuple. */
    Tuple8(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7, TRest rest)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)),
          Item7(std::move(i7)), Rest(std::move(rest)) {}

    /** @brief Converts this tuple to an equivalent std::tuple (Rest kept as its own element, not flattened). */
    [[nodiscard]] std::tuple<T1, T2, T3, T4, T5, T6, T7, TRest> ToStdTuple() const
    {
        return {Item1, Item2, Item3, Item4, Item5, Item6, Item7, Rest};
    }

    /** @brief Returns true if all elements, including Rest, compare equal. */
    bool operator==(const Tuple8& o) const
    {
        return Item1 == o.Item1 && Item2 == o.Item2 && Item3 == o.Item3 && Item4 == o.Item4
            && Item5 == o.Item5 && Item6 == o.Item6 && Item7 == o.Item7 && Rest == o.Rest;
    }

    /** @brief Returns true if any element differs. */
    bool operator!=(const Tuple8& o) const { return !(*this == o); }

    /**
     * @brief Compares this tuple to @p o element-by-element, then Rest.
     *
     * C++ counterpart of .NET Tuple<T1..T7,TRest>.CompareTo (via IStructuralComparable).
     */
    [[nodiscard]] intcs CompareTo(const Tuple8& o) const
    {
        intcs c = detail::tupleCompare(Item1, o.Item1);
        if (c != 0) return c;
        c = detail::tupleCompare(Item2, o.Item2);
        if (c != 0) return c;
        c = detail::tupleCompare(Item3, o.Item3);
        if (c != 0) return c;
        c = detail::tupleCompare(Item4, o.Item4);
        if (c != 0) return c;
        c = detail::tupleCompare(Item5, o.Item5);
        if (c != 0) return c;
        c = detail::tupleCompare(Item6, o.Item6);
        if (c != 0) return c;
        c = detail::tupleCompare(Item7, o.Item7);
        if (c != 0) return c;
        return detail::tupleCompare(Rest, o.Rest);
    }

    /** @brief Returns true if this tuple sorts before @p o. */
    bool operator<(const Tuple8& o) const { return CompareTo(o) < 0; }

    /**
     * @brief Returns a string representation, flattening Rest inline:
     * "(item1, item2, ..., item7, rest_item1, rest_item2, ...)".
     *
     * C++ counterpart of .NET Tuple<T1..T7,TRest>.ToString().
     */
    [[nodiscard]] std::string ToString() const
    {
        std::string restStr = Rest.ToString();
        // Rest.ToString() yields "(x)" or "(x, y, ...)" -- strip outer parens
        std::string restInner = restStr.size() >= 2
            ? restStr.substr(1, restStr.size() - 2) : "";
        return "(" + detail::tupleItemStr(Item1) + ", "
                   + detail::tupleItemStr(Item2) + ", "
                   + detail::tupleItemStr(Item3) + ", "
                   + detail::tupleItemStr(Item4) + ", "
                   + detail::tupleItemStr(Item5) + ", "
                   + detail::tupleItemStr(Item6) + ", "
                   + detail::tupleItemStr(Item7) + ", " + restInner + ")";
    }

    /**
     * @brief Returns a hash code for this tuple.
     *
     * C++ counterpart of .NET Tuple<T1..T7,TRest>.GetHashCode(). Matches real .NET's
     * exact algorithm for the common case (Rest is a single-element tuple, i.e. the
     * shape produced by Tuple::Create(item1..item8)); deeper manual nesting (a Rest
     * with 8+ elements of its own) isn't specially collapsed the way .NET's rotating
     * 8-element window is, since .NET's GetHashCode() values are documented as
     * unspecified/not to be persisted regardless.
     */
    [[nodiscard]] intcs GetHashCode() const
    {
        return detail::tupleHashCombine(
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item1), detail::tupleHash(Item2)),
                detail::tupleHashCombine(detail::tupleHash(Item3), detail::tupleHash(Item4))),
            detail::tupleHashCombine(
                detail::tupleHashCombine(detail::tupleHash(Item5), detail::tupleHash(Item6)),
                detail::tupleHashCombine(detail::tupleHash(Item7), Rest.GetHashCode())));
    }
};

// ---------------------------------------------------------------------------
// Tuple factory (mirrors .NET static Tuple.Create<T...>() methods)
// ---------------------------------------------------------------------------

/**
 * @brief Provides static factory methods that create Tuple instances.
 *
 * C++ counterpart of the static .NET System.Tuple class.
 * Use Tuple::Create(...) to construct typed tuples with inferred element types.
 */
struct Tuple {
    Tuple() = delete;

    /** @brief Creates a 1-element tuple. */
    template<typename T1>
    [[nodiscard]] static Tuple1<T1> Create(T1 item1)
    {
        return Tuple1<T1>(std::move(item1));
    }

    /** @brief Creates a 2-element tuple. */
    template<typename T1, typename T2>
    [[nodiscard]] static Tuple2<T1, T2> Create(T1 item1, T2 item2)
    {
        return Tuple2<T1, T2>(std::move(item1), std::move(item2));
    }

    /** @brief Creates a 3-element tuple. */
    template<typename T1, typename T2, typename T3>
    [[nodiscard]] static Tuple3<T1, T2, T3> Create(T1 item1, T2 item2, T3 item3)
    {
        return Tuple3<T1, T2, T3>(std::move(item1), std::move(item2), std::move(item3));
    }

    /** @brief Creates a 4-element tuple. */
    template<typename T1, typename T2, typename T3, typename T4>
    [[nodiscard]] static Tuple4<T1, T2, T3, T4> Create(T1 item1, T2 item2, T3 item3, T4 item4)
    {
        return Tuple4<T1, T2, T3, T4>(std::move(item1), std::move(item2),
                                      std::move(item3), std::move(item4));
    }

    /** @brief Creates a 5-element tuple. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    [[nodiscard]] static Tuple5<T1,T2,T3,T4,T5> Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5)
    {
        return Tuple5<T1,T2,T3,T4,T5>(std::move(i1),std::move(i2),std::move(i3),
                                      std::move(i4),std::move(i5));
    }

    /** @brief Creates a 6-element tuple. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    [[nodiscard]] static Tuple6<T1,T2,T3,T4,T5,T6> Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6)
    {
        return Tuple6<T1,T2,T3,T4,T5,T6>(std::move(i1),std::move(i2),std::move(i3),
                                         std::move(i4),std::move(i5),std::move(i6));
    }

    /** @brief Creates a 7-element tuple. */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
    [[nodiscard]] static Tuple7<T1,T2,T3,T4,T5,T6,T7>
    Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7)
    {
        return Tuple7<T1,T2,T3,T4,T5,T6,T7>(std::move(i1),std::move(i2),std::move(i3),
                                             std::move(i4),std::move(i5),std::move(i6),
                                             std::move(i7));
    }

    /**
     * @brief Creates an 8-element tuple.
     *
     * C++ counterpart of .NET Tuple.Create<T1..T8>(T1..T8).
     * Wraps item8 in a Tuple1 as the Rest field, matching .NET convention.
     */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
    [[nodiscard]] static Tuple8<T1,T2,T3,T4,T5,T6,T7,Tuple1<T8>>
    Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7, T8 i8)
    {
        return Tuple8<T1,T2,T3,T4,T5,T6,T7,Tuple1<T8>>(
            std::move(i1), std::move(i2), std::move(i3), std::move(i4),
            std::move(i5), std::move(i6), std::move(i7), Tuple1<T8>(std::move(i8)));
    }
};

} // namespace System
