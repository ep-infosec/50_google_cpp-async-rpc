/// \file
/// \brief Execution context with deadlines and cancellation.
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

#ifndef ARPC_CONTEXT_H_
#define ARPC_CONTEXT_H_

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "arpc/awaitable.h"
#include "arpc/container/flat_map.h"
#include "arpc/container/flat_set.h"
#include "arpc/dynamic_base_class.h"
#include "arpc/flag.h"
#include "arpc/serializable.h"

namespace arpc {

class context : public serializable<context> {
 public:
  ARPC_CUSTOM_SERIALIZATION_VERSION(1);

  template <typename E>
  void save(E& e) const {
    e(deadline_left());
    e(data());
    e(is_cancelled());
  }

  template <typename D>
  void load(D& d) {
    std::optional<duration> deadline_remaining;
    d(deadline_remaining);

    std::vector<std::shared_ptr<dynamic_base_class>> new_data;
    d(new_data);

    bool cancelled;
    d(cancelled);

    if (deadline_remaining) {
      set_timeout(*deadline_remaining);
    }
    set_data(std::move(new_data));
    if (cancelled) cancel();
  }

  using duration = std::chrono::milliseconds;
  using time_point =
      std::chrono::time_point<std::chrono::system_clock, duration>;

  context(context&&) = default;
  explicit context(context& parent = current(), bool set_current = true,
                   bool shield = false);

  ~context();

  void cancel();

  bool is_cancelled() const;

  std::optional<time_point> deadline() const;

  std::optional<duration> deadline_left() const;

  awaitable<void> wait_cancelled();

  awaitable<void> wait_deadline();

  static context& current();

  static context& top();

  void set_deadline(time_point when);

  void set_timeout(duration timeout);

  template <typename... V>
  void set(V&&... v) {
    std::scoped_lock lock(data_mu_);
    (..., set_one(std::forward<V>(v)));
  }

  template <typename... V>
  void reset() {
    std::scoped_lock lock(data_mu_);
    (..., reset_one<V>());
  }

  void reset_all();

  template <typename T>
  const T& get() const {
    std::scoped_lock lock(data_mu_);
    auto it = data_.find(portable_class_name<T>());
    if (it != data_.end()) {
      return static_cast<const T&>(*it->second);
    } else {
      return default_instance<T>();
    }
  }

 private:
  struct root {};
  explicit context(root);

  std::vector<std::shared_ptr<const dynamic_base_class>> data() const;
  void set_data(std::vector<std::shared_ptr<dynamic_base_class>>&& new_data);

  template <typename T>
  static const T& default_instance() {
    static const T instance;
    return instance;
  }

  template <typename T>
  void set_one(T&& t) {
    data_[t.portable_class_name()] =
        std::make_shared<const std::decay_t<T>>(std::forward<T>(t));
  }

  template <typename T>
  void reset_one() {
    data_.erase(portable_class_name<T>());
  }

  void add_child(context* child);
  void remove_child(context* child);

  template <bool daemon>
  friend class base_thread;

  mutable std::mutex data_mu_;
  mutable std::mutex children_mu_;
  std::condition_variable child_detached_;
  bool set_current_;
  context* parent_;
  context* previous_;
  arpc::flat_set<context*> children_;
  flag cancelled_;
  std::optional<time_point> deadline_;
  arpc::flat_map<std::string_view, std::shared_ptr<const dynamic_base_class>>
      data_;
  static thread_local context* current_;
};

class shield : public context {
 public:
  shield();
};

}  // namespace arpc

#endif  // ARPC_CONTEXT_H_
