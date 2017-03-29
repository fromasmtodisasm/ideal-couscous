#ifndef REFLECT_INVOKES_HPP
#define REFLECT_INVOKES_HPP

#include "../meta_utils/meta_utils.hpp"
#include "../reflect_information/reflect_information.hpp"

namespace reflect {

namespace utils {

template <class T>
using MetaClass = info::MetaClass<typename ::std::decay_t<typename T::MetaInfo_type>>; /**< Helper type template to specify Metadata class */

namespace detail {

template<class T, class... Args>
constexpr decltype (auto) invoke_pointer_impl (T&& p, Args&& ...args) {
    if constexpr (::std::decay_t<decltype(::boost::hana::size(std::declval<typename T::arg_types>()))>::value != 0) return p(::std::forward<Args>(args)...);
    else return p();
}

}

template<class T, class Generator = info::DefaultIndexGenerator, class I, class... Args>
constexpr decltype (auto) invoke (I&& index, Args&&... args) {
    if constexpr (!info::is_reflected_v<::std::decay_t<T>>) return ::boost::hana::nothing;
    if constexpr (!info::is_generator_v<::std::decay_t<Generator>>) return ::boost::hana::nothing;
    return detail::invoke_pointer_impl(::boost::hana::at(metautils::copy_tuple_sequence(MetaClass<T>::metadata,Generator::template generate<decltype(MetaClass<T>::metadata)>()),index),
                                       ::std::forward<Args>(args)...);
}

}

}

#endif // REFLECT_INVOKES_HPP