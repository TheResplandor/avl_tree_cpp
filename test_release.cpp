/*
   Simple manual test for the AVL_tree library.
*/
#include <iostream>
#include <print>
#include <tuple>
using namespace std;

#include "AVL_tree.hpp"

typedef std::tuple<bool, std::string> test_result;

// test_result test_no_comparison_class()
// {
//     class no_compare {
//     public:
//         int val;
//         auto operator<=>(const no_compare&) const = default;
//     };
//     AVL_tree<no_compare> tree {};
//     tree.add(no_compare { .val = 5 });
//     tree.add(no_compare { .val = 50 });

//     return test_result(true, __func__);
// }

test_result test_compiles_with_custom_type()
{
    class comparable {
    public:
        int val;
        auto operator<=>(const comparable&) const = default;
    };
    AVL_tree<comparable> tree { comparable { .val = 6 } };
    tree.add(comparable { .val = 5 });
    tree.add(comparable { .val = 50 });

    return test_result(true, __func__);
}

test_result test_variety()
{
    char chars1[] = {
        'k', 'd', 'r', 'd', 'e', 'f', 'z', 's', 'e', 'i', 'w', 'l', 'm', 'n', 'b', 'a'
    };

    char chars2[] = { 'A', 'N', '8', 'Y' };
    AVL_tree<char> tree { chars1[0] };

    for (size_t i = 1; i < sizeof(chars1) / sizeof(char); ++i) {
        std::println("adding {}", chars1[i]);
        tree.add(chars1[i]);
    }

    for (size_t i = 0; i < sizeof(chars1) / sizeof(char); ++i) {
        if (!tree.contains(chars1[i])) {
            cout << "character " << chars1[i] << " was not found!" << endl;
            return test_result(false, __func__);
        }
    }

    for (size_t i = 0; i < sizeof(chars2) / sizeof(char); ++i) {
        if (tree.contains(chars2[i])) {
            cout << "character " << chars2[i] << " was found!" << endl;
            return test_result(false, __func__);
        }
    }

    auto status = tree.remove('.');
    if (avl_statuses::VALUE_NOT_FOUND != status) {
        cout << "remove function didnt fail correctly!" << endl;
        return test_result(false, __func__);
    }

    for (size_t i = 0; i < sizeof(chars1) / sizeof(char); ++i) {
        std::println("removing {}", chars1[i]);
        tree.remove(chars1[i]);
    }

    return test_result(true, __func__);
}

test_result test_many_additions()
{
    AVL_tree<char> tree { '+' };
    auto addition = [&tree](char first, char last) {
        for (char i = first; i <= last; ++i) {
            std::println("adding {}", i);
            tree.add(i);
        }
        return true;
    };
    auto removal = [&tree](char first, char last) {
        for (char i = first; i <= last; ++i) {
            std::println("removing {}", i);
            tree.remove(i);
        }
        return true;
    };
    bool out = addition('a', 'z');
    if (!out) {
        return test_result(false, __func__);
    }
    out = addition('A', 'Z');
    if (!out) {
        return test_result(false, __func__);
    }
    out = addition('0', '9');
    if (!out) {
        return test_result(false, __func__);
    }
    out = removal('a', 'z');
    if (!out) {
        return test_result(false, __func__);
    }
    out = removal('A', 'Z');
    if (!out) {
        return test_result(false, __func__);
    }
    out = removal('0', '9');
    if (!out) {
        return test_result(false, __func__);
    }

    return test_result(true, __func__);
}

test_result test_add_contains()
{
    int numbers[] = { 10, 5, 9, 8, 5, 600, 700, 15 };
    AVL_tree<int> tree {};
    for (auto number : numbers) {
        tree.add(number);
    }

    for (auto number : numbers) {
        if (!tree.contains(number)) {
            return test_result(false, __func__);
        }
    }

    return test_result(true, __func__);
}

static const auto tests = {
    test_variety,
    test_many_additions,
    // test_no_comparison_class,
    test_compiles_with_custom_type,
    test_add_contains,
};

int main()
{
    for (auto test : tests) {
        auto [result, name] = test();
        if (!result) {
            std::println("FAILED: {}", name);
        } else {
            std::println("SUCCESS: {}", name);
        }
    }

    return 0;
}
