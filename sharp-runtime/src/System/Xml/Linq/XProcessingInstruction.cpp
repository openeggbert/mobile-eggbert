// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XProcessingInstruction.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XProcessingInstruction::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteProcessingInstruction(target_, data_);
    }

    void XProcessingInstruction::SerializeTo(std::ostream& os, int depth, bool indent) const {
        if (indent) os << std::string(static_cast<size_t>(depth) * 2, ' ');
        os << "<?" << target_;
        if (!data_.empty()) os << " " << data_;
        os << "?>";
    }

} // namespace System::Xml::Linq
