// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XDocumentType.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XDocumentType::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteDocType(name_, publicId_, systemId_, internalSubset_);
    }

    void XDocumentType::SerializeTo(std::ostream& os, int depth, bool indent) const {
        if (indent) os << std::string(static_cast<size_t>(depth) * 2, ' ');
        os << "<!DOCTYPE " << name_;
        if (!publicId_.empty()) {
            os << " PUBLIC \"" << publicId_ << "\" \"" << systemId_ << "\"";
        } else if (!systemId_.empty()) {
            os << " SYSTEM \"" << systemId_ << "\"";
        }
        if (!internalSubset_.empty()) os << " [" << internalSubset_ << "]";
        os << ">";
    }

} // namespace System::Xml::Linq
