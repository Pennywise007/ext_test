// checking detail functions

#include "pch.h"

#include <ext/utils/constructor_traits.h>

struct DetailTestDefConstructor
{
    DetailTestDefConstructor() = default;
    DetailTestDefConstructor(const DetailTestDefConstructor&) = default;
    DetailTestDefConstructor(DetailTestDefConstructor&&) = default;
};
static_assert(ext::detail::constructor_size<DetailTestDefConstructor> == 0, "Failed to determine default constructor size");

struct DetailTestOneArgumentConstructor
{
    DetailTestOneArgumentConstructor(int _val) : val(_val) {}
    DetailTestOneArgumentConstructor(const DetailTestOneArgumentConstructor&) = default;
    DetailTestOneArgumentConstructor(DetailTestOneArgumentConstructor&&) = default;
    int val;
};
static_assert(ext::detail::constructor_size<DetailTestOneArgumentConstructor> == 1, "Failed to determine one argument constructor size");

struct DetailTestMultipleArgumentConstructor
{
    DetailTestMultipleArgumentConstructor(int _val, long, bool) : val(_val) {}
    DetailTestMultipleArgumentConstructor(const DetailTestMultipleArgumentConstructor&) = default;
    DetailTestMultipleArgumentConstructor(DetailTestMultipleArgumentConstructor&&) = default;
    int val;
};
static_assert(ext::detail::constructor_size<DetailTestMultipleArgumentConstructor> == 3, "Failed to determine multiple argument constructor size");
