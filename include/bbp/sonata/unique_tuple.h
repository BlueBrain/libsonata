
// Slightly modified version of:
// https://stackoverflow.com/a/57528226
//
// We've back ported `disjunction` from cppreference, and inlined the basecase
// for `unique`.

namespace bbp {
namespace sonata {
namespace detail {

template <class...>
struct disjunction: std::false_type {};
template <class B1>
struct disjunction<B1>: B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...>: std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

template <typename... Ts>
struct unique;

template <typename... Ts>
struct unique<typename std::tuple<Ts...>> {
    using type = typename std::tuple<Ts...>;
};


template <typename... Ts, typename U, typename... Us>
struct unique<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<disjunction<std::is_same<U, Ts>...>::value,
                         unique<std::tuple<Ts...>, Us...>,
                         unique<std::tuple<Ts..., U>, Us...>> {};

template <typename... Ts>
using unique_tuple = typename unique<std::tuple<>, Ts...>::type;

}  // namespace detail
}  // namespace sonata
}  // namespace bbp
