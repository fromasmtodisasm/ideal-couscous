#ifndef REFLECT_METADDATA_HPP
#define REFLECT_METADDATA_HPP

#include "../meta_utils/meta_utils.hpp"
#include "../reflect_information/reflect_information.hpp"

namespace reflect {

namespace utils {


template <class T>
using MetaClass = info::MetaClass<typename ::std::decay_t<typename T::MetaInfo_type>>; /**< Helper type template to specify Metadata class */

namespace detail {

template <class... Args> struct get_method_args_helper_impl {constexpr static auto value {::boost::hana::tuple_t<Args...>};};

template <class... Args> struct get_method_args_helper_impl<::boost::hana::tuple<Args...>> {constexpr static auto value {::boost::hana::tuple_t<Args...>};};

/**
 * @brief Return tuple of args type of method
 * @param index - method index
 * @return boost::hana::type_t<Tuple...> or boost::hana::type_t<boost::hana::optional<>> if out-of range, class is not reflected or Generator is not a generator class
 */
template<class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype (auto) get_method_args_helper_method_impl(I&& index) {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>)) {
        if constexpr (::std::decay_t<decltype(::boost::hana::size(metautils::copy_tuple_sequence(MetaClass<T>::metadata,Generator::template generate<decltype(MetaClass<T>::metadata)>())))>::value >
                      ::std::decay_t<I>::value)
            return ::boost::hana::type_c<typename ::std::decay_t<decltype(::boost::hana::at(metautils::copy_tuple_sequence(MetaClass<T>::metadata,
                                         Generator::template generate<decltype(MetaClass<T>::metadata)>()),index))>::arg_types>;
        else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
    }
    else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
}


}

/**
 * @brief Returns class name
 * @return ct-string (boost::hana::string) or boost::hana::optional<> if out-of range, class is not reflected
 */
template <class T>
constexpr decltype (auto) get_class_name() {
    if constexpr (info::is_reflected_v<::std::decay_t<T>>) return info::MetaClass<typename ::std::decay_t<typename T::MetaInfo_type> >::class_name;
    else return ::boost::hana::nothing;
}

/**
 * @brief Returns amount of elements generated by the generator
 * @return boost::hana::size_t or boost::hana::optional<> if out-of range, class is not reflected or Generator is not a generator class
 */
template <class T, class Generator = info::DefaultIndexGenerator>
constexpr decltype (auto) get_count() {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>))
    //Forcing unevaluated context to not interact with data(only with types)
        return boost::hana::size_c<decltype(::boost::hana::size(metautils::copy_tuple_sequence(MetaClass<T>::metadata,Generator::template generate<decltype(MetaClass<T>::metadata)>())))::value>;
    else return ::boost::hana::nothing;
}

/**
 * @brief Get name of element
 * @param index - index of number(boost::hana::size_t)
 * @return ct-string (boost::hana::string) or boost::hana::optional<> if out-of range, class is not reflected or Generator is not a generator class
 */
template <class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype (auto) get_name (I&& index) {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>)) {
        if constexpr (decltype(::boost::hana::size(metautils::copy_tuple_sequence(MetaClass<T>::names,Generator::template generate<decltype(MetaClass<T>::metadata)>())))::value >
         ::std::decay_t<I>::value) return ::boost::hana::at(metautils::copy_tuple_sequence(MetaClass<T>::names,Generator::template generate<decltype(MetaClass<T>::metadata)>()),index);
        else return ::boost::hana::nothing;
    }
    else return ::boost::hana::nothing;
}

/**
 * @brief Find element index by name
 * @param str - ct-string (boost::hana::string)
 * @return boost::hana::size_t or boost::hana::optional<> if out-of range, class is not reflected or Generator is not a generator class
 */
template <class T, class Generator = info::DefaultIndexGenerator, class String>
constexpr decltype (auto) find_name(String&& str) {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>))
        return metautils::find_value_types(str,metautils::copy_tuple_sequence(MetaClass<T>::names,Generator::template generate<decltype(MetaClass<T>::metadata)>()));
    else return ::boost::hana::nothing;
}

/**
 * @brief Return args types of method
 * @param index - method index
 * @return boost::hana::tuple_t<Types...> or boost::hana::type_t<boost::hana::optional<>> if out-of range, class is not reflected or Generator is not a generator class
 */
template<class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype (auto) get_method_args(I&& index) {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>)) {
        if constexpr (::std::decay_t<decltype(::boost::hana::size(metautils::copy_tuple_sequence(MetaClass<T>::metadata,Generator::template generate<decltype(MetaClass<T>::metadata)>())))>::value >
                      ::std::decay_t<I>::value)
            return detail::get_method_args_helper_impl<typename ::std::decay_t<decltype(::boost::hana::at(metautils::copy_tuple_sequence(MetaClass<T>::metadata,
                                                       Generator::template generate<decltype(MetaClass<T>::metadata)>()),index))>::arg_types>::value;
        else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
    }
    else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
}

/**
 * @brief Returns amount of method args
 * @param index - method index
 * @return boost::hana::size_t<...> or boost::hana::optional<> if out-of range, class is not reflected or Generator is not a generator class
 */
template<class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype (auto) get_methods_args_count (I&& index) {
    if constexpr (!::std::is_same<decltype (get_method_args<T,Generator>(::std::forward<I>(index))),decltype (::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>)>::value) {
        return ::boost::hana::size(get_method_args<T,Generator>(index));
    } else return ::boost::hana::nothing;
}

/**
 * @brief Returns result type of method
 * @param index - method index
 * @return boost::hana::type_t<...> or boost::hana::type_t<boost::hana::optional<>> if out-of range, class is not reflected or Generator is not a generator class
 */
template<class T, class Generator = info::DefaultIndexGenerator, class I>
constexpr decltype (auto) get_method_result_type(I&& index) {
    if constexpr ((info::is_reflected_v<::std::decay_t<T>>) && (info::is_generator_v<::std::decay_t<Generator>>)) {
        if constexpr(::std::decay_t<decltype(::boost::hana::size(metautils::copy_tuple_sequence(MetaClass<T>::metadata,Generator::template generate<decltype(MetaClass<T>::metadata)>())))>::value >
                     ::std::decay_t<I>::value)
            return ::boost::hana::type_c<typename ::std::decay_t<decltype(::boost::hana::at(metautils::copy_tuple_sequence(MetaClass<T>::metadata,
                                         Generator::template generate<decltype(MetaClass<T>::metadata)>()),index))>::return_type>;
        else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
    }
    else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
}

/**
 * @brief Returns Jth argument type of method
 * @param index1 - method index
 * @param index2 - argument index
 * @return boost::hana::type_t<...> or boost::hana::type_t<boost::hana::optional<>> if out-of range, class is not reflected or Generator is not a generator class
 *
 */
template<class T, class Generator = info::DefaultIndexGenerator, class I, class J>
constexpr decltype (auto) get_method_arg(I&& index1, J&& index2) {
    if constexpr (!::std::is_same<decltype (get_method_args<T,Generator>(::std::forward<I>(index1))),decltype (::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>)>::value) {
        if constexpr (decltype (::boost::hana::size(get_method_args<T,Generator>(::std::forward<I>(index1))))::value > ::std::decay_t<J>::value)
            return ::boost::hana::at(get_method_args<T,Generator>(::std::forward<I>(index1)),index2);
        else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
    }
    else return ::boost::hana::type_c<::std::decay_t<decltype(::boost::hana::nothing)>>;
}

template<class T, class Generator = info::DefaultIndexGenerator, class IndexTuple>
constexpr decltype (auto) check_invoke() {

}

}

}

#endif // REFLECT_METADDATA_HPP
