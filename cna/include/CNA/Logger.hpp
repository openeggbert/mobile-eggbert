#pragma once

#include <string>

#include "CNA/LogLevel.hpp"
#include "CNA/LogCategory.hpp"

namespace CNA
{
    /**
     * @brief Simple logging utility built on top of SDL logging.
     *
     * Provides convenience methods for logging messages with severity
     * and category metadata.
     */
    class Logger
    {
    public:
        /**
         * @brief Logs a message with an explicit level.
         *
         * @param level Severity level.
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Log(
            LogLevel level,
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION,
            bool condition = true
        );

        /**
         * @brief Logs a fatal message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Fatal(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs an error message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Error(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs a warning message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Warn(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs an informational message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Info(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs a debug message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Debug(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs a trace message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Trace(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );

        /**
         * @brief Logs an experimental message.
         *
         * @param message Message text.
         * @param category Functional category. Defaults to APPLICATION.
         */
        static void Experiment(
            const std::string& message,
            LogCategory category = LogCategory::APPLICATION
        );
        /**
         * @brief Logs a fatal message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void FatalIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs an error message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void ErrorIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs a warning message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void WarnIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs an informational message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void InfoIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs a debug message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void DebugIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs a trace message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void TraceIf(
            const std::string& message,
            bool condition
        );

        /**
         * @brief Logs an experimental message if the condition is true.
         *
         * @param message Message text.
         * @param condition Condition that must be true for the message to be logged.
         */
        static void ExperimentIf(
            const std::string& message,
            bool condition
        );
        /**
         * @brief Sets the minimum enabled log level.
         *
         * Messages below this threshold are ignored.
         *
         * @param level New minimum log level.
         */
        static void SetMinimumLevel(LogLevel level);

        /**
         * @brief Gets the current minimum enabled log level.
         *
         * @return Current minimum log level.
         */
        [[nodiscard]] static LogLevel GetMinimumLevel();

    private:
        [[nodiscard]] static int ToSDLPriority(LogLevel level);
        [[nodiscard]] static int ToSDLCategory(LogCategory category);
        [[nodiscard]] static bool IsEnabled(LogLevel level);
        [[nodiscard]] static const char* ToString(LogLevel level);
        [[nodiscard]] static const char* ToString(LogCategory category);

    private:
        static LogLevel minimumLevel_;
    };
}
