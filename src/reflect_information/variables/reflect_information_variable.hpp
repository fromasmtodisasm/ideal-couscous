#ifndef REFL_INFORMATION_VARIABLE_HPP
#define REFL_INFORMATION_VARIABLE_HPP

#include "../../meta_utils/meta_utils.hpp"
#include "../types/reflect_information_object.hpp"
#include "../types/reflect_information_static.hpp"

namespace reflect {

namespace info {

/**
 * @brief SFINAE check if type is pointer to variable
 */
constexpr auto is_variable = ::boost::hana::is_valid(
    [](auto &&p) -> decltype(&::std::decay_t<decltype(p)>::is_variable) {});

template <class T>
constexpr bool is_variable_v =
    decltype(is_variable(::std::declval<T>()))::value; /**< Helper variable
                                                          template for
                                                          is_variable */

namespace detail {}

/**
 * @brief The VariableIndexGenerator class - generate indices where vars are
 * located
 */
template <class ParentGenerator, bool condition = true>
class VariableIndexGenerator final {
  template <class Item, long long Index>
  constexpr static decltype(auto) check_metadata_variable() {
    if constexpr (is_variable_v<::std::decay_t<Item>> == condition)
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
  using reverse = VariableIndexGenerator<ParentGenerator,
                                         !condition>; /**< Reverse generator */

  /**
   * @brief generate function
   * @return ::boost::hana::tuple of indices
   */
  template <class Tuple> constexpr static decltype(auto) generate() {
    return generate_impl<Tuple>(ParentGenerator::template generate<Tuple>());
  }
};

/**
 * @brief Pointer to object variable container
 *
 */
template <class Result, class Obj> class obj_var_t final {
  Result Obj::*p; /**< Pointer to object variable */
public:
  using obj_type = Obj; /**< Object type of pointer */

  using type = Result Obj::*; /**< Pointer type */

  using arg_types = ::std::decay_t<decltype(::boost::hana::tuple_t<Obj>)>; /**<
                                                     Tuple pointer type (Needed
                                                     to unify all
                                                     pointer structs) */

  using return_type = decltype(metautils::constexpr_invoke(
      std::declval<type>(), std::declval<obj_type>())); /**< Return type */

  /**
   * @brief method that indicates that the pointer - variable
   *
   * @return std::true_type
   */
  static constexpr auto is_variable() { return std::true_type(); }

  /**
   * @brief method that indicates that the pointer - pointer to object variable
   *
   * @return std::true_type
   */
  static constexpr auto is_object() { return std::true_type(); }

  /**
   * @brief Constexpr constructor
   *
   * @param p object pointer
   */
  explicit constexpr obj_var_t(type p) noexcept : p(p) {}

  /**
   * @brief Invoke operator
   *
   * @param obj member object
   * @return utils::constexpr_invoke object of obj_var_t::return_type
   */
  constexpr auto operator()(obj_type &&obj)
      -> decltype(metautils::constexpr_invoke(p, obj)) const {
    return metautils::constexpr_invoke(p, obj);
  }

  /**
   * @brief Invoke operator
   *
   * @param obj member object
   * @return utils::constexpr_invoke object of obj_var_t::return_type
   */
  constexpr auto operator()(const obj_type &obj)
      -> decltype(metautils::constexpr_invoke(p, obj)) const {
    return metautils::constexpr_invoke(p, obj);
  }

  /**
   * @brief Invoke operator
   *
   * @param obj member object
   * @return utils::constexpr_invoke object of obj_var_t::return_type
   */
  constexpr auto operator()(obj_type &obj)
      -> decltype(metautils::constexpr_invoke(p, obj)) const {
    return metautils::constexpr_invoke(p, obj);
  }
};

/**
 * @brief Pointer to a static variable container
 *
 */
template <class Obj_class, class Obj> class static_var_t final {
  Obj *p; /**< Pointer to a static variable */
public:
  using obj_type = Obj_class; /**< Object type of pointer to object variable */

  using type = Obj *; /**< Pointer type */

  using arg_types = ::std::decay_t<decltype(
      ::boost::hana::tuple_t<>)>; /**< Tuple pointer type (Needed to
        unify all pointer structs) */

  using return_type = std::add_rvalue_reference_t<
      std::remove_reference_t<decltype(*p)>>; /**< Object of pointer type */

  /**
   * @brief method that indicates that the pointer - variable
   *
   * @return std::true_type
   */
  static constexpr auto is_variable() { return std::true_type(); }

  /**
   * @brief method that indicates that the pointer - static
   *
   * @return std::true_type
   */
  static constexpr auto is_static() { return std::true_type(); }

  /**
   * @brief Constexpr constructor
   *
   * @param p pointer
   */
  explicit constexpr static_var_t(type p) noexcept : p(p) {}

  /**
   * @brief Dereference of pointer (if class object is not provided)
   *
   * @return object of static_var_t::return_type
   */
  constexpr auto operator()() -> decltype(*p) const { return *p; }

  /**
   * @brief Dereference of pointer (if class object is provided)
   *
   * @return object of static_var_t::return_type
   */
  template <class T> constexpr auto operator()(T &&) -> decltype(*p) const {
    return *p;
  }
};

/**
 * @brief Create obj_var_t
 *
 * @param pm pointer to object variable
 * @return mem_fn_t<R, T>
 */
template <class R, class T> constexpr decltype(auto) make_var(R T::*pm) {
  return obj_var_t<R, T>(pm);
}

/**
 * @brief Create static_var_t
 *
 * @param pm pointer
 * @return static_var_t<T>
 */
template <class R, class T> constexpr decltype(auto) make_var(T *pm) {
  return static_var_t<R, T>(pm);
}
} // namespace info
} // namespace reflect

/**
 * @brief Reflect object variable. Appends to names_state name of var, to
 * metadata_state pointer to variable and increase counter
 */
#define REFLECT_OBJ_VAR(NAME)                                                  \
  TUPLE_APPEND(names_state, counter, HANA_STR(#NAME))                          \
  TUPLE_APPEND(metadata_state, counter,                                        \
               ::reflect::info::make_var(&Type::NAME))                         \
  INCREASE_COUNTER(counter)

/**
 * @brief Reflect static variable. Appends to names_state name of var, to
 * metadata_state pointer to variable and increase counter
 */
#define REFLECT_STATIC_VAR(NAME)                                               \
  TUPLE_APPEND(names_state, counter, HANA_STR(#NAME))                          \
  TUPLE_APPEND(metadata_state, counter,                                        \
               ::reflect::info::make_var<Type>(&Type::NAME))                   \
  INCREASE_COUNTER(counter)

#endif // REFLECT_INFORMATION_VARIABLE_HPP
