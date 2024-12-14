/*
   Simple manual test for the AVL_tree library.
*/
#include <iostream>
using namespace std;

#include "AVL_tree.hpp"

typedef pair<uint16_t, uint16_t> dos;

int main()
{
    char chars1[] = { 'k',
        'd',
        'r',
        'd',
        'e',
        'f',
        'z',
        's',
        'e',
        'i',
        'w',
        'l',
        'm',
        'n',
        'b',
        'a' };

    char chars2[] = { 'A', 'N', '8', 'Y' };
    AVL_tree<char> tree { chars1[0] };

    for (size_t i = 1; i < sizeof(chars1) / sizeof(char); ++i) {
        tree.add(chars1[i]);
        tree.print_tree();
        cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
             << endl;
    }

    for (size_t i = 0; i < sizeof(chars1) / sizeof(char); ++i) {
        if (!tree.is_inside(chars1[i])) {
            cout << "character " << chars1[i] << " was not found!" << endl;
            return 1;
        }
    }

    for (size_t i = 0; i < sizeof(chars2) / sizeof(char); ++i) {
        if (tree.is_inside(chars2[i])) {
            cout << "character " << chars2[i] << " was found!" << endl;
            return 1;
        }
    }

    auto status = tree.remove('.');
    if (avl_statuses::VALUE_NOT_FOUND != status) {
        cout << "remove function didnt fail correctly!" << endl;
        return 1;
    }

    for (size_t i = 0; i < sizeof(chars1) / sizeof(char); ++i) {
        tree.remove(chars1[i]);
        tree.print_tree();
        cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
             << endl;
    }

    return 0;
}
