// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Json/HttpClientJsonExtensions.hpp"
#include "System/Net/Sockets/TcpClient.hpp"
#include "System/Net/IPEndPoint.hpp"
#include <thread>

using namespace System::Net::Http;
using namespace System::Net::Http::Json;
using System::Net::IPAddress;
using System::Net::IPEndPoint;
using System::Net::Sockets::TcpListener;

namespace {
    // Minimal HTTP/1.1 test server: accepts one connection, drains the request, and replies
    // with a canned response. Good enough to exercise HttpClientJsonExtensions end-to-end
    // without a full HTTP server implementation.
    void serveOnce(TcpListener& listener, const std::string& responseBody, const std::string& status = "200 OK") {
        auto client = listener.AcceptTcpClient();
        auto stream = client.GetStream();

        SharpRuntime::bytecs buf[4096];
        std::string received;
        while (received.find("\r\n\r\n") == std::string::npos) {
            SharpRuntime::intcs n = stream->Read(buf, 0, 4096);
            if (n <= 0) break;
            received.append(reinterpret_cast<char*>(buf), static_cast<size_t>(n));
        }

        std::string response = "HTTP/1.1 " + status + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
            "Connection: close\r\n\r\n" + responseBody;
        stream->Write(reinterpret_cast<const SharpRuntime::bytecs*>(response.data()), 0, static_cast<SharpRuntime::intcs>(response.size()));
        client.Close();
    }
}

TEST(HttpClientJsonExtensionsTests, GetFromJsonAsync_ParsesResponseBody) {
    TcpListener listener(IPEndPoint(IPAddress::Loopback, 0));
    listener.Start();
    int port = listener.getLocalEndpointProperty().getPortProperty();

    std::thread server([&]() { serveOnce(listener, "{\"greeting\":\"hello\"}"); });

    HttpClient client;
    auto task = HttpClientJsonExtensions::GetFromJsonAsync(client, "http://127.0.0.1:" + std::to_string(port) + "/data");
    auto doc = task.getResultProperty();
    server.join();
    listener.Stop();

    ASSERT_TRUE(doc != nullptr);
    const auto& root = doc->getRootElementProperty();
    EXPECT_EQ(root.GetProperty("greeting").GetString(), "hello");
}

TEST(HttpClientJsonExtensionsTests, PostAsJsonAsync_SendsSerializedBody) {
    TcpListener listener(IPEndPoint(IPAddress::Loopback, 0));
    listener.Start();
    int port = listener.getLocalEndpointProperty().getPortProperty();

    std::string capturedBody;
    std::thread server([&]() {
        auto client = listener.AcceptTcpClient();
        auto stream = client.GetStream();
        SharpRuntime::bytecs buf[4096];
        std::string received;
        for (;;) {
            SharpRuntime::intcs n = stream->Read(buf, 0, 4096);
            if (n <= 0) break;
            received.append(reinterpret_cast<char*>(buf), static_cast<size_t>(n));
            if (received.find("\r\n\r\n") != std::string::npos) {
                size_t headerEnd = received.find("\r\n\r\n") + 4;
                size_t clPos = received.find("Content-Length:");
                if (clPos != std::string::npos) {
                    size_t clEnd = received.find("\r\n", clPos);
                    int len = std::stoi(received.substr(clPos + 16, clEnd - clPos - 16));
                    while (received.size() - headerEnd < static_cast<size_t>(len)) {
                        n = stream->Read(buf, 0, 4096);
                        if (n <= 0) break;
                        received.append(reinterpret_cast<char*>(buf), static_cast<size_t>(n));
                    }
                }
                capturedBody = received.substr(headerEnd);
                break;
            }
        }
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        stream->Write(reinterpret_cast<const SharpRuntime::bytecs*>(response.data()), 0, static_cast<SharpRuntime::intcs>(response.size()));
        client.Close();
    });

    HttpClient client;
    nlohmann::json payload;
    payload["id"] = 7;
    auto task = HttpClientJsonExtensions::PostAsJsonAsync(client, "http://127.0.0.1:" + std::to_string(port) + "/items", payload);
    auto response = task.getResultProperty();
    server.join();
    listener.Stop();

    ASSERT_TRUE(response != nullptr);
    EXPECT_EQ(response->getStatusCodeProperty(), System::Net::HttpStatusCode::OK);
    auto parsed = nlohmann::json::parse(capturedBody);
    EXPECT_EQ(parsed["id"], 7);
}
