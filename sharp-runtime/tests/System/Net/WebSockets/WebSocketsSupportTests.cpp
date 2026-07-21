// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/WebSockets/ValueWebSocketReceiveResult.hpp"
#include "System/Net/WebSockets/WebSocket.hpp"
#include "System/Net/WebSockets/WebSocketCloseStatus.hpp"
#include "System/Net/WebSockets/WebSocketCreationOptions.hpp"
#include "System/Net/WebSockets/WebSocketError.hpp"
#include "System/Net/WebSockets/WebSocketException.hpp"
#include "System/Net/WebSockets/WebSocketMessageFlags.hpp"
#include "System/Net/WebSockets/WebSocketMessageType.hpp"
#include "System/Net/WebSockets/WebSocketReceiveResult.hpp"
#include "System/Net/WebSockets/WebSocketState.hpp"

using namespace System::Net::WebSockets;

TEST(WebSocketStateTests, Values) {
    EXPECT_EQ(static_cast<int>(WebSocketState::None), 0);
    EXPECT_EQ(static_cast<int>(WebSocketState::Open), 2);
    EXPECT_EQ(static_cast<int>(WebSocketState::Aborted), 6);
}

TEST(WebSocketMessageTypeTests, Values) {
    EXPECT_EQ(static_cast<int>(WebSocketMessageType::Text), 0);
    EXPECT_EQ(static_cast<int>(WebSocketMessageType::Binary), 1);
    EXPECT_EQ(static_cast<int>(WebSocketMessageType::Close), 2);
}

TEST(WebSocketCloseStatusTests, Values) {
    EXPECT_EQ(static_cast<int>(WebSocketCloseStatus::NormalClosure), 1000);
    EXPECT_EQ(static_cast<int>(WebSocketCloseStatus::ProtocolError), 1002);
}

TEST(WebSocketErrorTests, Values) {
    EXPECT_EQ(static_cast<int>(WebSocketError::Success), 0);
    EXPECT_EQ(static_cast<int>(WebSocketError::InvalidState), 9);
}

TEST(WebSocketMessageFlagsTests, BitwiseOperators) {
    auto combined = WebSocketMessageFlags::EndOfMessage | WebSocketMessageFlags::DisableCompression;
    EXPECT_EQ(static_cast<int>(combined), 3);
    EXPECT_EQ(combined & WebSocketMessageFlags::EndOfMessage, WebSocketMessageFlags::EndOfMessage);
}

TEST(ValueWebSocketReceiveResultTests, BasicAccessors) {
    ValueWebSocketReceiveResult result(10, WebSocketMessageType::Binary, true);
    EXPECT_EQ(result.getCountProperty(), 10);
    EXPECT_EQ(result.getMessageTypeProperty(), WebSocketMessageType::Binary);
    EXPECT_TRUE(result.getEndOfMessageProperty());
}

TEST(ValueWebSocketReceiveResultTests, NegativeCount_Throws) {
    EXPECT_THROW((ValueWebSocketReceiveResult(-1, WebSocketMessageType::Text, false)), System::ArgumentOutOfRangeException);
}

// Real .NET validates messageType against the same range it packs alongside count/endOfMessage:
// if ((uint)messageType > (uint)WebSocketMessageType.Close). The port previously only validated
// count, silently accepting an out-of-range messageType value.
TEST(ValueWebSocketReceiveResultTests, InvalidMessageType_Throws) {
    EXPECT_THROW((ValueWebSocketReceiveResult(0, static_cast<WebSocketMessageType>(99), false)),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW((ValueWebSocketReceiveResult(0, static_cast<WebSocketMessageType>(-1), false)),
                 System::ArgumentOutOfRangeException);
}

TEST(WebSocketReceiveResultTests, BasicAccessors) {
    WebSocketReceiveResult result(5, WebSocketMessageType::Text, true);
    EXPECT_EQ(result.getCountProperty(), 5);
    EXPECT_TRUE(result.getEndOfMessageProperty());
    EXPECT_EQ(result.getMessageTypeProperty(), WebSocketMessageType::Text);
    EXPECT_FALSE(result.getCloseStatusProperty().has_value());
}

TEST(WebSocketReceiveResultTests, WithCloseStatus) {
    WebSocketReceiveResult result(0, WebSocketMessageType::Close, true, WebSocketCloseStatus::NormalClosure,
                                   std::string("bye"));
    ASSERT_TRUE(result.getCloseStatusProperty().has_value());
    EXPECT_EQ(*result.getCloseStatusProperty(), WebSocketCloseStatus::NormalClosure);
    ASSERT_TRUE(result.getCloseStatusDescriptionProperty().has_value());
    EXPECT_EQ(*result.getCloseStatusDescriptionProperty(), "bye");
}

TEST(WebSocketCreationOptionsTests, Defaults) {
    WebSocketCreationOptions options;
    EXPECT_FALSE(options.getIsServerProperty());
    EXPECT_FALSE(options.getSubProtocolProperty().has_value());
}

TEST(WebSocketCreationOptionsTests, SetSubProtocol) {
    WebSocketCreationOptions options;
    options.setSubProtocolProperty(std::string("chat"));
    ASSERT_TRUE(options.getSubProtocolProperty().has_value());
    EXPECT_EQ(*options.getSubProtocolProperty(), "chat");
}

TEST(WebSocketExceptionTests, DefaultMessage) {
    WebSocketException ex(WebSocketError::NotAWebSocket);
    EXPECT_EQ(ex.getWebSocketErrorCodeProperty(), WebSocketError::NotAWebSocket);
}

TEST(WebSocketExceptionTests, CustomMessage) {
    WebSocketException ex(WebSocketError::HeaderError, "custom message");
    EXPECT_EQ(ex.getWebSocketErrorCodeProperty(), WebSocketError::HeaderError);
    EXPECT_EQ(ex.getMessageProperty(), "custom message");
}

TEST(WebSocketTests, CreateClientBuffer) {
    auto buffer = WebSocket::CreateClientBuffer(1024, 512);
    EXPECT_EQ(buffer.size(), 1024u);
}

TEST(WebSocketTests, CreateServerBuffer) {
    auto buffer = WebSocket::CreateServerBuffer(2048);
    EXPECT_EQ(buffer.size(), 2048u);
}

TEST(WebSocketTests, CreateClientBuffer_ZeroSize_Throws) {
    EXPECT_THROW(WebSocket::CreateClientBuffer(0, 10), System::ArgumentOutOfRangeException);
}

TEST(WebSocketTests, DefaultKeepAliveInterval) {
    EXPECT_GT(WebSocket::getDefaultKeepAliveIntervalProperty().getTicksProperty(), 0);
}
