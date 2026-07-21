// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "System/Exception.hpp"

namespace System {

/**
 * @brief Represents one or more errors that occur during application execution.
 *
 * C++ counterpart of .NET System.AggregateException.
 * Stores a collection of inner exceptions and provides Flatten() and Handle() helpers.
 */
class AggregateException : public Exception {
    std::vector<std::exception_ptr> innerExceptions_;

    static std::string buildMessage(const std::vector<std::exception_ptr>& exs) {
        if (exs.empty()) return "One or more errors occurred.";
        std::string m = "One or more errors occurred. (";
        bool first = true;
        for (auto& ep : exs) {
            try { std::rethrow_exception(ep); }
            catch (const std::exception& e) {
                if (!first) m += ") (";
                m += e.what();
                first = false;
            } catch (...) {
                if (!first) m += ") (";
                m += "unknown error";
                first = false;
            }
        }
        m += ")";
        return m;
    }

public:
    /** @brief Initializes a new instance with the default aggregate error message. */
    AggregateException() : Exception("One or more errors occurred.") {}

    /** @brief Initializes a new instance with the specified error message. */
    explicit AggregateException(const std::string& message) : Exception(message) {}

    /** @brief Initializes a new instance with a collection of inner exceptions. */
    explicit AggregateException(std::vector<std::exception_ptr> innerExceptions)
        : Exception(buildMessage(innerExceptions)),
          innerExceptions_(std::move(innerExceptions)) {}

    /** @brief Initializes a new instance with an initializer list of inner exceptions. */
    AggregateException(std::initializer_list<std::exception_ptr> innerExceptions)
        : AggregateException(std::vector<std::exception_ptr>(innerExceptions)) {}

    /** @brief Initializes a new instance with a message and a collection of inner exceptions. */
    AggregateException(const std::string& message, std::vector<std::exception_ptr> innerExceptions)
        : Exception(message), innerExceptions_(std::move(innerExceptions)) {}

    /** @brief Initializes a new instance with a message and a single inner exception. */
    AggregateException(const std::string& message, std::exception_ptr innerException)
        : Exception(message), innerExceptions_({innerException}) {}

    /**
     * @brief Gets the read-only collection of inner exceptions that caused this aggregate exception.
     *
     * C++ counterpart of .NET AggregateException.InnerExceptions.
     */
    [[nodiscard]] const std::vector<std::exception_ptr>& getInnerExceptionsProperty() const {
        return innerExceptions_;
    }

    /**
     * @brief Returns the number of inner exceptions contained in this aggregate exception.
     *
     * C++ extension — convenience wrapper over InnerExceptions.size().
     */
    [[nodiscard]] std::size_t getInnerExceptionCountProperty() const {
        return innerExceptions_.size();
    }

    /**
     * @brief Returns the first inner exception, or this exception if there are none or more than one.
     *
     * C++ counterpart of .NET AggregateException.GetBaseException().
     */
    [[nodiscard]] std::exception_ptr GetBaseException() const {
        if (innerExceptions_.size() == 1) {
            try {
                std::rethrow_exception(innerExceptions_[0]);
            } catch (const AggregateException& ae) {
                return ae.GetBaseException();
            } catch (...) {}
            return innerExceptions_[0];
        }
        return std::make_exception_ptr(*this);
    }

    /**
     * @brief Returns the single inner exception if exactly one is present, otherwise returns this exception.
     *
     * C++ extension — convenience wrapper around GetBaseException() for the common single-exception case.
     */
    [[nodiscard]] std::exception_ptr Unwrap() const {
        if (innerExceptions_.size() == 1) return innerExceptions_[0];
        return std::make_exception_ptr(*this);
    }

    /**
     * @brief Flattens nested AggregateExceptions into a single flat AggregateException.
     *
     * C++ counterpart of .NET AggregateException.Flatten().
     */
    [[nodiscard]] AggregateException Flatten() const {
        std::vector<std::exception_ptr> flat;
        collectLeaves(innerExceptions_, flat);
        return AggregateException(std::move(flat));
    }

    /**
     * @brief Invokes a handler on each inner exception; rethrows unhandled ones as a new AggregateException.
     *
     * C++ counterpart of .NET AggregateException.Handle(Func&lt;Exception,bool&gt;).
     * @param predicate Handler returning true if the exception is handled.
     */
    void Handle(std::function<bool(std::exception_ptr)> predicate) const {
        std::vector<std::exception_ptr> unhandled;
        for (auto& ep : innerExceptions_) {
            if (!predicate(ep)) unhandled.push_back(ep);
        }
        if (!unhandled.empty()) throw AggregateException(std::move(unhandled));
    }

private:
    static void collectLeaves(const std::vector<std::exception_ptr>& exs,
                              std::vector<std::exception_ptr>& result) {
        for (auto& ep : exs) {
            try {
                std::rethrow_exception(ep);
            } catch (const AggregateException& ae) {
                collectLeaves(ae.innerExceptions_, result);
            } catch (...) {
                result.push_back(ep);
            }
        }
    }
};

} // namespace System
