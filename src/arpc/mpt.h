/// \file
/// \brief Metaprogramming toolkit.
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

#ifndef ARPC_MPT_H_
#define ARPC_MPT_H_

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include "arpc/preprocessor.h"
#include "arpc/traits/type_traits.h"

namespace arpc {

/// Type-based meta-programming toolkit.
namespace mpt {

/// \brief Define a whole set of arithmetic operators.
/// Macro to define the whole set of arithmetic operations given a pair of
/// macros
/// for unary and binary operations.
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator
/// in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary
/// operator in terms of the `NAME` and the operation (`OP`).
#define ARPC_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
  CREATE_UNARY_OPERATOR(negate, -);                                                   \
  CREATE_UNARY_OPERATOR(logical_not, !);                                              \
  CREATE_UNARY_OPERATOR(bit_not, ~);                                                  \
  CREATE_BINARY_OPERATOR(plus, +);                                                    \
  CREATE_BINARY_OPERATOR(minus, -);                                                   \
  CREATE_BINARY_OPERATOR(multiplies, *);                                              \
  CREATE_BINARY_OPERATOR(divides, /);                                                 \
  CREATE_BINARY_OPERATOR(modulus, %);                                                 \
  CREATE_BINARY_OPERATOR(equal_to, ==);                                               \
  CREATE_BINARY_OPERATOR(not_equal_to, !=);                                           \
  CREATE_BINARY_OPERATOR(greater, >);                                                 \
  CREATE_BINARY_OPERATOR(less, <);                                                    \
  CREATE_BINARY_OPERATOR(greater_equal, >=);                                          \
  CREATE_BINARY_OPERATOR(less_equal, <=);                                             \
  CREATE_BINARY_OPERATOR(logical_and, &&);                                            \
  CREATE_BINARY_OPERATOR(logical_or, ||);                                             \
  CREATE_BINARY_OPERATOR(bit_and, &);                                                 \
  CREATE_BINARY_OPERATOR(bit_or, |);                                                  \
  CREATE_BINARY_OPERATOR(bit_xor, ^);

/// \brief Define a whole set of arithmetic operators plus the identity.
/// Macro to define the whole set of arithmetic operations (plus the identitiy
/// operation)
/// given a pair of macros for unary and binary operations.
///
/// Notice that the identity operation is special in the sense that the value of
/// `OP` for it
/// is the empty string, and thus it's not suitable to define a `operator OP ()`
/// function.
///
/// \param CREATE_UNARY_OPERATOR Macro to be used for defining a unary operator
/// in terms of the `NAME` and the operation (`OP`).
/// \param CREATE_BINARY_OPERATOR Macro to be used for defining a binary
/// operator in terms of the `NAME` and the operation (`OP`).
#define ARPC_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(CREATE_UNARY_OPERATOR,     \
                                                     CREATE_BINARY_OPERATOR)    \
  ARPC_CREATE_OPERATOR_HIERARCHY(CREATE_UNARY_OPERATOR, CREATE_BINARY_OPERATOR) \
  CREATE_UNARY_OPERATOR(identity, /**/);

/// Define unary arithmetic operation functors on arbitrary types.
#define ARPC_CREATE_UNARY_OPERATOR(NAME, OP) \
  struct NAME {                              \
    template <typename T>                    \
    constexpr auto operator()(T v) {         \
      return OP v;                           \
    }                                        \
  };
/// Define binary arithmetic operation functors on arbitrary types.
#define ARPC_CREATE_BINARY_OPERATOR(NAME, OP) \
  struct NAME {                               \
    template <typename T, typename U>         \
    constexpr auto operator()(T v, U w) {     \
      return v OP w;                          \
    }                                         \
  };
/// Create a complete set of arithmetic operation functors on arbitrary types.
ARPC_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ARPC_CREATE_UNARY_OPERATOR,
                                             ARPC_CREATE_BINARY_OPERATOR);

/// \brief Types representing sequences of homogeneously typed integers.
/// An `integer_sequence<T, ints...>` type represents a compile-time sequence of
/// a variable number of integers (given by `ints...`), all of the same type
/// `T`.
/// \param T The type of all the integers.
/// \param ints... The list of `T`-typed integers to encode in the type.
template <typename T, T... ints>
struct integer_sequence {
  /// The type's own type.
  using type = integer_sequence<T, ints...>;

  /// The type of the integers in the sequence.
  using value_type = T;

  /// The number of integers in the sequence.
  static constexpr std::size_t size = sizeof...(ints);

 private:
  template <typename O>
  struct unary_op {
    using result_type = std::remove_cv_t<decltype(O()(T()))>;
    using type = integer_sequence<result_type, (O()(ints))...>;
  };

  template <typename U, typename O>
  struct binary_op;

  template <typename U, U... ints_u, typename O>
  struct binary_op<integer_sequence<U, ints_u...>, O> {
    using result_type = std::remove_cv_t<decltype(O()(T(), U()))>;
    using type = integer_sequence<result_type, (O()(ints, ints_u))...>;
  };

 public:
/// Type-based unary operator generator macro.
#define ARPC_INT_SEQ_TYPE_UNARY_OP(NAME, OP)    \
  struct NAME : unary_op<::arpc::mpt::NAME> {}; \
  using NAME##_t = typename NAME::type;
/// Type-based binary operator generator macro.
#define ARPC_INT_SEQ_TYPE_BINARY_OP(NAME, OP)       \
  template <typename U>                             \
  struct NAME : binary_op<U, ::arpc::mpt::NAME> {}; \
  template <typename U>                             \
  using NAME##_t = typename NAME<U>::type;
  /// Type-based operator hierarchy.
  ARPC_CREATE_OPERATOR_HIERARCHY_WITH_IDENTITY(ARPC_INT_SEQ_TYPE_UNARY_OP,
                                               ARPC_INT_SEQ_TYPE_BINARY_OP);

/// Value-based unary operator generator macro.
#define ARPC_INT_SEQ_VALUE_UNARY_OP(NAME, OP) \
  constexpr NAME##_t operator OP() { return {}; }
/// Value-based binary operator generator macro.
#define ARPC_INT_SEQ_VALUE_BINARY_OP(NAME, OP) \
  template <typename U>                        \
  constexpr NAME##_t<U> operator OP(U) {       \
    return {};                                 \
  }
  // Value-based operator hierarchy.
  ARPC_CREATE_OPERATOR_HIERARCHY(ARPC_INT_SEQ_VALUE_UNARY_OP, ARPC_INT_SEQ_VALUE_BINARY_OP);
};

namespace detail {

template <typename Is, typename Js, bool adjust_values = false>
struct join_integer_sequences;

template <typename T, T... is, T... js, bool adjust_values>
struct join_integer_sequences<integer_sequence<T, is...>, integer_sequence<T, js...>,
                              adjust_values> {
  using type = integer_sequence<T, is..., ((adjust_values ? sizeof...(is) : 0) + js)...>;
};

template <typename T, std::size_t n>
struct make_integer_sequence
    : join_integer_sequences<typename make_integer_sequence<T, n / 2>::type,
                             typename make_integer_sequence<T, n - n / 2>::type, true> {};

template <typename T>
struct make_integer_sequence<T, 1> {
  using type = integer_sequence<T, 0>;
};

template <typename T>
struct make_integer_sequence<T, 0> {
  using type = integer_sequence<T>;
};

template <typename T, std::size_t n, T v>
struct make_constant_integer_sequence
    : join_integer_sequences<typename make_constant_integer_sequence<T, n / 2, v>::type,
                             typename make_constant_integer_sequence<T, n - n / 2, v>::type> {};

template <typename T, T v>
struct make_constant_integer_sequence<T, 1, v> {
  using type = integer_sequence<T, v>;
};

template <typename T, T v>
struct make_constant_integer_sequence<T, 0, v> {
  using type = integer_sequence<T>;
};

}  // namespace detail

/// \brief Generate a sequence of integers.
/// The integers are consecutive, in the range `0 .. n-1`.
/// \param T The type of the integers.
/// \param n The total number of integers the sequence will have.
template <typename T, std::size_t n>
using make_integer_sequence = typename detail::make_integer_sequence<T, n>::type;

/// \brief Generate a repeated sequence of the same integer.
/// This will generate `n` integers with the value `v` of type `T`.
/// \param T The type of the integers.
/// \param n The total number of integers the sequence will have.
/// \param v The value of the integers.
template <typename T, std::size_t n, T v>
using make_constant_integer_sequence =
    typename detail::make_constant_integer_sequence<T, n, v>::type;

/// \brief Types representing sequences of `std::size_t` which can be used as
/// indices.
/// An `index_sequence<ints...>` type represents a compile-time sequence of
/// a variable number of `std::size_t` values (given by `ints...`).
/// \param ints... The list of `std::size_t`-typed integers to encode in the
/// type.
template <std::size_t... ints>
using index_sequence = integer_sequence<std::size_t, ints...>;
/// \brief Generate a sequence of indices.
/// The integers are consecutive, in the range `0 .. n-1`, and of `std::size_t`
/// type.
/// \param n The total number of integers the sequence will have.
template <std::size_t n>
using make_index_sequence = make_integer_sequence<std::size_t, n>;
/// \brief Generate a repeated sequence of the same index.
/// This will generate `n` indices with the value `v` of type `std::size_t`.
/// \param n The total number of indices the sequence will have.
/// \param v The value of the indices.
template <std::size_t n, std::size_t v>
using make_constant_index_sequence = make_constant_integer_sequence<std::size_t, n, v>;

// Technique for detecting the number of fields from an aggregate adapted from
// https://github.com/felixguendling/cista
//
// Credits: Implementation by Anatoliy V. Tomilov (@tomilov),
//          based on gist by Rafal T. Janik (@ChemiaAion)
//
// Resources:
// https://playfulprogramming.blogspot.com/2016/12/serializing-structs-with-c17-structured.html
// https://codereview.stackexchange.com/questions/142804/get-n-th-data-member-of-a-struct
// https://stackoverflow.com/questions/39768517/structured-bindings-width
// https://stackoverflow.com/questions/35463646/arity-of-aggregate-in-logarithmic-time
// https://stackoverflow.com/questions/38393302/returning-variadic-aggregates-struct-and-syntax-for-c17-variadic-template-c
namespace detail {

struct wildcard_type {
  template <typename T>
  operator T() const;
};

template <typename Aggregate, typename IndexSequence = index_sequence<>, typename = void>
struct aggregate_arity_impl : IndexSequence {};

template <typename Aggregate, std::size_t... Indices>
struct aggregate_arity_impl<
    Aggregate, index_sequence<Indices...>,
    std::void_t<decltype(Aggregate{(static_cast<void>(Indices), std::declval<wildcard_type>())...,
                                   std::declval<wildcard_type>()})>>
    : aggregate_arity_impl<Aggregate, index_sequence<Indices..., sizeof...(Indices)>> {};

}  // namespace detail

template <typename T>
struct aggregate_arity
    : std::integral_constant<std::size_t,
                             detail::aggregate_arity_impl<traits::remove_cvref_t<T>>::size> {};

template <typename T>
inline constexpr std::size_t aggregate_arity_v = aggregate_arity<T>::value;

/// \brief Wrap one type so that it can be passed around without constructing
/// any instance of it.
/// `wrap_type<T>` encodes a single type in a class whose objects can be passed
/// around at runtime
/// without having to copy any data.
/// \param T The type to wrap.
template <typename T>
struct wrap_type {
  /// Actual type wrapped by the `wrap_type` specialization.
  using type = T;
};

/// \brief Template class to represent sequences of types.
/// `pack` structs contain no data; all the information they contain is the list
/// of types represented by `T...`.
/// \param T... List of types to pack.
template <typename... T>
struct pack {};

/// \brief Template class to represent sequences of constexpr values.
/// `value_pack` structs contain no data; all the information they contain is
/// the static list of values represented by `v...`.
/// \param v... List of values to pack, possibly of different types.
template <auto... v>
struct value_pack {};

// Helpers to determine if a type is a pack / tuple / integer_sequence.
namespace detail {
template <typename T>
struct is_tuple : public std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : public std::true_type {};

template <typename U, typename V>
struct is_tuple<std::pair<U, V>> : public std::true_type {};

template <typename T>
struct is_pack : public std::false_type {};

template <typename... T>
struct is_pack<pack<T...>> : public std::true_type {};

template <typename T>
struct is_value_pack : public std::false_type {};

template <auto... v>
struct is_value_pack<value_pack<v...>> : public std::true_type {};

template <typename T>
struct is_integer_sequence : public std::false_type {};

template <typename T, T... ints>
struct is_integer_sequence<integer_sequence<T, ints...>> : public std::true_type {};
}  // namespace detail

template <typename T>
using is_tuple = detail::is_tuple<traits::remove_cvref_t<T>>;
template <typename T>
static constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename T>
using is_pack = detail::is_pack<traits::remove_cvref_t<T>>;
template <typename T>
static constexpr bool is_pack_v = is_pack<T>::value;

template <typename T>
using is_value_pack = detail::is_value_pack<traits::remove_cvref_t<T>>;
template <typename T>
static constexpr bool is_value_pack_v = is_value_pack<T>::value;

template <typename T>
using is_integer_sequence = detail::is_integer_sequence<traits::remove_cvref_t<T>>;
template <typename T>
static constexpr bool is_integer_sequence_v = is_integer_sequence<T>::value;

namespace detail {
// Get the number of types.
template <typename T, typename Enable = void>
struct size;

template <typename... T>
struct size<pack<T...>> {
  static constexpr std::size_t value = sizeof...(T);
};

template <auto... v>
struct size<value_pack<v...>> {
  static constexpr std::size_t value = sizeof...(v);
};

template <typename T>
struct size<T, std::enable_if_t<is_tuple_v<T>>> {
  static constexpr std::size_t value = std::tuple_size_v<T>;
};

template <typename T, T... ints>
struct size<integer_sequence<T, ints...>> {
  static constexpr std::size_t value = sizeof...(ints);
};
}  // namespace detail

/// \brief Convert a bindable aggregate value into a tuple value.
/// The result is a `std::tuple` with as many elements as the aggregate with
/// every element being a reference to each field.
/// \return An appropriate tuple type, containing references to the aggregate
/// fields.
namespace detail {
template <std::size_t arity>
auto unsupported_arity() {
  static_assert(!(arity >= 0), "Unsupported arity");
  return std::forward_as_tuple();
}
}  // namespace detail

#define ARPC_MAKE_BINDER_BINDING(field, ...) f##field
#define ARPC_MAKE_BINDER_BINDING_SEP() ,
#define ARPC_MAKE_BINDER_BINDING_LIST(lastfield) \
  ARPC_DEFER_2(ARPC_FOR_AGAIN)                   \
  ()(lastfield, ARPC_MAKE_BINDER_BINDING, ARPC_MAKE_BINDER_BINDING_SEP)
#define ARPC_MAKE_BINDER_CASE(lastfield, ...)                               \
  if constexpr (aggregate_arity_v<T> == lastfield) {                        \
    auto& [ARPC_MAKE_BINDER_BINDING_LIST(lastfield)] = t;                   \
    return std::forward_as_tuple(ARPC_MAKE_BINDER_BINDING_LIST(lastfield)); \
  }
#define ARPC_MAKE_BINDER_CASE_SEP() else

template <typename T>
constexpr auto as_tuple(
    T&& t,
    std::enable_if_t<traits::is_bindable_aggregate_v<traits::remove_cvref_t<T>>>* dummy = nullptr) {
  ARPC_EXPAND(ARPC_FOR(32, ARPC_MAKE_BINDER_CASE, ARPC_MAKE_BINDER_CASE_SEP))
  else if constexpr (aggregate_arity_v<T> == 0) {
    return std::forward_as_tuple();
  }
  else {
    return detail::unsupported_arity<aggregate_arity_v<T>>();
  }
}

/// \brief Convert an integer sequence value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// all of them of type `T`, set to the values of `ints...`.
/// \return An appropriate tuple type, containing the values of `ints...`.
template <typename T, T... ints>
constexpr auto as_tuple(integer_sequence<T, ints...>) {
  return std::make_tuple(ints...);
}

/// \brief Convert an value pack sequence value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// each one of the type and value of `v...`.
/// \return An appropriate tuple type, containing the values of `v...`.
template <auto... v>
constexpr auto as_tuple(value_pack<v...>) {
  return std::make_tuple(v...);
}

/// \brief Convert an `pack` value into a tuple value.
/// The result is a `std::tuple` with as many elements as the input sequence,
/// every one of the wrapped type as the same-index element of the `pack`.
///
/// \return An appropriate tuple type, default-initialized.
template <typename... T>
constexpr std::tuple<wrap_type<T>...> as_tuple(pack<T...>) {
  return {};
}

/// \brief Convert a a tuple value into a tuple value.
/// The result is the input.
///
/// \return The input, as-is.
template <typename... T>
constexpr decltype(auto) as_tuple(const std::tuple<T...>& t) {
  return t;
}
template <typename... T>
constexpr decltype(auto) as_tuple(std::tuple<T...>& t) {
  return t;
}
template <typename... T>
constexpr decltype(auto) as_tuple(const std::tuple<T...>&& t) {
  return t;
}
template <typename... T>
constexpr decltype(auto) as_tuple(std::tuple<T...>&& t) {
  return t;
}

/// \brief Convert a a pair value into a tuple value.
/// The result is the input.
///
/// \return The input, as-is.
template <typename U, typename V>
constexpr decltype(auto) as_tuple(const std::pair<U, V>& t) {
  return t;
}
template <typename U, typename V>
constexpr decltype(auto) as_tuple(std::pair<U, V>& t) {
  return t;
}
template <typename U, typename V>
constexpr decltype(auto) as_tuple(const std::pair<U, V>&& t) {
  return t;
}
template <typename U, typename V>
constexpr decltype(auto) as_tuple(std::pair<U, V>&& t) {
  return t;
}

/// \brief Convert a tuple value into a `pack` value.
/// The result is a `pack` with as many elements as the input `std::tuple`,
/// every one of the unwrapped type of the same-index element of the
/// `std::tuple`. \return An appropriate `pack` type with the types in the
/// tuple.
template <typename... T>
constexpr pack<typename T::type...> as_pack(std::tuple<T...>) {
  return {};
}
/// \brief Convert a pair value into a `pack` value.
/// The result is a `pack` with as many elements as the input `std::pair`,
/// every one of the unwrapped type of the same-index element of the
/// `std::tuple`. \return An appropriate `pack` type with the types in the
/// tuple.
template <typename U, typename V>
constexpr pack<U, V> as_pack(std::pair<U, V>) {
  return {};
}

/// \brief Convert an `integer_sequence` value into a `pack` value.
/// The result is a `pack` with as many elements as the input
/// `integer_sequence`,
/// every one of the same type, which is the common type of the
/// `integer_sequence`
/// elements.
/// \return An appropriate `pack` type with all elements of the integer type in
/// the sequence.
template <typename T, T... ints>
constexpr auto as_pack(integer_sequence<T, ints...>) {
  return as_pack(std::make_tuple(ints...));
}

/// \brief Convert an `value_sequence` value into a `pack` value.
/// The result is a `pack` with as many elements as the input
/// `integer_sequence`,
/// every one of the wrapped type of each element's type.
/// \return An appropriate `pack` type with all elements of the integer type in
/// the sequence.
template <auto... v>
constexpr auto as_pack(value_pack<v...>) {
  return as_pack(std::make_tuple(v...));
}

template <typename... T>
constexpr auto as_pack(pack<T...> t) {
  return t;
}

/// \brief Get the size of a sequence-like type.
/// This template struct provides a standard way to look at the size (number of
/// elements)
/// in any of the sequence types supported by `arpc::mpt`.
/// \param T The type from which to obtain the size.
template <typename T>
struct size : std::integral_constant<std::size_t, detail::size<traits::remove_cvref_t<T>>::value> {
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::value;

/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard way to look at the type
/// of a particular element in any of the sequence types supported by
/// `arpc::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename T>
struct element_type {
  /// Type of the `i`th element.
  using type =
      std::tuple_element_t<i, traits::remove_cvref_t<decltype(as_tuple(std::declval<T>()))>>;
};

/// \brief Get the type of the `i`th element of a sequence-like type.
/// This template struct provides a standard shortcut way to look at the type
/// of a particular element in any of the sequence types supported by
/// `arpc::mpt`.
/// \param T The type for which to inspect the element type.
/// \param i The index of the element for which we want to retrieve the type.
template <std::size_t i, typename T>
using element_type_t = typename element_type<i, T>::type;

template <std::size_t i, typename... T>
constexpr decltype(auto) at(const std::tuple<T...>& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename... T>
constexpr decltype(auto) at(std::tuple<T...>& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename... T>
constexpr decltype(auto) at(const std::tuple<T...>&& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename... T>
constexpr decltype(auto) at(std::tuple<T...>&& t) {
  return std::get<i>(std::move(t));
}

template <std::size_t i, typename U, typename V>
constexpr decltype(auto) at(const std::pair<U, V>& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename U, typename V>
constexpr decltype(auto) at(std::pair<U, V>& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename U, typename V>
constexpr decltype(auto) at(const std::pair<U, V>&& t) {
  return std::get<i>(t);
}
template <std::size_t i, typename U, typename V>
constexpr decltype(auto) at(std::pair<U, V>&& t) {
  return std::get<i>(std::move(t));
}

template <std::size_t i, typename... T>
constexpr auto at(pack<T...> t) {
  return std::get<i>(as_tuple(t));
}
template <std::size_t i, auto... v>
constexpr auto at(value_pack<v...> t) {
  return std::get<i>(as_tuple(t));
}
template <std::size_t i, typename T, T... ints>
constexpr auto at(integer_sequence<T, ints...> t) {
  return std::get<i>(as_tuple(t));
}

/// \brief Convert an input sequence to a `value_pack`.
///
/// \return The `as_tuple` values of the input sequence, packed into a
/// `value_pack`'s parameters.
template <auto... v>
constexpr value_pack<v...> as_value_pack(value_pack<v...>) {
  return {};
}
template <typename T, T... ints>
constexpr value_pack<ints...> as_value_pack(integer_sequence<T, ints...>) {
  return {};
}

namespace detail {
template <typename T, typename F, std::size_t... ints, typename... Args>
constexpr void for_each(T&& v, index_sequence<ints...>, F f, Args&&... a) {
  (void)f;
  (void)(..., void(f(at<ints>(std::forward<T>(v)), a...)));
}

template <typename T, typename F, std::size_t... ints, typename... Args>
constexpr auto transform(T&& v, index_sequence<ints...>, F f, Args&&... a) {
  (void)f;
  return std::make_tuple(f(at<ints>(std::forward<T>(v)), a...)...);
}
}  // namespace detail

/// \brief Run a functor for each element in a sequence type.
///
/// `for_each` performs compile-time iteration on sequences.
///
/// When calling:
///  `for_each(sequence_value, functor, args...);`
///
/// For each element in the sequence, functor will be called in this way
/// (for every `i` in `0 ... size`):
///
///  `functor(at<i>(sequence_value), args...);`
///
/// This function doesn't return any value.
/// \param v The sequence over which to iterate.
/// \param f The functor to call on every element.
/// \param args... Further arguments to forward to the functor call.
template <typename T, typename F, typename... Args>
constexpr void for_each(T&& v, F&& f, Args&&... args) {
  detail::for_each(std::forward<T>(v), make_index_sequence<size_v<T>>{}, std::forward<F>(f),
                   std::forward<Args>(args)...);
}

/// \brief Run a functor for each element in a sequence type and return the
/// results.
///
/// `transform` performs compile-time iteration on sequences returning the
/// resulting values.
///
/// When calling:
///  `transform(sequence_value, functor, args...);`
///
/// For each element in the sequence, functor will be called in this way
/// (for every `i` in `0 ... size`):
///
///  `functor(at<i>(sequence_value), args...);`
///
/// The returned result will be a `std::tuple` composed of all the values
/// returned
/// by the successive calls of the functor (each one can have a distinct type).
/// \param v The sequence over which to iterate.
/// \param f The functor to call on every element.
/// \param args... Further arguments to forward to the functor call.
/// \return A `std::tuple` of appropriate type to contain the results of every
/// call.
template <typename T, typename F, typename... Args>
constexpr auto transform(T&& v, F&& f, Args&&... args) {
  return detail::transform(std::forward<T>(v), make_index_sequence<size_v<T>>{}, std::forward<F>(f),
                           std::forward<Args>(args)...);
}

/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...idxs The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr auto subset(std::tuple<T...>& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr auto subset(const std::tuple<T...>& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr auto subset(std::tuple<T...>&& t, index_sequence<idxs...>) {
  return std::make_tuple(std::move(at<idxs>(t))...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr auto subset(const std::tuple<T...>&& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}

/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...idxs The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename U, typename V, std::size_t... idxs>
constexpr auto subset(std::pair<U, V>& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename U, typename V, std::size_t... idxs>
constexpr auto subset(const std::pair<U, V>& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename U, typename V, std::size_t... idxs>
constexpr auto subset(std::pair<U, V>&& t, index_sequence<idxs...>) {
  return std::make_tuple(std::move(at<idxs>(t))...);
}
/// Return a new tuple containing a subset of the fields as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename U, typename V, std::size_t... idxs>
constexpr auto subset(const std::pair<U, V>&& t, index_sequence<idxs...>) {
  return std::forward_as_tuple(at<idxs>(t)...);
}

/// Return a new pack containing a subset of the types as determined by the
/// passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename... T, std::size_t... idxs>
constexpr pack<typename element_type_t<idxs, pack<T...>>::type...> subset(pack<T...>,
                                                                          index_sequence<idxs...>) {
  return {};
}
/// Return a new value pack containing a subset of the types as determined by
/// the passed index sequence.
/// \param t The sequence to subset.
/// \param ...ints The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <auto... v, std::size_t... idxs>
constexpr value_pack<at<idxs>(value_pack<v...>{})...> subset(value_pack<v...>,
                                                             index_sequence<idxs...>) {
  return {};
}
/// Return a new integer sequence containing a subset of the integers determined
/// by the passed index sequence.
/// \param t The sequence to subset.
/// \param ...idxs The indices to extract.
/// \return A sliced sequence containing just the elements specified by the
/// indices.
template <typename T, T... ints, std::size_t... idxs>
constexpr integer_sequence<T, at<idxs>(integer_sequence<T, ints...>{})...> subset(
    integer_sequence<T, ints...>, index_sequence<idxs...>) {
  return {};
}

/// Return a subsequence based on a semi-open index range.
/// \param t The sequence from which to extract a range.
/// \param begin The first index to extract.
/// \param end The index following the last one to extract.
/// \return A sliced sequence containing just the elements in the range `begin
/// .. end`.
template <std::size_t begin, std::size_t end, typename T>
constexpr auto range(T&& t) {
  return subset(std::forward<T>(t), make_index_sequence<(end - begin)>{} +
                                        make_constant_index_sequence<(end - begin), begin>{});
}

/// Return the head element of a sequence.
/// \param t The sequence from which to extract the head element.
/// \return The result of calling `at<0>` over the sequence.
template <typename T>
constexpr decltype(auto) head(T&& t) {
  return at<0>(std::forward<T>(t));
}

/// Return the tail subsequence of a sequence.
/// \param t The sequence from which to remove the head element.
/// \return The original sequence with the head removed.
template <typename T>
constexpr auto tail(T&& t) {
  return range<1, size_v<T>>(t);
}

template <typename I, typename T, typename O, typename... Args>
constexpr auto accumulate(I&& a, T&& t, O o, Args&&... args);

namespace detail {
template <std::size_t n>
struct accumulate_helper {
  template <typename I, typename T, typename O, typename... Args>
  static constexpr auto accumulate_internal(I&& a, T&& t, O o, Args&&... args) {
    return accumulate(o(std::forward<I>(a), head(std::forward<T>(t)), std::forward<Args>(args)...),
                      tail(std::forward<T>(t)), o, std::forward<Args>(args)...);
  }
};
template <>
struct accumulate_helper<0> {
  template <typename I, typename T, typename O, typename... Args>
  static constexpr auto accumulate_internal(I&& a, T&& t, O o, Args&&... args) {
    return a;
  }
};
}  // namespace detail

// Accumulate elements with an initial value using the given operator.
template <typename I, typename T, typename O, typename... Args>
constexpr auto accumulate(I&& a, T&& t, O o, Args&&... args) {
  return detail::accumulate_helper<size_v<T>>::accumulate_internal(
      std::forward<I>(a), std::forward<T>(t), o, std::forward<Args>(args)...);
}

// Wrap and unwrap pack types. Wrapped types can be used in contexts where we
// are worried that type instantiation can happen. The wrapped types are inert.
template <typename... T>
constexpr pack<wrap_type<T>...> wrap(pack<T...>) {
  return {};
}

template <typename... T>
constexpr pack<T...> unwrap(pack<wrap_type<T>...>) {
  return {};
}

namespace detail {
template <typename... TN>
struct integer_sequence_cat_helper;

template <typename T1, typename T2, typename... TN>
struct integer_sequence_cat_helper<T1, T2, TN...> {
  using rest_type = typename integer_sequence_cat_helper<T2, TN...>::type;
  using type = typename join_integer_sequences<T1, rest_type, false>::type;
};

template <typename T1>
struct integer_sequence_cat_helper<T1> {
  using type = T1;
};

template <typename T1, typename T2>
struct join_value_packs;

template <auto... v1, auto... v2>
struct join_value_packs<value_pack<v1...>, value_pack<v2...>> {
  using type = value_pack<v1..., v2...>;
};

template <typename... T>
struct value_pack_cat_helper;

template <typename T1, typename T2, typename... TN>
struct value_pack_cat_helper<T1, T2, TN...> {
  using rest_type = typename value_pack_cat_helper<T2, TN...>::type;
  using type = typename join_value_packs<T1, rest_type>::type;
};

template <typename T1>
struct value_pack_cat_helper<T1> {
  using type = T1;
};

template <typename... Args>
static constexpr bool all_tuples = (... && is_tuple_v<traits::remove_cvref_t<Args>>);
template <typename... Args>
static constexpr bool all_packs = (... && is_pack_v<traits::remove_cvref_t<Args>>);
template <typename... Args>
static constexpr bool all_value_packs = (... && is_value_pack_v<traits::remove_cvref_t<Args>>);
template <typename... Args>
static constexpr bool all_integer_sequences = (... &&
                                               is_integer_sequence_v<traits::remove_cvref_t<Args>>);

}  // namespace detail

// Concatenate one or more sequence types of the same kind.
template <typename... Args>
constexpr auto cat(Args&&... args) {
  static_assert(sizeof...(Args) > 0, "arpc::mpt::cat requires at least one argument");
  static_assert(detail::all_tuples<Args...> || detail::all_packs<Args...> ||
                    detail::all_value_packs<Args...> || detail::all_integer_sequences<Args...>,
                "arpc::mpt::cat requires all arguments to be of the same "
                "supported sequence type");

  if constexpr (detail::all_tuples<Args...>) {
    return std::tuple_cat(std::forward<Args>(args)...);
  } else if constexpr (detail::all_packs<Args...>) {
    return as_pack(std::tuple_cat(as_tuple(std::forward<Args>(args))...));
  } else if constexpr (detail::all_value_packs<Args...>) {
    return typename detail::value_pack_cat_helper<Args...>::type{};
  } else if constexpr (detail::all_integer_sequences<Args...>) {
    return typename detail::integer_sequence_cat_helper<Args...>::type{};
  }
}

// Check whether the passed value is v.
template <auto v>
struct is_value {
  constexpr bool operator()(const decltype(v)& w) { return w == v; }
  constexpr bool operator()(...) { return false; }
};
// Check whether the passed wrapped type is T.
template <typename T>
struct is_type {
  constexpr bool operator()(wrap_type<T> w) { return true; }
  constexpr bool operator()(...) { return false; }
};

template <typename T, bool... selected>
struct index_if;

template <typename T, bool... selected>
using index_if_t = typename index_if<T, selected...>::type;

template <>
struct index_if<index_sequence<>> {
  using type = index_sequence<>;
};

template <std::size_t idx>
struct index_if<index_sequence<idx>, true> {
  using type = index_sequence<idx>;
};

template <std::size_t idx>
struct index_if<index_sequence<idx>, false> {
  using type = index_sequence<>;
};

template <std::size_t idx1, std::size_t... idx, bool selected1, bool... selected>
struct index_if<index_sequence<idx1, idx...>, selected1, selected...> {
  using type = decltype(cat(index_if_t<index_sequence<idx1>, selected1>{},
                            index_if_t<index_sequence<idx...>, selected...>{}));
};
namespace detail {
template <typename T, typename O, typename S>
struct find_if_helper;

template <typename T, typename O, std::size_t... ints>
struct find_if_helper<T, O, index_sequence<ints...>> {
  using type = index_if_t<index_sequence<ints...>, O{}(at<ints>(T{}))...>;
};
}  // namespace detail

template <typename T, typename O>
struct find_if {
  using type = typename detail::find_if_helper<T, O, make_index_sequence<size_v<T>>>::type;
};

template <typename T, typename O>
using find_if_t = typename find_if<T, O>::type;

template <typename T, typename O>
struct find {
  using type = find_if_t<T, O>;
  static_assert((size_v<type>) > 0, "find didn't find any coincidence");
  static_assert((size_v<type>) < 2, "find found multiple coincidences");
  static constexpr std::size_t value = head(type{});
};

template <typename T, typename O>
inline constexpr std::size_t find_v = find<T, O>::value;

template <typename T, typename O>
struct filter_if {
  using type = decltype(subset(T{}, find_if_t<T, O>{}));
};

template <typename T, typename O>
using filter_if_t = typename filter_if<T, O>::type;

template <typename T, typename O>
struct count_if {
  static constexpr std::size_t value = size_v<find_if_t<T, O>>;
};

template <typename T, typename O>
inline constexpr std::size_t count_if_v = count_if<T, O>::value;

template <auto v, typename S>
struct is_value_in {
  static constexpr bool value = ((count_if_v<S, is_value<v>>) > 0);
};

template <auto v, typename S>
static constexpr bool is_value_in_v = is_value_in<v, S>::value;

template <typename T, typename S>
struct is_type_in {
  static constexpr bool value = ((count_if_v<S, is_type<T>>) > 0);
};

template <typename T, typename S>
static constexpr bool is_type_in_v = is_type_in<T, S>::value;

template <typename T, typename S>
struct insert_type_into {
  using type = std::conditional_t<is_type_in_v<T, S>, S, decltype(cat(S{}, pack<T>{}))>;
};

template <typename T, typename S>
using insert_type_into_t = typename insert_type_into<T, S>::type;

template <auto v, typename S>
struct insert_value_into {
  using type = std::conditional_t<is_value_in_v<v, S>, S, decltype(cat(S{}, value_pack<v>{}))>;
};

template <auto v, typename S>
using insert_value_into_t = typename insert_value_into<v, S>::type;

}  // namespace mpt

}  // namespace arpc

#endif  // ARPC_MPT_H_
