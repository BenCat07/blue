#pragma once

#include <utility>

namespace BlueList {

// Helpers for creating a compile time list of types
// Whilst this is clumsy it is also better than doing the F1Cheat method of creating a define
// with all of the types and then iterating over them

// This works by constructing a type that inherits the previous type that inherits the previous type...
// So on and so forth, eventually resulting in Nil - the terminator type

struct Nil {};

template <typename T, typename U>
struct Cons {
    using Head = T;
    using Tail = U;
};

template <typename List, typename Reversed>
struct ListReverseHelper;

template <typename Reversed>
struct ListReverseHelper<Nil, Reversed> {
    using Type = Reversed;
};

template <typename Head, typename Tail, typename Reversed>
struct ListReverseHelper<Cons<Head, Tail>, Reversed> {
    using Type = typename ListReverseHelper<Tail, Cons<Head, Reversed>>::Type;
};

template <typename T, int N>
struct ListMakerKey : ListMakerKey<T, N - 1> {};
template <typename T>
struct ListMakerKey<T, 0> {};

#define START_LIST_(name, modifier) \
    struct name##_ListMaker {};     \
    modifier BlueList::Nil list_maker_helper_(BlueList::ListMakerKey<name##_ListMaker, __COUNTER__>);
#define ADD_TO_LIST_(name, type, modifier)                                                                               \
    modifier BlueList::Cons<type, decltype(list_maker_helper_(BlueList::ListMakerKey<name##_ListMaker, __COUNTER__>{}))> \
        list_maker_helper_(BlueList::ListMakerKey<name##_ListMaker, __COUNTER__>);
#define END_LIST(name) \
    using name = typename BlueList::ListReverseHelper<decltype(list_maker_helper_(BlueList::ListMakerKey<name##_ListMaker, __COUNTER__>{})), BlueList::Nil>::Type;

#define START_LIST(name) START_LIST_(name, static)
#define ADD_TO_LIST(name, type) ADD_TO_LIST_(name, type, static)
#define START_LIST_FUNC(name) START_LIST_(name, )
#define ADD_TO_LIST_FUNC(name, type) ADD_TO_LIST_(name, type, )

#if defined(_DEBUG)
#define FORCE_INLINE_HINT inline
#elif defined(NDEBUG)
#if blueplatform_windows()
#define FORCE_INLINE_HINT __forceinline
#endif

#endif

#define DEFINE_LIST_CALL_FUNCTION_RECURSIVE(list_name, InvokeClass, function_name)                                                                           \
    template <typename Con, typename... Args>                                                                                                                \
    FORCE_INLINE_HINT auto list_name##_call_##function_name##_recursive(Args... args) {                                                                      \
        InvokeClass<typename Con::Head>::invoke_##function_name(args...);                                                                                    \
        if constexpr (std::is_same_v<typename Con::Tail, BlueList::Nil> == false) list_name##_call_##function_name##_recursive<typename Con::Tail>(args...); \
    }                                                                                                                                                        \
    template <typename... Args>                                                                                                                              \
    FORCE_INLINE_HINT auto list_name##_call_##function_name(Args... args) {                                                                                  \
        list_name##_call_##function_name##_recursive<list_name>(args...);                                                                                    \
    }

// Helper define so that derived classes do not have to implement all the functions that the base module class does
// Here i use this terrible SFINAE becuase msvc has difficulty using std::enable_if
// (The int&& forces the compiler to check if the templated version matches first before resorting to the ... one)
// And since it is forwarded into nothing, it vanishes into thin air.
#define BLUE_CREATE_INVOKE(name)                                                                                     \
private:                                                                                                             \
    template <typename T, typename... Args, typename = decltype(T::name)>                                            \
    static auto invoke_##name##_impl(int &&, Args &&... args) { return Derived::name(std::forward<Args>(args)...); } \
    template <typename T>                                                                                            \
    static auto invoke_##name##_impl(...) {}                                                                         \
                                                                                                                     \
public:                                                                                                              \
    template <typename... Args>                                                                                      \
    static auto invoke_##name(Args... args) { return invoke_##name##_impl<Derived>(0, args...); }

} // namespace BlueList
