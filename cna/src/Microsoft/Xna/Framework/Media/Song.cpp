// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Media/Song.hpp"

#include <experimental/filesystem>
#include <utility>

#include "System/IO/FileNotFoundException.hpp"

namespace Microsoft::Xna::Framework::Media
{
    Song::Song(std::string fileName, std::string name)
        : name_(std::move(name)),
          duration_(System::TimeSpan::Zero),
          playCount_(0),
          isDisposed_(false),
          handle_(std::move(fileName))
    {
        // FNA's ctor throws FileNotFoundException(fileName) directly (Song.cs); match the
        // established CNA-wide convention (SoundBank/WaveBank) of a descriptive message plus the
        // path via getFileNameProperty(), rather than a bare std::runtime_error(handle_).
        if (!std::experimental::filesystem::exists(handle_))
        {
            throw System::IO::FileNotFoundException(
                "Could not find file '" + handle_ + "'.", handle_);
        }
    }

    Song::Song(std::string fileName, std::string assetName, SharpRuntime::intcs durationMS)
        : Song(std::move(fileName), std::move(assetName))
    {
        duration_ = System::TimeSpan::FromMilliseconds(durationMS);
    }

    Song::~Song()
    {
        Dispose();
    }

    const std::string& Song::getNameProperty() const
    {
        return name_;
    }

    System::TimeSpan Song::getDurationProperty() const
    {
        return duration_;
    }

    void Song::setDurationProperty(System::TimeSpan value)
    {
        duration_ = value;
    }

    SharpRuntime::intcs Song::getPlayCountProperty() const
    {
        return playCount_;
    }

    void Song::setPlayCountProperty(SharpRuntime::intcs value)
    {
        playCount_ = value;
    }

    void Song::Dispose()
    {
        isDisposed_ = true;
    }

    const std::string& Song::getHandle() const
    {
        return handle_;
    }

    const std::string& Song::GetTypeName() const
    {
        static const std::string typeName = "Microsoft.Xna.Framework.Media.Song";
        return typeName;
    }
}
