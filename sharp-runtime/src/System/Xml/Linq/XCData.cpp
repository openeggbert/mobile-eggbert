// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XCData.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XCData::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteCData(getValueProperty());
    }

    void XCData::SerializeTo(std::ostream& os, int /*depth*/, bool /*indent*/) const {
        os << "<![CDATA[" << getValueProperty() << "]]>";
    }

} // namespace System::Xml::Linq
