// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Mime {

    /**
     * @brief Specifies the media (MIME) type of data used by another class.
     *
     * C++ counterpart of .NET System.Net.Mime.MediaTypeNames.
     */
    struct MediaTypeNames {
        MediaTypeNames() = delete;

        /** @brief Specifies the kind of application data in a message attachment. */
        struct Application {
            Application() = delete;
            static constexpr const char* FormUrlEncoded = "application/x-www-form-urlencoded";
            static constexpr const char* GZip = "application/gzip";
            static constexpr const char* Json = "application/json";
            static constexpr const char* JsonPatch = "application/json-patch+json";
            static constexpr const char* JsonSequence = "application/json-seq";
            static constexpr const char* Manifest = "application/manifest+json";
            static constexpr const char* Octet = "application/octet-stream";
            static constexpr const char* Pdf = "application/pdf";
            static constexpr const char* ProblemJson = "application/problem+json";
            static constexpr const char* ProblemXml = "application/problem+xml";
            static constexpr const char* Rtf = "application/rtf";
            static constexpr const char* Soap = "application/soap+xml";
            static constexpr const char* Wasm = "application/wasm";
            static constexpr const char* Xml = "application/xml";
            static constexpr const char* XmlDtd = "application/xml-dtd";
            static constexpr const char* XmlPatch = "application/xml-patch+xml";
            static constexpr const char* Yaml = "application/yaml";
            static constexpr const char* Zip = "application/zip";
        };

        /** @brief Specifies the kind of font data in a message attachment. */
        struct Font {
            Font() = delete;
            static constexpr const char* Collection = "font/collection";
            static constexpr const char* Otf = "font/otf";
            static constexpr const char* Sfnt = "font/sfnt";
            static constexpr const char* Ttf = "font/ttf";
            static constexpr const char* Woff = "font/woff";
            static constexpr const char* Woff2 = "font/woff2";
        };

        /** @brief Specifies the kind of image data in a message attachment. */
        struct Image {
            Image() = delete;
            static constexpr const char* Avif = "image/avif";
            static constexpr const char* Bmp = "image/bmp";
            static constexpr const char* Gif = "image/gif";
            static constexpr const char* Icon = "image/x-icon";
            static constexpr const char* Jpeg = "image/jpeg";
            static constexpr const char* Png = "image/png";
            static constexpr const char* Svg = "image/svg+xml";
            static constexpr const char* Tiff = "image/tiff";
            static constexpr const char* Webp = "image/webp";
        };

        /** @brief Specifies the kind of multipart data in a message attachment. */
        struct Multipart {
            Multipart() = delete;
            static constexpr const char* ByteRanges = "multipart/byteranges";
            static constexpr const char* FormData = "multipart/form-data";
            static constexpr const char* Mixed = "multipart/mixed";
            static constexpr const char* Related = "multipart/related";
        };

        /** @brief Specifies the kind of text data in a message attachment. */
        struct Text {
            Text() = delete;
            static constexpr const char* Css = "text/css";
            static constexpr const char* Csv = "text/csv";
            static constexpr const char* EventStream = "text/event-stream";
            static constexpr const char* Html = "text/html";
            static constexpr const char* JavaScript = "text/javascript";
            static constexpr const char* Markdown = "text/markdown";
            static constexpr const char* Plain = "text/plain";
            static constexpr const char* RichText = "text/richtext";
            static constexpr const char* Rtf = "text/rtf";
            static constexpr const char* Xml = "text/xml";
        };

        /** @brief Specifies the kind of video data in a message attachment. */
        struct Video {
            Video() = delete;
            static constexpr const char* Mp4 = "video/mp4";
            static constexpr const char* Mpeg = "video/mpeg";
            static constexpr const char* Ogg = "video/ogg";
            static constexpr const char* QuickTime = "video/quicktime";
            static constexpr const char* WebM = "video/webm";
        };
    };

} // namespace System::Net::Mime
