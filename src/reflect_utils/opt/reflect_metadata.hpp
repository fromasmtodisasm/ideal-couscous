#ifndef REFLECT_METADDATA_HPP
#define REFLECT_METADDATA_HPP

#include <boost/hana/greater.hpp>
#include <boost/hana/greater_equal.hpp>

#include "../../meta_utils/meta_utils.hpp"
#include "../../reflect_information/reflect_information.hpp"

namespace reflect {

namespace utils {

template <class T>
using MetaClass = info::MetaClass<typename ::std::decay_t<
    T>::MetaInfo_type>; /**< Helper type template to specify Metadata class */

template <class T, class Generator = info::DefaultIndexGenerator, class... Args,
          class I>
constexpr decltype(auto) check_invoke(I &&);

namespace detail {

/**
 * @brief Helper struct to get tuple of types(boost::hana::tuple_t)
 */
template <class... Args> struct method_args_helper_impl {
  constexpr static auto value{::boost::hana::tuple_t<Args...>};
};

/**
 * @brief Helper struct to get tuple of types(boost::hana::tuple_t) (template
 * spetialization for boost::hana::tuple)
 */
template <class... Args>
struct method_args_helper_impl<::boost::hana::tuple<Args...>> {
  constexpr static auto value{::boost::hana::tuple_t<Args...>};
};

/**
 * @brief Return tuple of args type of method
 * @param index - method index
 * @return boost::hana::type_t<Tuple...> or
 * boost::hana::type_t<boost::hana::optional<>> if out-of range, class is not
 * reflected or Generator is not a generator class
 */
template <class T, class Generator, class I>
constexpr decltype(auto) method_args_helper_method_impl(I &&index) {
  if
    constexpr((info::is_reflected_v<::std::decay_t<T>>)&&(
        info::is_generator_v<::std::decay_t<Generator>>)) {
      if
        constexpr(::std::decay_t<decltype(
                      ::boost::hana::size(metautils::copy_tuple_sequence(
                          MetaClass<T>::metadata,
                          Generator::template generate<decltype(
                              MetaClass<T>::metadata)>())))>::value >
                  ::std::decay_t<I>::value) return ::boost::hana::
            type_c<typename ::std::decay_t<decltype(
                ::boost::hana::at(metautils::copy_tuple_sequence(
                                      MetaClass<T>::metadata,
                                      Generator::template generate<decltype(
                                          MetaClass<T>::metadata)>()),
                                  index))>::arg_types>;
      else
        return ::boost::hana::type_c<
            ::std::decay_t<decltype(::boost::hana::nothing)>>;
    }
  else
    return ::boost::hana::type_c<
        ::std::decay_t<decltype(::boost::hana::nothing)>>;
}

/**
 * @brief SFINAE check whether we can call operator () with given types
 */
template <class T, class... Args> struct is_invocable {
  template <class C>
  static constexpr ::std::true_type
  check(::std::decay_t<decltype(::std::declval<C>()(::std::declval<Args>()...))>
            *);
  template <class> static constexpr ::std::false_type check(...);
  static constexpr bool value =
      ::std::is_same<::std::true_type, decltype(check<T>(nullptr))>::value;
};

template <class... Args>
constexpr bool is_invocable_v = is_invocable<Args...>::value; /**< Helper
                                                                 variable
                                                                 template for
                                                                 is_invocable */

/**
 * @brief Struct checker if return value of invocation is assignable
 */
template <class T, class SetArg, class... Args> struct is_invoke_assignable {
  static constexpr bool value = ::std::is_assignable<
      decltype(::std::declval<T>()(::std::declval<Args>()...)), SetArg>::value;
};

template <class... Args>
constexpr bool is_invoke_assignable_v =
    is_invoke_assignable<Args...>::value; /**< Helper variable template for
                                             is_invoke_assignable */

/**
 * @brief Implementation of check_invoke
 * @return boost::hana::bool_c<true/false>
 */
template <class... Args> constexpr decltype(auto) check_invoke_impl() {
  if
    constexpr(is_invocable_v<Args...>) return ::boost::hana::bool_c<true>;
  else
    return ::boost::hana::bool_c<false>;
}

/**
 * @brief Implementation of check_set
 * @return boost::hana::bool_c<true/false>
 */
template <class T, class SetArg, class... Args>
constexpr decltype(auto) check_set_impl() {
  if
    constexpr(is_invocable_v<T, Args...>) {
      if
        constexpr(
            is_invoke_assignable_v<T, SetArg,
                                   Args...>) return ::boost::hana::bool_c<true>;
      else
        return ::boost::hana::bool_c<false>;
    }
  else
    return ::boost::hana::bool_c<false>;
}
}

/**
 * @brief Check if class is reflected
 * @return boost::hana::bool_c<true/false>
 */
template <class T> constexpr decltype(auto) check_reflected() {
  return ::boost::hana::bool_c<info::is_reflected_v<::std::decay_t<T>>>;
}

/**
 * @brief Returns class name
 * @return boost::hana::optional<...> of ct-string or boost::hana::nothing if
 * error happens
 */
template <class T> constexpr decltype(auto) class_name() {
  if
    constexpr(decltype(check_reflected<T>())::value) return ::boost::hana::just(
        MetaClass<T>::class_name);
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Returns amount of elements generated by the generator
 * @return boost::hana::optional<...> of boost::llong or
 * boost::hana::nothing if error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator>
constexpr decltype(auto) count() {
  if
    constexpr((decltype(check_reflected<T>())::value) &&
              (info::is_generator_v<::std::decay_t<Generator>>))
        // Forcing unevaluated context to not interact with data(only with
        // types)
        return ::boost::hana::just(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::metadata,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>);
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Get name of element
 * @param index - index(boost::hana::llong)
 * @return boost::hana::optional<...> of ct-string or boost::hana::nothing if
 * error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype(auto) member_name(I &&index) {
  if
    constexpr(
        (decltype(check_reflected<T>())::value) &&
        (info::is_generator_v<::std::decay_t<Generator>>)&&(
            ::std::is_same<::boost::hana::integral_constant_tag<long long>,
                           ::boost::hana::tag_of_t<I>>::value)) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::names,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>,
            index))::value) return ::boost::hana::
            just(::boost::hana::at(
                metautils::copy_tuple_sequence(
                    MetaClass<T>::names, Generator::template generate<decltype(
                                             MetaClass<T>::metadata)>()),
                index));
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Find element index by name
 * @param str - ct-string (boost::hana::string)
 * @return boost::hana::optional<...> of boost::hana::tuple_c<long long,...>
 * or
 * boost::hana::nothing if error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class String>
constexpr decltype(auto) find_by_name(String &&str) {
  if
    constexpr(
        (decltype(check_reflected<T>())::value) &&
        (info::is_generator_v<::std::decay_t<Generator>>)&&(
            ::std::is_same<::boost::hana::string_tag,
                           ::boost::hana::tag_of_t<String>>::value)) return ::
        boost::hana::just(metautils::find_value_types(
            str, metautils::copy_tuple_sequence(
                     MetaClass<T>::names, Generator::template generate<decltype(
                                              MetaClass<T>::metadata)>())));
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Find first element index by name
 * @param str - ct-string (boost::hana::string)
 * @return boost::hana::optional<...> of long long or boost::hana::nothing if
 * error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class String>
constexpr decltype(auto) find_by_name_first(String &&str) {
  if
    constexpr(::boost::hana::transform(find_by_name<T, Generator>(str),
                                       ::boost::hana::size) >
              ::boost::hana::just(::boost::hana::size_c<0>)) {
      return ::boost::hana::just(
          ::boost::hana::at_c<0>(find_by_name<T, Generator>(str).value()));
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Return args types of method
 * @param index - method index(boost::llong)
 * @return boost::hana::optional<...> of boost::hana::tuple_t<Types...> or
 * boost::hana::nothing if error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype(auto) method_args(I &&index) {
  if
    constexpr(
        (decltype(check_reflected<T>())::value) &&
        (info::is_generator_v<::std::decay_t<Generator>>)&&(
            ::std::is_same<::boost::hana::integral_constant_tag<long long>,
                           ::boost::hana::tag_of_t<I>>::value)) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::metadata,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>,
            index))::value) return ::boost::hana::
            just(detail::method_args_helper_impl<
                 typename ::std::decay_t<decltype(::boost::hana::at(
                     metautils::copy_tuple_sequence(
                         MetaClass<T>::metadata,
                         Generator::template generate<decltype(
                             MetaClass<T>::metadata)>()),
                     index))>::arg_types>::value);
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Returns amount of method args
 * @param index - method index(boost::hana::llong)
 * @return boost::hana::optional<...> of boost::hana::llong<...> or
 * boost::hana::nothing if error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype(auto) methods_args_count(I &&index) {
  return ::boost::hana::transform(method_args<T, Generator>(index),
                                  ::boost::hana::size);
}

/**
 * @brief Returns result type of method/variable
 * @param index - method index(boost::hana::llong)
 * @return boost::hana::optional<...> of boost::hana::type_t<...> or
 * boost::hana::nothing if error happens
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype(auto) result_type(I &&index) {
  if
    constexpr(
        (decltype(check_reflected<T>())::value) &&
        (info::is_generator_v<::std::decay_t<Generator>>)&&(
            ::std::is_same<::boost::hana::integral_constant_tag<long long>,
                           ::boost::hana::tag_of_t<I>>::value)) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::metadata,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>,
            index))::value) return ::boost::hana::
            just(
                ::boost::hana::type_c<typename ::std::decay_t<decltype(
                    ::boost::hana::at(metautils::copy_tuple_sequence(
                                          MetaClass<T>::metadata,
                                          Generator::template generate<decltype(
                                              MetaClass<T>::metadata)>()),
                                      index))>::return_type>);
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Returns N-th argument type of method
 * @param index1 - method index(boost::hana::llong)
 * @param index2 - argument index(boost::hana::llong)
 * @return boost::hana::optional<...> of boost::hana::type_t<...> or
 * boost::hana::nothing if error happens
 *
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I,
          class J>
constexpr decltype(auto) method_arg(I &&index1, J &&index2) {
  if
    constexpr(
        ::std::is_same<::boost::hana::integral_constant_tag<long long>,
                       ::boost::hana::tag_of_t<::std::decay_t<J>>>::value) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::transform(
                method_args<T, Generator>(::std::forward<I>(index1)),
                ::boost::hana::size),
            ::boost::hana::just(::boost::hana::size_c<
                                ::std::decay_t<J>::value>)))::value) return ::
            boost::hana::just(::boost::hana::at(
                method_args<T, Generator>(::std::forward<I>(index1)).value(),
                index2));
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Checks whether method can be invoked
 * @param index - index(boost::hana::llong)
 * @return boost::hana::optional<...> of boost::hana::type_c<bool,...> (true or
 * false if method can be invoked) or boost::hana::nothing if error happens
 */
template <class T, class Generator, class... Args, class I>
constexpr decltype(auto) check_invoke(I &&index) {
  if
    constexpr((decltype(check_reflected<T>())::value) &&
              (info::is_generator_v<::std::decay_t<Generator>>)&&(
                  ::std::is_same<
                      ::boost::hana::integral_constant_tag<long long>,
                      ::boost::hana::tag_of_t<::std::decay_t<I>>>::value)) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::metadata,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>,
            index))::value) return ::boost::hana::
            just(detail::check_invoke_impl<
                 ::std::decay_t<decltype(::boost::hana::at(
                     metautils::copy_tuple_sequence(
                         MetaClass<T>::metadata,
                         Generator::template generate<decltype(
                             MetaClass<T>::metadata)>()),
                     index))>,
                 Args...>());
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}

/**
 * @brief Check if we can assign to return value(or value) of method/variable
 * @param index - index(boost::hana::llong)
 * @return boost::hana::optional<...> of boost::hana::type_c<bool,...> (true or
 * false if method can be invoked) or boost::hana::nothing if error happens
 */
template <class T, class Generator, class SetArg, class... Args, class I>
constexpr decltype(auto) check_set(I &&index) {
  if
    constexpr((decltype(check_reflected<T>())::value) &&
              (info::is_generator_v<::std::decay_t<Generator>>)&&(
                  ::std::is_same<
                      ::boost::hana::integral_constant_tag<long long>,
                      ::boost::hana::tag_of_t<::std::decay_t<I>>>::value)) {
      if
        constexpr(decltype(::boost::hana::greater(
            ::boost::hana::llong_c<decltype(
                ::boost::hana::size(metautils::copy_tuple_sequence(
                    MetaClass<T>::metadata,
                    Generator::template generate<decltype(
                        MetaClass<T>::metadata)>())))::value>,
            index))::value) return ::boost::hana::
            just(detail::check_set_impl<
                 ::std::decay_t<decltype(::boost::hana::at(
                     metautils::copy_tuple_sequence(
                         MetaClass<T>::metadata,
                         Generator::template generate<decltype(
                             MetaClass<T>::metadata)>()),
                     index))>,
                 SetArg, Args...>());
      else
        return ::boost::hana::nothing;
    }
  else
    return ::boost::hana::nothing;
}
}
}

#endif // REFLECT_METADDATA_HPP