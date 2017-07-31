
#ifndef API_FUNCTIONAL_HPP_INCLUDED
#define API_FUNCTIONAL_HPP_INCLUDED
namespace api {

#include <utility>
#include <tuple>

// namespace api{
//     namespace detail {
//         template <class T>
//         struct is_reference_wrapper : std::false_type {};
//         template <class U>
//         struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};
//         template <class T>
//         constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

//         template <typename T, typename EnableT>
//         struct 
//         template <class T, class Type, class T1, class... Args>
//         decltype(auto) INVOKE(Type T::* f, T1&& t1, Args&&... args)
//         {
//             if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
//                     if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
//                                      return (std::forward<T1>(t1).*f)(std::forward<Args>(args)...);
//                     else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
//                                           return (t1.get().*f)(std::forward<Args>(args)...);
//                     else
//                         return ((*std::forward<T1>(t1)).*f)(std::forward<Args>(args)...);
//             }
//             else {
//                 static_assert(std::is_member_object_pointer_v<decltype(f)>,"");
//                 static_assert(sizeof...(args) == 0,"");
//                 if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
//                                  return std::forward<T1>(t1).*f;
//                 else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
//                                       return t1.get().*f;
//                 else
//                     return (*std::forward<T1>(t1)).*f;
//             }
//         }

//         template <class F, class... Args>
//         decltype(auto) INVOKE(F&& f, Args&&... args)
//         {
//             return std::forward<F>(f)(std::forward<Args>(args)...);
//         }
//     } // namespace detail

//     template< class F, class... Args>
//     std::invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
//         // noexcept(std::is_nothrow_invocable_v<F, Args...>)
//     {
//         return detail::INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
//     }
// } // namespace api

    namespace detail {
        template <class F, class Tuple, std::size_t... I>
        constexpr decltype(auto) apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>) 
        {
            return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
        }
    }  // namespace detail

    template <class F, class Tuple>
    constexpr decltype(auto) apply(F &&f, Tuple &&t) 
    {
        return detail::apply_impl(
            std::forward<F>(f), std::forward<Tuple>(t),
            std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
    }

} // namespace api
#endif // API_FUNCTIONAL_HPP_INCLUDED
