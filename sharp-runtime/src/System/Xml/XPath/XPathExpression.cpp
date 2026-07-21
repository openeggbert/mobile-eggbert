// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XPathExpression.hpp"

#include "XPathAstInternal.hpp"

namespace System::Xml::XPath {

    XPathExpression::XPathExpression(std::string expression, std::shared_ptr<Internal::AstNode> ast)
        : expression_(std::move(expression)), ast_(std::move(ast)) {}

    XPathExpression::~XPathExpression() = default;

    XPathExpression XPathExpression::Compile(const std::string& xpath) {
        std::shared_ptr<Internal::AstNode> ast = Internal::ParseXPath(xpath);
        return XPathExpression(xpath, std::move(ast));
    }

    XPathResultType XPathExpression::getReturnTypeProperty() const {
        return ast_->InferReturnType();
    }

    void XPathExpression::AddSort(const std::string& sortKeyXPath, XmlSortOrder order, XmlCaseOrder caseOrder,
                                   const std::string& lang, XmlDataType dataType) {
        sortKeys_.push_back(SortKey{sortKeyXPath, order, caseOrder, lang, dataType});
    }

    XPathExpression XPathExpression::Clone() const {
        return *this;
    }

} // namespace System::Xml::XPath
