/*** Includes ***/
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;

/*** Functions ***/
int main(int argc, char* argv[])
{
    int n = 5;
    int const* p = &n;

    *p = 6;

    cout << *p << endl;

    return 0;
}