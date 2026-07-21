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

namespace detail {

    inline size_t vtHashCombine(size_t seed, size_t h) noexcept {
        return seed ^ (h + 0x9e3779b9u + (seed << 6) + (seed >> 2));
    }

    template<typename T>
    std::string vtItemStr(const T& v) {
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        oss << v;
        return oss.str();
    }

    template<typename T>
    size_t vtHash(const T& v) noexcept {
        return std::hash<T>{}(v);
    }

} // namespace detail

// ===========================================================================
// ValueTuple1<T1>  —  System.ValueTuple<T1>
// ===========================================================================

/**
 * @brief Value-semantic single-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1>.
 */
template<typename T1>
struct ValueTuple1 {
    /** @brief The first (and only) element. */
    T1 Item1;

    /** @brief Default-constructs all elements. */
    ValueTuple1() = default;

    /** @brief Constructs a ValueTuple1 with the specified element. */
    explicit ValueTuple1(T1 item1) : Item1(std::move(item1)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple1& other) const {
        return Item1 == other.Item1;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        return static_cast<intcs>(detail::vtHash(Item1));
    }

    /**
     * @brief Compares this instance to another ValueTuple1.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple1& other) const {
        if (Item1 < other.Item1) return -1;
        if (other.Item1 < Item1) return  1;
        return 0;
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1>.ToString() — formats as "(Item1)".
     */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ")";
    }

    bool operator==(const ValueTuple1& o) const { return Item1 == o.Item1; }
    bool operator!=(const ValueTuple1& o) const { return !(*this == o); }
    bool operator< (const ValueTuple1& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple1& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple1& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple1& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple2<T1,T2>  —  System.ValueTuple<T1,T2>
// ===========================================================================

/**
 * @brief Value-semantic two-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2>.
 */
template<typename T1, typename T2>
struct ValueTuple2 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;

    /** @brief Default-constructs all elements. */
    ValueTuple2() = default;

    /** @brief Constructs a ValueTuple2 with the specified elements. */
    ValueTuple2(T1 i1, T2 i2) : Item1(std::move(i1)), Item2(std::move(i2)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple2& other) const {
        return Item1 == other.Item1 && Item2 == other.Item2;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1,T2>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple2 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple2& other) const {
        if (Item1 < other.Item1) return -1;
        if (other.Item1 < Item1) return  1;
        if (Item2 < other.Item2) return -1;
        if (other.Item2 < Item2) return  1;
        return 0;
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1,T2>.ToString() — formats as "(Item1, Item2)".
     */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " + detail::vtItemStr(Item2) + ")";
    }

    bool operator==(const ValueTuple2& o) const { return Item1 == o.Item1 && Item2 == o.Item2; }
    bool operator!=(const ValueTuple2& o) const { return !(*this == o); }
    bool operator< (const ValueTuple2& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple2& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple2& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple2& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple3<T1,T2,T3>  —  System.ValueTuple<T1,T2,T3>
// ===========================================================================

/**
 * @brief Value-semantic three-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3>.
 */
template<typename T1, typename T2, typename T3>
struct ValueTuple3 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;
    /** @brief The third element. */
    T3 Item3;

    /** @brief Default-constructs all elements. */
    ValueTuple3() = default;

    /** @brief Constructs a ValueTuple3 with the specified elements. */
    ValueTuple3(T1 i1, T2 i2, T3 i3)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple3& other) const {
        return Item1 == other.Item1 && Item2 == other.Item2 && Item3 == other.Item3;
    }

    /** @brief Returns a hash code for this instance. */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple3 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple3& other) const {
        if (Item1 < other.Item1) return -1;
        if (other.Item1 < Item1) return  1;
        if (Item2 < other.Item2) return -1;
        if (other.Item2 < Item2) return  1;
        if (Item3 < other.Item3) return -1;
        if (other.Item3 < Item3) return  1;
        return 0;
    }

    /** @brief Returns a string that represents the value of this instance. */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " +
                     detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ")";
    }

    bool operator==(const ValueTuple3& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3;
    }
    bool operator!=(const ValueTuple3& o) const { return !(*this == o); }
    bool operator< (const ValueTuple3& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple3& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple3& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple3& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple4<T1,T2,T3,T4>  —  System.ValueTuple<T1,T2,T3,T4>
// ===========================================================================

/**
 * @brief Value-semantic four-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3, T4>.
 */
template<typename T1, typename T2, typename T3, typename T4>
struct ValueTuple4 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;
    /** @brief The third element. */
    T3 Item3;
    /** @brief The fourth element. */
    T4 Item4;

    /** @brief Default-constructs all elements. */
    ValueTuple4() = default;

    /** @brief Constructs a ValueTuple4 with the specified elements. */
    ValueTuple4(T1 i1, T2 i2, T3 i3, T4 i4)
        : Item1(std::move(i1)), Item2(std::move(i2)),
          Item3(std::move(i3)), Item4(std::move(i4)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple4& other) const {
        return Item1==other.Item1 && Item2==other.Item2 &&
               Item3==other.Item3 && Item4==other.Item4;
    }

    /** @brief Returns a hash code for this instance. */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        h = detail::vtHashCombine(h, detail::vtHash(Item4));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple4 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple4& other) const {
        if (Item1 < other.Item1) return -1;
        if (other.Item1 < Item1) return 1;
        if (Item2 < other.Item2) return -1;
        if (other.Item2 < Item2) return 1;
        if (Item3 < other.Item3) return -1;
        if (other.Item3 < Item3) return 1;
        if (Item4 < other.Item4) return -1;
        if (other.Item4 < Item4) return 1;
        return 0;
    }

    /** @brief Returns a string that represents the value of this instance. */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " +
                     detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ", " +
                     detail::vtItemStr(Item4) + ")";
    }

    bool operator==(const ValueTuple4& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 && Item4==o.Item4;
    }
    bool operator!=(const ValueTuple4& o) const { return !(*this == o); }
    bool operator< (const ValueTuple4& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple4& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple4& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple4& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple5<T1..T5>  —  System.ValueTuple<T1,T2,T3,T4,T5>
// ===========================================================================

/**
 * @brief Value-semantic five-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3, T4, T5>.
 */
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct ValueTuple5 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;
    /** @brief The third element. */
    T3 Item3;
    /** @brief The fourth element. */
    T4 Item4;
    /** @brief The fifth element. */
    T5 Item5;

    /** @brief Default-constructs all elements. */
    ValueTuple5() = default;

    /** @brief Constructs a ValueTuple5 with the specified elements. */
    ValueTuple5(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple5& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 &&
               Item4==o.Item4 && Item5==o.Item5;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T5>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        h = detail::vtHashCombine(h, detail::vtHash(Item4));
        h = detail::vtHashCombine(h, detail::vtHash(Item5));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple5 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple5& o) const {
        if (Item1 < o.Item1) return -1;
        if (o.Item1 < Item1) return 1;
        if (Item2 < o.Item2) return -1;
        if (o.Item2 < Item2) return 1;
        if (Item3 < o.Item3) return -1;
        if (o.Item3 < Item3) return 1;
        if (Item4 < o.Item4) return -1;
        if (o.Item4 < Item4) return 1;
        if (Item5 < o.Item5) return -1;
        if (o.Item5 < Item5) return 1;
        return 0;
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T5>.ToString() — formats as
     * "(Item1, Item2, Item3, Item4, Item5)".
     */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " + detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ", " + detail::vtItemStr(Item4) + ", " +
                     detail::vtItemStr(Item5) + ")";
    }
    bool operator==(const ValueTuple5& o) const { return Equals(o); }
    bool operator!=(const ValueTuple5& o) const { return !Equals(o); }
    bool operator< (const ValueTuple5& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple5& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple5& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple5& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple6<T1..T6>  —  System.ValueTuple<T1,T2,T3,T4,T5,T6>
// ===========================================================================

/**
 * @brief Value-semantic six-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3, T4, T5, T6>.
 */
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct ValueTuple6 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;
    /** @brief The third element. */
    T3 Item3;
    /** @brief The fourth element. */
    T4 Item4;
    /** @brief The fifth element. */
    T5 Item5;
    /** @brief The sixth element. */
    T6 Item6;

    /** @brief Default-constructs all elements. */
    ValueTuple6() = default;

    /** @brief Constructs a ValueTuple6 with the specified elements. */
    ValueTuple6(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple6& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 &&
               Item4==o.Item4 && Item5==o.Item5 && Item6==o.Item6;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T6>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        h = detail::vtHashCombine(h, detail::vtHash(Item4));
        h = detail::vtHashCombine(h, detail::vtHash(Item5));
        h = detail::vtHashCombine(h, detail::vtHash(Item6));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple6 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple6& o) const {
        if (Item1 < o.Item1) return -1;
        if (o.Item1 < Item1) return 1;
        if (Item2 < o.Item2) return -1;
        if (o.Item2 < Item2) return 1;
        if (Item3 < o.Item3) return -1;
        if (o.Item3 < Item3) return 1;
        if (Item4 < o.Item4) return -1;
        if (o.Item4 < Item4) return 1;
        if (Item5 < o.Item5) return -1;
        if (o.Item5 < Item5) return 1;
        if (Item6 < o.Item6) return -1;
        if (o.Item6 < Item6) return 1;
        return 0;
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T6>.ToString() — formats as
     * "(Item1, Item2, Item3, Item4, Item5, Item6)".
     */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " + detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ", " + detail::vtItemStr(Item4) + ", " +
                     detail::vtItemStr(Item5) + ", " + detail::vtItemStr(Item6) + ")";
    }
    bool operator==(const ValueTuple6& o) const { return Equals(o); }
    bool operator!=(const ValueTuple6& o) const { return !Equals(o); }
    bool operator< (const ValueTuple6& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple6& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple6& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple6& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// ValueTuple7<T1..T7>  —  System.ValueTuple<T1,T2,T3,T4,T5,T6,T7>
// ===========================================================================

/**
 * @brief Value-semantic seven-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3, T4, T5, T6, T7>.
 */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7>
struct ValueTuple7 {
    /** @brief The first element. */
    T1 Item1;
    /** @brief The second element. */
    T2 Item2;
    /** @brief The third element. */
    T3 Item3;
    /** @brief The fourth element. */
    T4 Item4;
    /** @brief The fifth element. */
    T5 Item5;
    /** @brief The sixth element. */
    T6 Item6;
    /** @brief The seventh element. */
    T7 Item7;

    /** @brief Default-constructs all elements. */
    ValueTuple7() = default;

    /** @brief Constructs a ValueTuple7 with the specified elements. */
    ValueTuple7(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)),
          Item7(std::move(i7)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple7& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 && Item4==o.Item4 &&
               Item5==o.Item5 && Item6==o.Item6 && Item7==o.Item7;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T7>.GetHashCode().
     */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        h = detail::vtHashCombine(h, detail::vtHash(Item4));
        h = detail::vtHashCombine(h, detail::vtHash(Item5));
        h = detail::vtHashCombine(h, detail::vtHash(Item6));
        h = detail::vtHashCombine(h, detail::vtHash(Item7));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple7 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple7& o) const {
        if (Item1 < o.Item1) return -1;
        if (o.Item1 < Item1) return 1;
        if (Item2 < o.Item2) return -1;
        if (o.Item2 < Item2) return 1;
        if (Item3 < o.Item3) return -1;
        if (o.Item3 < Item3) return 1;
        if (Item4 < o.Item4) return -1;
        if (o.Item4 < Item4) return 1;
        if (Item5 < o.Item5) return -1;
        if (o.Item5 < Item5) return 1;
        if (Item6 < o.Item6) return -1;
        if (o.Item6 < Item6) return 1;
        if (Item7 < o.Item7) return -1;
        if (o.Item7 < Item7) return 1;
        return 0;
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * C++ counterpart of .NET ValueTuple<T1..T7>.ToString() — formats as
     * "(Item1, Item2, Item3, Item4, Item5, Item6, Item7)".
     */
    [[nodiscard]] std::string ToString() const {
        return "(" + detail::vtItemStr(Item1) + ", " + detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ", " + detail::vtItemStr(Item4) + ", " +
                     detail::vtItemStr(Item5) + ", " + detail::vtItemStr(Item6) + ", " +
                     detail::vtItemStr(Item7) + ")";
    }
    bool operator==(const ValueTuple7& o) const { return Equals(o); }
    bool operator!=(const ValueTuple7& o) const { return !Equals(o); }
    bool operator< (const ValueTuple7& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple7& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple7& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple7& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// Factory helpers  —  System.ValueTuple.Create(...)
// ===========================================================================

/** @brief Creates a ValueTuple1 containing the specified value. */
template<typename T1>
ValueTuple1<T1> MakeValueTuple(T1 i1) { return ValueTuple1<T1>(std::move(i1)); }

/** @brief Creates a ValueTuple2 containing the specified values. */
template<typename T1, typename T2>
ValueTuple2<T1,T2> MakeValueTuple(T1 i1, T2 i2) {
    return {std::move(i1), std::move(i2)};
}

/** @brief Creates a ValueTuple3 containing the specified values. */
template<typename T1, typename T2, typename T3>
ValueTuple3<T1,T2,T3> MakeValueTuple(T1 i1, T2 i2, T3 i3) {
    return {std::move(i1), std::move(i2), std::move(i3)};
}

/** @brief Creates a ValueTuple4 containing the specified values. */
template<typename T1, typename T2, typename T3, typename T4>
ValueTuple4<T1,T2,T3,T4> MakeValueTuple(T1 i1, T2 i2, T3 i3, T4 i4) {
    return {std::move(i1), std::move(i2), std::move(i3), std::move(i4)};
}

/** @brief Creates a ValueTuple5 containing the specified values. */
template<typename T1, typename T2, typename T3, typename T4, typename T5>
ValueTuple5<T1,T2,T3,T4,T5> MakeValueTuple(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5) {
    return {std::move(i1), std::move(i2), std::move(i3), std::move(i4), std::move(i5)};
}

/** @brief Creates a ValueTuple6 containing the specified values. */
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
ValueTuple6<T1,T2,T3,T4,T5,T6> MakeValueTuple(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6) {
    return {std::move(i1), std::move(i2), std::move(i3),
            std::move(i4), std::move(i5), std::move(i6)};
}

/** @brief Creates a ValueTuple7 containing the specified values. */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7>
ValueTuple7<T1,T2,T3,T4,T5,T6,T7> MakeValueTuple(T1 i1, T2 i2, T3 i3, T4 i4,
                                                    T5 i5, T6 i6, T7 i7) {
    return {std::move(i1), std::move(i2), std::move(i3), std::move(i4),
            std::move(i5), std::move(i6), std::move(i7)};
}

// ===========================================================================
// ValueTuple8<T1..T7,TRest>  —  System.ValueTuple<T1,T2,T3,T4,T5,T6,T7,TRest>
// ===========================================================================

/**
 * @brief Value-semantic eight-or-more-element tuple.
 *
 * C++ counterpart of .NET System.ValueTuple<T1, T2, T3, T4, T5, T6, T7, TRest>.
 * @p TRest should be another ValueTupleN to represent additional elements.
 */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7, typename TRest>
struct ValueTuple8 {
    /** @brief The first element. */
    T1    Item1;
    /** @brief The second element. */
    T2    Item2;
    /** @brief The third element. */
    T3    Item3;
    /** @brief The fourth element. */
    T4    Item4;
    /** @brief The fifth element. */
    T5    Item5;
    /** @brief The sixth element. */
    T6    Item6;
    /** @brief The seventh element. */
    T7    Item7;
    /** @brief The remaining elements (another ValueTupleN). */
    TRest Rest;

    /** @brief Default-constructs all elements. */
    ValueTuple8() = default;

    /** @brief Constructs a ValueTuple8 with the specified elements and rest. */
    ValueTuple8(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7, TRest rest)
        : Item1(std::move(i1)), Item2(std::move(i2)), Item3(std::move(i3)),
          Item4(std::move(i4)), Item5(std::move(i5)), Item6(std::move(i6)),
          Item7(std::move(i7)), Rest(std::move(rest)) {}

    /** @brief Determines whether this instance equals another. */
    [[nodiscard]] bool Equals(const ValueTuple8& o) const {
        return Item1==o.Item1 && Item2==o.Item2 && Item3==o.Item3 && Item4==o.Item4 &&
               Item5==o.Item5 && Item6==o.Item6 && Item7==o.Item7 && Rest.Equals(o.Rest);
    }

    /** @brief Returns a hash code for this instance. */
    [[nodiscard]] intcs GetHashCode() const noexcept {
        size_t h = detail::vtHash(Item1);
        h = detail::vtHashCombine(h, detail::vtHash(Item2));
        h = detail::vtHashCombine(h, detail::vtHash(Item3));
        h = detail::vtHashCombine(h, detail::vtHash(Item4));
        h = detail::vtHashCombine(h, detail::vtHash(Item5));
        h = detail::vtHashCombine(h, detail::vtHash(Item6));
        h = detail::vtHashCombine(h, detail::vtHash(Item7));
        h = detail::vtHashCombine(h, static_cast<size_t>(Rest.GetHashCode()));
        return static_cast<intcs>(h);
    }

    /**
     * @brief Compares this instance to another ValueTuple8 lexicographically.
     * @return -1 if less, 0 if equal, 1 if greater.
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple8& o) const {
        if (Item1 < o.Item1) return -1;
        if (o.Item1 < Item1) return 1;
        if (Item2 < o.Item2) return -1;
        if (o.Item2 < Item2) return 1;
        if (Item3 < o.Item3) return -1;
        if (o.Item3 < Item3) return 1;
        if (Item4 < o.Item4) return -1;
        if (o.Item4 < Item4) return 1;
        if (Item5 < o.Item5) return -1;
        if (o.Item5 < Item5) return 1;
        if (Item6 < o.Item6) return -1;
        if (o.Item6 < Item6) return 1;
        if (Item7 < o.Item7) return -1;
        if (o.Item7 < Item7) return 1;
        return Rest.CompareTo(o.Rest);
    }

    /**
     * @brief Returns a string that represents the value of this instance.
     *
     * Flattens Rest inline: "(i1, i2, ..., i7, rest_i1, rest_i2, ...)".
     */
    [[nodiscard]] std::string ToString() const {
        std::string restStr = Rest.ToString();
        // Rest.ToString() yields "(x)" or "(x, y, ...)" — strip outer parens
        std::string restInner = restStr.size() >= 2
            ? restStr.substr(1, restStr.size() - 2) : "";
        return "(" + detail::vtItemStr(Item1) + ", " + detail::vtItemStr(Item2) + ", " +
                     detail::vtItemStr(Item3) + ", " + detail::vtItemStr(Item4) + ", " +
                     detail::vtItemStr(Item5) + ", " + detail::vtItemStr(Item6) + ", " +
                     detail::vtItemStr(Item7) + ", " + restInner + ")";
    }

    bool operator==(const ValueTuple8& o) const { return Equals(o); }
    bool operator!=(const ValueTuple8& o) const { return !Equals(o); }
    bool operator< (const ValueTuple8& o) const { return CompareTo(o) < 0; }
    bool operator<=(const ValueTuple8& o) const { return CompareTo(o) <= 0; }
    bool operator> (const ValueTuple8& o) const { return CompareTo(o) > 0; }
    bool operator>=(const ValueTuple8& o) const { return CompareTo(o) >= 0; }
};

// ===========================================================================
// MakeValueTuple for 8 elements — wraps item8 in ValueTuple1
// ===========================================================================

/**
 * @brief Creates a ValueTuple8 containing the specified values.
 *
 * C++ counterpart of .NET ValueTuple.Create(T1..T8).
 * Wraps @p i8 in a ValueTuple1 as the Rest field, matching .NET convention.
 */
template<typename T1, typename T2, typename T3, typename T4,
         typename T5, typename T6, typename T7, typename T8>
ValueTuple8<T1,T2,T3,T4,T5,T6,T7,ValueTuple1<T8>>
MakeValueTuple(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7, T8 i8) {
    return {std::move(i1), std::move(i2), std::move(i3), std::move(i4),
            std::move(i5), std::move(i6), std::move(i7),
            ValueTuple1<T8>(std::move(i8))};
}

// ===========================================================================
// ValueTuple  —  System.ValueTuple (zero-element, non-generic)
// Defined last so its Create() methods can reference ValueTuple1..8.
// ===========================================================================

/**
 * @brief Represents a zero-element value tuple.
 *
 * C++ counterpart of .NET System.ValueTuple (non-generic struct).
 * Also provides static Create() factory methods for all arities 0–8,
 * mirroring .NET ValueTuple.Create(T1, T2, ...) overloads.
 */
struct ValueTuple {
    /** @brief Returns true; all zero-element tuples are equal. */
    [[nodiscard]] bool Equals(const ValueTuple&) const noexcept { return true; }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET ValueTuple.GetHashCode() — always 0.
     */
    [[nodiscard]] intcs GetHashCode() const noexcept { return 0; }

    /**
     * @brief Compares this instance to another ValueTuple — always 0.
     *
     * C++ counterpart of .NET ValueTuple.CompareTo(ValueTuple).
     */
    [[nodiscard]] intcs CompareTo(const ValueTuple&) const noexcept { return 0; }

    /**
     * @brief Returns a string that represents this instance.
     *
     * C++ counterpart of .NET ValueTuple.ToString() — always "()".
     */
    [[nodiscard]] std::string ToString() const { return "()"; }

    bool operator==(const ValueTuple&) const noexcept { return true; }
    bool operator!=(const ValueTuple&) const noexcept { return false; }
    bool operator< (const ValueTuple&) const noexcept { return false; }
    bool operator<=(const ValueTuple&) const noexcept { return true; }
    bool operator> (const ValueTuple&) const noexcept { return false; }
    bool operator>=(const ValueTuple&) const noexcept { return true; }

    // -----------------------------------------------------------------------
    // Static Create() factory methods  —  System.ValueTuple.Create(...)
    // -----------------------------------------------------------------------

    /** @brief Creates a zero-element ValueTuple. */
    static ValueTuple Create() noexcept { return {}; }

    /** @brief Creates a 1-tuple (singleton). */
    template<typename T1>
    static ValueTuple1<T1> Create(T1 item1) {
        return ValueTuple1<T1>(std::move(item1));
    }

    /** @brief Creates a 2-tuple (pair). */
    template<typename T1, typename T2>
    static ValueTuple2<T1,T2> Create(T1 item1, T2 item2) {
        return {std::move(item1), std::move(item2)};
    }

    /** @brief Creates a 3-tuple (triple). */
    template<typename T1, typename T2, typename T3>
    static ValueTuple3<T1,T2,T3> Create(T1 item1, T2 item2, T3 item3) {
        return {std::move(item1), std::move(item2), std::move(item3)};
    }

    /** @brief Creates a 4-tuple (quadruple). */
    template<typename T1, typename T2, typename T3, typename T4>
    static ValueTuple4<T1,T2,T3,T4> Create(T1 item1, T2 item2, T3 item3, T4 item4) {
        return {std::move(item1), std::move(item2), std::move(item3), std::move(item4)};
    }

    /** @brief Creates a 5-tuple (quintuple). */
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    static ValueTuple5<T1,T2,T3,T4,T5> Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5) {
        return {std::move(i1), std::move(i2), std::move(i3), std::move(i4), std::move(i5)};
    }

    /** @brief Creates a 6-tuple (sextuple). */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    static ValueTuple6<T1,T2,T3,T4,T5,T6> Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6) {
        return {std::move(i1), std::move(i2), std::move(i3),
                std::move(i4), std::move(i5), std::move(i6)};
    }

    /** @brief Creates a 7-tuple (septuple). */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
    static ValueTuple7<T1,T2,T3,T4,T5,T6,T7> Create(T1 i1, T2 i2, T3 i3, T4 i4,
                                                       T5 i5, T6 i6, T7 i7) {
        return {std::move(i1), std::move(i2), std::move(i3), std::move(i4),
                std::move(i5), std::move(i6), std::move(i7)};
    }

    /**
     * @brief Creates an 8-tuple (octuple).
     *
     * C++ counterpart of .NET ValueTuple.Create<T1..T8>(T1..T8).
     * Wraps item8 in a ValueTuple1 as the Rest field, matching .NET convention.
     */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
    static ValueTuple8<T1,T2,T3,T4,T5,T6,T7,ValueTuple1<T8>>
    Create(T1 i1, T2 i2, T3 i3, T4 i4, T5 i5, T6 i6, T7 i7, T8 i8) {
        return {std::move(i1), std::move(i2), std::move(i3), std::move(i4),
                std::move(i5), std::move(i6), std::move(i7),
                ValueTuple1<T8>(std::move(i8))};
    }
};

} // namespace System
