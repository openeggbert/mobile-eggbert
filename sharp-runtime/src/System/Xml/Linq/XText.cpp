// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XText.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XText::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteString(text_);
    }

    void XText::SerializeTo(std::ostream& os, int /*depth*/, bool /*indent*/) const {
        os << EscapeText(text_);
    }

    bool XText::DeepEqualsCore(const XNode& other) const {
        return text_ == static_cast<const XText&>(other).text_;
    }

} // namespace System::Xml::Linq
