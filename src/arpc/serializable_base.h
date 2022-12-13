/// \file
/// \brief Header defining a base class for serializable data.
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

#ifndef ARPC_SERIALIZABLE_BASE_H_
#define ARPC_SERIALIZABLE_BASE_H_

#include <array>
#include <cstdint>
#include <type_traits>

#include "arpc/dynamic_base_class.h"
#include "arpc/errors.h"
#include "arpc/mpt.h"
#include "arpc/preprocessor.h"
#include "arpc/traits/type_traits.h"

namespace arpc {

namespace detail {
struct dynamic_class_filter {
  template <typename T>
  constexpr bool operator()(mpt::wrap_type<T>) {
    return is_dynamic<T>::value;
  }
};

template <bool dynamic, typename OwnType, typename... Bases>
struct serializable_mixin;

template <typename OwnType, typename... Bases>
struct serializable_mixin<false, OwnType, Bases...> : Bases... {
  using own_type = OwnType;
  using base_classes = mpt::pack<Bases...>;
  using dynamic_base_classes =
      mpt::filter_if_t<base_classes, dynamic_class_filter>;

  // Don't inherit load/save behavior from the parent, and provide an empty set
  // of fields.
  template <typename S>
  void save(S& s) const = delete;
  template <typename S>
  void load(S& s) = delete;
  using field_descriptors = mpt::pack<>;
};

template <typename OwnType, typename... Bases>
struct serializable_mixin<true, OwnType, Bases...>
    : serializable_mixin<false, OwnType, Bases...> {
 private:
  // Implement the virtual function that returns the name of our type.
  std::string_view portable_class_name_internal() const override {
    using Descriptor = arpc::detail::dynamic_class_descriptor<OwnType>;
    return Descriptor::class_name;
  }
};

}  // namespace detail

/// A field_descriptor type specifies how to access one data member.
template <auto mptr>
struct field_descriptor : public traits::member_data_pointer_traits<mptr> {
  /// Get the field's name.
  static const char* name() {
    using class_type = typename field_descriptor<mptr>::class_type;
    return class_type::field_names()
        [mpt::find_v<typename class_type::field_descriptors,
                     mpt::is_type<field_descriptor<mptr>>>];
  }
};

/// Inherit publicly from this in serializable classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using serializable = detail::serializable_mixin<
    ((mpt::count_if_v<mpt::pack<Bases...>, detail::dynamic_class_filter>) > 0),
    OwnType, Bases...>;

/// Inherit publicly from this in dynamic classes, specifying own type and
/// public bases.
template <typename OwnType, typename... Bases>
using dynamic = std::conditional_t<
    ((mpt::count_if_v<mpt::pack<Bases...>, detail::dynamic_class_filter>) > 0),
    serializable<OwnType, Bases...>,
    serializable<OwnType, ::arpc::dynamic_base_class, Bases...>>;

/// Define a `field_descriptor` type for a member field named `NAME`.
#define ARPC_FIELD(NAME) ::arpc::field_descriptor<&own_type::NAME>
#define ARPC_FIELD_SEP() ,
#define ARPC_FIELD_NAME(NAME) #NAME
#define ARPC_FIELD_NAME_SEP() ,

/// Needed to find our own type in template classes, as the base class is
/// dependent.
#define ARPC_OWN_TYPE(...) using own_type = __VA_ARGS__

/// Define the list of `field_descriptor` elements for the current class.
#define ARPC_FIELDS(...)                                                      \
  using field_descriptors = ::arpc::mpt::pack<ARPC_FOREACH(                   \
      ARPC_FIELD, ARPC_FIELD_SEP, __VA_ARGS__)>;                              \
  static const std::array<const char*,                                        \
                          ::arpc::mpt::size_v<field_descriptors>>&            \
  field_names() {                                                             \
    static const std::array<const char*,                                      \
                            ::arpc::mpt::size_v<field_descriptors>>           \
        names{                                                                \
            ARPC_FOREACH(ARPC_FIELD_NAME, ARPC_FIELD_NAME_SEP, __VA_ARGS__)}; \
    return names;                                                             \
  }

/// Version of the load/save methods.
#define ARPC_CUSTOM_SERIALIZATION_VERSION(VERSION)                 \
  static_assert(VERSION != 0,                                      \
                "Custom serialization version must be non-zero."); \
  static constexpr std::uint32_t custom_serialization_version = VERSION

}  // namespace arpc

#endif  // ARPC_SERIALIZABLE_BASE_H_
