#include <iostream>
#include "boost/lexical_cast.hpp"
#include <string>


using namespace std;

int main()
{
    string s = "100";
    int a = boost::lexical_cast<int>(s);
    int b = 1;
    cout << (a + b) << endl;
    return 0;
}