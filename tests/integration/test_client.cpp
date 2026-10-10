// Standalone UDP test client for multiplayer integration testing.
//
// Deliberately outside the server project code: it speaks only the raw wire
// protocol (control byte + payload) and does not include any server headers.
//
// Usage:
//   test_client --port N [--host H] [--connect] [--send voice|position|none]
//               [--payload TEXT] [--repeat N] [--listen SECONDS]
//
// Output (stdout, one token per line):
//   CONNECTED <id>        after the server assigns an id
//   RECV VOICE <payload>  on a relayed voice packet
//   RECV POSITION <payload>
//   SENT <kind> <payload> after sending to the server

#include <asio/ip/udp.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

/// Control byte: client -> server, request to connect.
constexpr uint8_t kMsgConnect = 0xFF;

/// Control byte: server -> client, assigned id.
constexpr uint8_t kMsgAssignId = 0xFE;

/// Control byte: voice packet, relayed to other clients.
constexpr uint8_t kMsgVoice = 0x01;

/// Control byte: position packet, relayed to other clients.
constexpr uint8_t kMsgPosition = 0x02;

/// Default number of seconds spent listening after connecting.
constexpr int kDefaultListenSeconds = 3;

/// Seconds to wait for the ASSIGN_ID reply before proceeding.
constexpr int kAssignTimeoutSeconds = 1;

/// Default number of times a message is sent (UDP is lossy).
constexpr int kDefaultRepeat = 3;

/// Pause between repeated sends, in milliseconds.
constexpr int kSendGapMs = 200;

/// Maximum datagram size we are willing to receive.
constexpr std::size_t kMaxDatagram = 2048;

using asio::ip::udp;

/// Fully resolved command-line options for the test client.
struct ClientOptions {
    std::string host = "127.0.0.1";
    uint16_t port = 0;
    bool connect = true;
    std::string send = "none";
    std::string payload = "ping";
    int repeat = kDefaultRepeat;
    int listen_seconds = kDefaultListenSeconds;
};

/// Parses argv into ClientOptions; exits with usage on a missing --port.
ClientOptions parse_args(int argc, char* argv[]) {
    ClientOptions opts;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        const bool has_value = (i + 1 < argc);
        if (arg == "--host" && has_value) {
            opts.host = argv[++i];
        } else if (arg == "--port" && has_value) {
            opts.port = static_cast<uint16_t>(std::atoi(argv[++i]));
        } else if (arg == "--send" && has_value) {
            opts.send = argv[++i];
        } else if (arg == "--payload" && has_value) {
            opts.payload = argv[++i];
        } else if (arg == "--repeat" && has_value) {
            opts.repeat = std::atoi(argv[++i]);
        } else if (arg == "--listen" && has_value) {
            opts.listen_seconds = std::atoi(argv[++i]);
        } else if (arg == "--no-connect") {
            opts.connect = false;
        }
    }
    if (opts.port == 0) {
        std::cerr << "test_client: --port is required\n";
        std::exit(2);
    }
    return opts;
}

/// Maps a --send value to its control byte (0 when nothing is sent).
uint8_t send_kind_byte(const std::string& kind) {
    if (kind == "voice") {
        return kMsgVoice;
    }
    if (kind == "position") {
        return kMsgPosition;
    }
    return 0;
}

/// Sends one datagram to the server endpoint.
void send_datagram(udp::socket& sock, const udp::endpoint& server, std::vector<uint8_t> bytes) {
    asio::error_code ec;
    sock.send_to(asio::buffer(bytes), server, 0, ec);
}

/// Builds a payload datagram: control byte followed by the payload bytes.
std::vector<uint8_t> make_payload_datagram(uint8_t kind, const std::string& payload) {
    std::vector<uint8_t> bytes;
    bytes.reserve(1 + payload.size());
    bytes.push_back(kind);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    return bytes;
}

/// Prints the assigned id carried by an ASSIGN_ID datagram.
void report_assigned_id(const std::vector<uint8_t>& buf, std::size_t n) {
    const std::string id(buf.begin() + 1, buf.begin() + n);
    std::cout << "CONNECTED " << id << '\n';
}

/// Prints a relayed packet (VOICE / POSITION).
void report_relay(const std::vector<uint8_t>& buf, std::size_t n) {
    const std::string payload(buf.begin() + 1, buf.begin() + n);
    if (buf[0] == kMsgVoice) {
        std::cout << "RECV VOICE " << payload << '\n';
    } else if (buf[0] == kMsgPosition) {
        std::cout << "RECV POSITION " << payload << '\n';
    }
    std::cout.flush();
}

/// Dispatches a received datagram.
void handle_datagram(const std::vector<uint8_t>& buf, std::size_t n) {
    if (n == 0) {
        return;
    }
    if (buf[0] == kMsgAssignId) {
        report_assigned_id(buf, n);
    } else if (buf[0] == kMsgVoice || buf[0] == kMsgPosition) {
        report_relay(buf, n);
    }
}

/// Receives and prints datagrams until the deadline elapses.
void receive_until(udp::socket& sock, std::chrono::steady_clock::time_point deadline) {
    std::vector<uint8_t> buf(kMaxDatagram);
    while (std::chrono::steady_clock::now() < deadline) {
        asio::error_code ec;
        const std::size_t n = sock.receive(asio::buffer(buf), 0, ec);
        if (ec == asio::error::would_block) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }
        if (ec) {
            break;
        }
        handle_datagram(buf, n);
    }
}

/// Waits for the ASSIGN_ID reply, printing CONNECTED <id>. Returns whether the
/// assignment arrived before the deadline. Other datagrams are ignored.
bool wait_for_assignment(udp::socket& sock, std::chrono::steady_clock::time_point deadline) {
    std::vector<uint8_t> buf(kMaxDatagram);
    while (std::chrono::steady_clock::now() < deadline) {
        asio::error_code ec;
        const std::size_t n = sock.receive(asio::buffer(buf), 0, ec);
        if (ec == asio::error::would_block) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }
        if (ec) {
            return false;
        }
        if (n > 0 && buf[0] == kMsgAssignId) {
            report_assigned_id(buf, n);
            return true;
        }
    }
    return false;
}

}  // namespace

int main(int argc, char* argv[]) {
    const ClientOptions opts = parse_args(argc, argv);

    asio::io_context io;
    udp::socket sock(io);
    sock.open(udp::v4());
    sock.bind(udp::endpoint(udp::v4(), 0));
    sock.non_blocking(true);

    const udp::endpoint server(asio::ip::make_address(opts.host), opts.port);

    if (opts.connect) {
        send_datagram(sock, server, std::vector<uint8_t>{kMsgConnect});
        wait_for_assignment(sock, std::chrono::steady_clock::now() + std::chrono::seconds(kAssignTimeoutSeconds));
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(opts.listen_seconds);

    const uint8_t kind = send_kind_byte(opts.send);
    if (kind != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kSendGapMs));
        for (int i = 0; i < opts.repeat; ++i) {
            send_datagram(sock, server, make_payload_datagram(kind, opts.payload));
            std::cout << "SENT " << opts.send << ' ' << opts.payload << '\n';
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(kSendGapMs));
        }
    }

    receive_until(sock, deadline);
    return 0;
}
