#ifndef REFL_INFORMATION_OBJECT_HPP
#define REFL_INFORMATION_OBJECT_HPP

#include "../../meta_utils/meta_utils.hpp"

namespace reflect {

namespace info {

/**
 * @brief SFINAE check if type is pointer to object field
 */
constexpr auto is_object = ::boost::hana::is_valid(
    [](auto &&p) -> decltype(&::std::decay_t<decltype(p)>::is_object) {});

template <class T>
constexpr bool is_object_v = decltype(is_object(
    ::std::declval<T>()))::value; /**< Helper variable template for is_object */

/**
 * @brief The ObjectIndexGenerator class - generate indices where object fields
 * are located
 */
template <class ParentGenerator, bool condition = true>
class ObjectIndexGenerator final {
  template <class Item, long long Index>
  constexpr static decltype(auto) check_metadata_variable() {
    if constexpr (is_object_v<::std::decay_t<Item>> == condition)
      return ::boost::hana::make_tuple(::boost::hana::llong_c<Index>);
    else
      return ::boost::hana::make_tuple();
  }

  template <class Tuple, long long... Indices>
  constexpr static decltype(auto) generate_impl(
      const ::boost::hana::tuple<::boost::hana::llong<Indices>...> &) {
    return metautils::multiple_concat(
        check_metadata_variable<decltype(::boost::hana::at_c<Indices>(
                                    ::std::declval<Tuple>())),
                                Indices>()...);
  }

public:
  using reverse = ObjectIndexGenerator<ParentGenerator,
                                       !condition>; /**< Reverse generator */

  /**
   * @brief generate function
   * @return ::boost::hana::tuple of indices
   */
  template <class Tuple> constexpr static decltype(auto) generate() {
    return generate_impl<Tuple>(ParentGenerator::template generate<Tuple>());
  }
};
} // namespace info
} // namespace reflect

#endif // REFLECT_INFORMATION_OBJECT_HPP
