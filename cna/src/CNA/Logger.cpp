#include "CNA/Logger.hpp"

#include <SDL2/SDL.h>

#include <string>

namespace CNA
{
#ifdef NDEBUG
    static constexpr LogLevel defaultLevel = LogLevel::INFO;
#else
    static constexpr LogLevel defaultLevel = LogLevel::TRACE;
#endif

    LogLevel Logger::minimumLevel_ = defaultLevel;

    void Logger::Log(
        LogLevel level,
        const std::string& message,
        LogCategory category,
        bool condition
    )
    {
        if (!condition)
        {
            return;
        }
        if (!IsEnabled(level))
        {
            return;
        }

        const int sdlCategory = ToSDLCategory(category);
        const SDL_LogPriority sdlPriority = static_cast<SDL_LogPriority>(ToSDLPriority(level));

        std::string finalMessage =
            std::string("[")
            + ToString(level)
            + "]["
            + ToString(category)
            + "] "
            + std::string(message);

        SDL_LogMessage(sdlCategory, sdlPriority, "%s", finalMessage.c_str());
    }

    void Logger::Fatal(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::FATAL, message, category);
    }

    void Logger::Error(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::ERROR, message, category);
    }

    void Logger::Warn(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::WARN, message, category);
    }

    void Logger::Info(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::INFO, message, category);
    }

    void Logger::Debug(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::DEBUG, message, category);
    }

    void Logger::Trace(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::TRACE, message, category);
    }

    void Logger::Experiment(
        const std::string& message,
        LogCategory category
    )
    {
        Log(LogLevel::EXPERIMENT, message, category);
    }

    void Logger::FatalIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::FATAL, message, LogCategory::APPLICATION, condition);
    }

    void Logger::ErrorIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::ERROR, message, LogCategory::APPLICATION, condition);
    }

    void Logger::WarnIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::WARN, message, LogCategory::APPLICATION, condition);
    }

    void Logger::InfoIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::INFO, message, LogCategory::APPLICATION, condition);
    }

    void Logger::DebugIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::DEBUG, message, LogCategory::APPLICATION, condition);
    }

    void Logger::TraceIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::TRACE, message, LogCategory::APPLICATION, condition);
    }

    void Logger::ExperimentIf(
        const std::string& message,
        bool condition
    )
    {
        Log(LogLevel::EXPERIMENT, message, LogCategory::APPLICATION, condition);
    }

    void Logger::SetMinimumLevel(LogLevel level)
    {
        minimumLevel_ = level;
        // SDL2's equivalent of SDL3's SDL_SetLogPriorities (set every category's priority) is
        // SDL_LogSetAllPriority.
        SDL_LogSetAllPriority(static_cast<SDL_LogPriority>(ToSDLPriority(level)));
    }

    LogLevel Logger::GetMinimumLevel()
    {
        return minimumLevel_;
    }

    int Logger::ToSDLPriority(LogLevel level)
    {
        switch (level)
        {
        // case LogLevel::FATAL:
        //     return SDL_LOG_PRIORITY_CRITICAL;
        // case LogLevel::ERROR:
        //     return SDL_LOG_PRIORITY_ERROR;
        // case LogLevel::WARN:
        //     return SDL_LOG_PRIORITY_WARN;
        // case LogLevel::INFO:
        //     return SDL_LOG_PRIORITY_INFO;

        //todo
        case LogLevel::DEBUG:
        case LogLevel::TRACE:
        case LogLevel::EXPERIMENT:
            return SDL_LOG_PRIORITY_DEBUG;
        default:
            return SDL_LOG_PRIORITY_INFO;
        }
    }

    int Logger::ToSDLCategory(LogCategory category)
    {
        switch (category)
        {
        case LogCategory::APPLICATION:
            return SDL_LOG_CATEGORY_APPLICATION;
        case LogCategory::ERROR:
            return SDL_LOG_CATEGORY_ERROR;
        case LogCategory::SYSTEM:
            return SDL_LOG_CATEGORY_SYSTEM;
        case LogCategory::AUDIO:
            return SDL_LOG_CATEGORY_AUDIO;
        case LogCategory::VIDEO:
            return SDL_LOG_CATEGORY_VIDEO;
        case LogCategory::RENDER:
            return SDL_LOG_CATEGORY_RENDER;
        case LogCategory::INPUT:
            return SDL_LOG_CATEGORY_INPUT;
        case LogCategory::TEST:
            return SDL_LOG_CATEGORY_TEST;
        case LogCategory::GPU:
            // SDL2 has no dedicated GPU log category (SDL3-only addition) -- RENDER is the
            // closest existing category.
            return SDL_LOG_CATEGORY_RENDER;
        default:
            return SDL_LOG_CATEGORY_APPLICATION;
        }
    }

    bool Logger::IsEnabled(LogLevel level)
    {
        return static_cast<int>(level) <= static_cast<int>(minimumLevel_);
    }

    const char* Logger::ToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::FATAL:
            return "FATAL";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::TRACE:
            return "TRACE";
        case LogLevel::EXPERIMENT:
            return "EXPERIMENT";
        default:
            return "UNKNOWN";
        }
    }

    const char* Logger::ToString(LogCategory category)
    {
        switch (category)
        {
        case LogCategory::APPLICATION:
            return "APPLICATION";
        case LogCategory::ERROR:
            return "ERROR";
        case LogCategory::SYSTEM:
            return "SYSTEM";
        case LogCategory::AUDIO:
            return "AUDIO";
        case LogCategory::VIDEO:
            return "VIDEO";
        case LogCategory::RENDER:
            return "RENDER";
        case LogCategory::INPUT:
            return "INPUT";
        case LogCategory::TEST:
            return "TEST";
        case LogCategory::GPU:
            return "GPU";
        default:
            return "UNKNOWN";
        }
    }
}
