#include <iostream>
#include <string>

using namespace std;

int main()
{
    string s = "item.cgi";
    int dot_pos = s.find('.');
    string a = s.substr(dot_pos);
    cout << a << endl;
    return 0;
}