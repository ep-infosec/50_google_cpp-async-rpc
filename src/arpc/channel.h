/// \file
/// \brief Channel descriptor wrapper.
///
/// \copyright
///   Copyright 2019 by Google LLC.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef ARPC_CHANNEL_H_
#define ARPC_CHANNEL_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <chrono>
#include <cstddef>
#include "arpc/address.h"
#include "arpc/awaitable.h"

namespace arpc {

class channel {
 public:
  static constexpr int default_backlog = 10;
  static constexpr std::chrono::seconds default_linger_time =
      std::chrono::seconds(10);

  channel() noexcept;
  explicit channel(int fd) noexcept;
  channel(channel&& fd) noexcept;
  ~channel() noexcept;
  void swap(channel& fd) noexcept;
  channel& operator=(channel&& fd) noexcept;
  int release() noexcept;
  int get() const noexcept;
  int operator*() const noexcept;
  void reset(int fd = -1) noexcept;
  explicit operator bool() const noexcept;
  void close() noexcept;
  std::size_t read(void* buf, std::size_t len);
  std::size_t write(const void* buf, std::size_t len);
  channel& make_non_blocking(bool non_blocking = true);
  channel dup() const;
  awaitable<void> can_read();
  awaitable<void> can_write();
  std::size_t maybe_read(void* buf, std::size_t len);
  std::size_t maybe_write(const void* buf, std::size_t len);
  awaitable<std::size_t> async_read(void* buf, std::size_t len);
  awaitable<std::size_t> async_write(const void* buf, std::size_t len);

  // Socket methods.
  channel& flush();
  address own_addr() const;
  address peer_addr() const;
  channel& shutdown(bool read, bool write);
  awaitable<void> async_connect(const address& addr);
  channel& connect(const address& addr);
  channel& bind(const address& addr);
  channel& listen(int backlog = default_backlog);
  awaitable<channel> async_accept();
  channel accept();
  channel maybe_accept();
  awaitable<channel> async_accept(address& addr);
  channel accept(address& addr);
  channel maybe_accept(address& addr);
  channel& keep_alive(bool keep_alive = true);
  channel& reuse_addr(bool reuse = true);
  channel& reuse_port(bool reuse = true);
  channel& linger(bool do_linger = true,
                  std::chrono::seconds linger_time = default_linger_time);
  channel& no_delay(bool no_delay = true);

 private:
  int fd_;
};

}  // namespace arpc

#endif  // ARPC_CHANNEL_H_
