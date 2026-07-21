// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XComment.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XComment::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteComment(value_);
    }

    void XComment::SerializeTo(std::ostream& os, int depth, bool indent) const {
        if (indent) os << std::string(static_cast<size_t>(depth) * 2, ' ');
        os << "<!--" << value_ << "-->";
    }

} // namespace System::Xml::Linq
