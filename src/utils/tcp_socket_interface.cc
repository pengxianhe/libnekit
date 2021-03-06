// MIT License

// Copyright (c) 2017 Zhuhao Wang

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "nekit/utils/tcp_socket_interface.h"

namespace nekit {
namespace utils {

TcpSocketInterface::ConnectRequest::ConnectRequest(std::string domain,
                                                   uint16_t port)
    : type_(kDomain), domain_(domain), port_(port) {}

TcpSocketInterface::ConnectRequest::ConnectRequest(
    boost::asio::ip::tcp::endpoint endpoint)
    : type_(kEndpoint), endpoint_(endpoint) {}

TcpSocketInterface::ConnectRequest::ConnectRequest(
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : type_(kEndpointIterator), endpoint_iterator_(endpoint_iterator) {}

TcpSocketInterface::ConnectRequest::Type
TcpSocketInterface::ConnectRequest::type() const {
  return type_;
}

std::string TcpSocketInterface::ConnectRequest::domain() const {
  return domain_;
}

uint16_t TcpSocketInterface::ConnectRequest::port() const { return port_; }

boost::asio::ip::tcp::endpoint TcpSocketInterface::ConnectRequest::endpoint()
    const {
  return endpoint_;
}

boost::asio::ip::tcp::resolver::iterator
TcpSocketInterface::ConnectRequest::endpoint_iterator() const {
  return endpoint_iterator_;
}

const char* TcpSocketInterface::ErrorCategory::name() const BOOST_NOEXCEPT {
  return "NEKit::utils::TcpSocket";
}

std::string TcpSocketInterface::ErrorCategory::message(int error_code) const {
  switch (ErrorCode(error_code)) {
    case ErrorCode::kNoError:
      return "No StreamCoder set.";
    case ErrorCode::kUnknownError:
      return "Unknown error.";
  }
}

const std::error_category& TcpSocketInterface::error_category() {
  static ErrorCategory category_;
  return category_;
}
}  // namespace utils
}  // namespace nekit

namespace std {
error_code make_error_code(nekit::utils::TcpSocketInterface::ErrorCode errc) {
  return error_code(static_cast<int>(errc),
                    nekit::utils::TcpSocketInterface::error_category());
}
}  // namespace std
