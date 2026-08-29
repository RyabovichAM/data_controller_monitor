#include <fcntl.h>
#include <pty.h>
#include <unistd.h>

#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <asio/io_context.hpp>

#include "serial_transfer.h"

namespace {

// A pseudo terminal stands in for the port: termios treats it like any other,
// so the settings, the reading and the framing are exercised for real. Only the
// wire is imaginary.
class SerialPair : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(openpty(&master_, &slave_, nullptr, nullptr, nullptr), 0);
        port_name_ = ttyname(slave_);
        ASSERT_FALSE(port_name_.empty());
    }

    void TearDown() override {
        if (master_ != -1) {
            close(master_);
        }
        if (slave_ != -1) {
            close(slave_);
        }
    }

    transfer::Settings SettingsFor(const std::string& port_name) const {
        return {{"type", "Serial"},
                {"port_name", port_name},
                {"baud_rate", "115200"},
                {"data_bits", "8"},
                {"parity", "None"},
                {"stop_bits", "1"},
                {"flow_control", "None"}};
    }

    void Write(const std::string& bytes) const {
        ASSERT_EQ(write(master_, bytes.data(), bytes.size()),
                  static_cast<ssize_t>(bytes.size()));
    }

    int master_{-1};
    int slave_{-1};
    std::string port_name_;
};

}   //namespace

TEST_F(SerialPair, ReadsWhatTheControllerWrites) {
    asio::io_context io;
    transfer::SerialTransfer serial{SettingsFor(port_name_), io};

    std::vector<std::string> received;
    serial.SetJsonHandler([&](const std::string& json) {
        received.push_back(json);
        if (received.size() == 2) {
            io.stop();
        }
    });

    ASSERT_TRUE(serial.Start());

    Write(R"({"temperature":21.5})");
    Write("\r\n");
    Write(R"({"nested":{"deep":1}})");

    // The loop ends itself on the second message; the thread is a fuse for the
    // case where it never arrives.
    std::thread fuse{[&io]() {
        std::this_thread::sleep_for(std::chrono::seconds{5});
        io.stop();
    }};

    io.run();
    fuse.detach();

    ASSERT_EQ(received.size(), 2u);
    EXPECT_EQ(received[0], R"({"temperature":21.5})");
    EXPECT_EQ(received[1], R"({"nested":{"deep":1}})");
}

TEST_F(SerialPair, MissingPortIsReported) {
    asio::io_context io;
    transfer::SerialTransfer serial{SettingsFor("/dev/tty-that-is-not-there"), io};

    std::string reported;
    serial.SetErrorHandler([&](const std::string& error) { reported = error; });

    EXPECT_FALSE(serial.Start());
    EXPECT_NE(reported.find("/dev/tty-that-is-not-there"), std::string::npos);
}
