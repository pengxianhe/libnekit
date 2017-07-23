#include <cassert>

#include "nekit/transport/tcp_socket.h"
#include "nekit/utils/auto.h"

namespace nekit {
namespace transport {

TcpSocket::TcpSocket(boost::asio::ip::tcp::socket &&socket)
    : socket_{std::move(socket)} {}

void TcpSocket::Read(std::unique_ptr<utils::Buffer> &&buffer,
                     TransportInterface::EventHandler &&handler) {
  read_buffer_ = std::move(buffer);
  read_handler_ = std::move(handler);
  socket_.async_read_some(
      boost::asio::mutable_buffers_1(buffer->buffer(), buffer->capacity()),
      [this](const boost::system::error_code &ec,
             std::size_t bytes_transferred) mutable {

        Auto(this->read_buffer_ = nullptr;
             this->read_handler_ = TransportInterface::EventHandler(););

        if (ec) {
          if (ec.category() == boost::asio::error::system_category &&
              ec.value() ==
                  boost::asio::error::basic_errors::operation_aborted) {
            return;
          }

          auto error = ConvertBoostError(ec);
          if (error == ErrorCode::EndOfFile) {
            this->read_closed_ = true;
          }
          this->read_handler_(std::move(this->read_buffer_), error);
          return;
        }

        this->read_buffer_->ReserveBack(this->read_buffer_->capacity() -
                                        bytes_transferred);
        this->read_handler_(std::move(this->read_buffer_), ErrorCode::NoError);
        return;
      });
};

void TcpSocket::Write(std::unique_ptr<utils::Buffer> &&buffer,
                      TransportInterface::EventHandler &&handler) {
  write_buffer_ = std::move(buffer);
  write_handler_ = std::move(handler);
  boost::asio::async_write(
      socket_,
      boost::asio::const_buffers_1(buffer->buffer(), buffer->capacity()),
      [this](const boost::system::error_code &ec,
             std::size_t bytes_transferred) mutable {
        assert(bytes_transferred == this->write_buffer_->capacity());

        Auto(this->write_buffer_ = nullptr;
             this->write_handler_ = TransportInterface::EventHandler(););

        if (ec) {
          if (ec.category() == boost::asio::error::system_category &&
              ec.value() ==
                  boost::asio::error::basic_errors::operation_aborted) {
            return;
          }

          this->write_handler_(std::move(this->write_buffer_),
                               ConvertBoostError(ec));
          return;
        }

        this->write_handler_(std::move(this->write_buffer_),
                             ErrorCode::NoError);
        return;
      });
}

void TcpSocket::CloseRead() {
  if (read_closed_) return;

  boost::system::error_code ec;
  socket_.shutdown(socket_.shutdown_receive, ec);
  // It is not likely there will be any error other than those related to SSL.
  // But SSL is never used in TcpSocket.
  assert(!ec);
  read_closed_ = true;
}

void TcpSocket::CloseWrite() {
  if (write_closed_) return;

  boost::system::error_code ec;
  socket_.shutdown(socket_.shutdown_send, ec);
  assert(!ec);
  write_closed_ = true;
}

void TcpSocket::Close() {
  CloseRead();
  CloseWrite();
}

bool TcpSocket::IsReadClosed() const { return read_closed_; }

bool TcpSocket::IsWriteClosed() const { return write_closed_; }

bool TcpSocket::IsClosed() const { return IsReadClosed() && IsWriteClosed(); }

utils::Endpoint TcpSocket::localEndpoint() const {
  boost::system::error_code ec;
  utils::Endpoint endpoint = socket_.local_endpoint(ec);
  assert(!ec);
  return endpoint;
}

utils::Endpoint TcpSocket::remoteEndpoint() const {
  boost::system::error_code ec;
  utils::Endpoint endpoint = socket_.remote_endpoint(ec);
  assert(!ec);
  return endpoint;
}

TcpSocket::ErrorCode TcpSocket::ConvertBoostError(
    const boost::system::error_code &ec) const {
  if (ec.category() == boost::asio::error::system_category) {
    switch (ec.value()) {
      case boost::asio::error::basic_errors::connection_aborted:
        return ErrorCode::ConnectionAborted;
      case boost::asio::error::basic_errors::connection_reset:
        return ErrorCode::ConnectionReset;
      case boost::asio::error::basic_errors::host_unreachable:
        return ErrorCode::HostUnreachable;
      case boost::asio::error::basic_errors::network_down:
        return ErrorCode::NetworkDown;
      case boost::asio::error::basic_errors::network_reset:
        return ErrorCode::NetworkReset;
      case boost::asio::error::basic_errors::network_unreachable:
        return ErrorCode::NetworkUnreachable;
      case boost::asio::error::basic_errors::timed_out:
        return ErrorCode::TimedOut;
    }
  } else if (ec.category() == boost::asio::error::misc_category) {
    if (ec.value() == boost::asio::error::misc_errors::eof) {
      return ErrorCode::EndOfFile;
    }
  }
  return ErrorCode::UnknownError;
}  // namespace transport

namespace {
struct TcpSocketErrorCategory : std::error_category {
  const char *name() const noexcept override;
  std::string message(int) const override;
};

const char *TcpSocketErrorCategory::name() const noexcept {
  return "TCP socket";
}

std::string TcpSocketErrorCategory::message(int ev) const {
  switch (static_cast<TcpSocket::ErrorCode>(ev)) {
    case TcpSocket::ErrorCode::NoError:
      return "no error";
    case TcpSocket::ErrorCode::ConnectionAborted:
      return "connection aborted";
    case TcpSocket::ErrorCode::ConnectionReset:
      return "connection reset";
    case TcpSocket::ErrorCode::HostUnreachable:
      return "host unreachable";
    case TcpSocket::ErrorCode::NetworkDown:
      return "network down";
    case TcpSocket::ErrorCode::NetworkReset:
      return "network reset";
    case TcpSocket::ErrorCode::NetworkUnreachable:
      return "network unreachable";
    case TcpSocket::ErrorCode::TimedOut:
      return "timeout";
    case TcpSocket::ErrorCode::EndOfFile:
      return "end of file";
    case TcpSocket::ErrorCode::UnknownError:
      return "unknown error";
  }
}

const TcpSocketErrorCategory tcpSocketErrorCategory{};

}  // namespace

std::error_code make_error_code(TcpSocket::ErrorCode e) {
  return {static_cast<int>(e), tcpSocketErrorCategory};
}
}  // namespace transport
}  // namespace nekit