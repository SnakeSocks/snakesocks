#ifndef _SRC_ATOM_H
#define _SRC_ATOM_H 1

#ifdef __GNUG__
template<typename T>
concept bool NumericIncreasable = requires(T a) {
{ a + 1 } -> T;
};


template<NumericIncreasable T>
inline T InterlockedAdd(T volatile *addend, long val)
{
    return __sync_fetch_and_add(addend, val);
}

template<NumericIncreasable T>
inline T InterlockedIncrement(T volatile *addend)
{
    return InterlockedAdd(addend, 1);
}

template<NumericIncreasable T>
inline T InterlockedDecrement(T volatile *addend)
{
    return InterlockedAdd(addend, -1);
}
#endif

#endif