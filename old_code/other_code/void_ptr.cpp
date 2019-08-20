#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//undefine, dangerous.
// void printfvoid(void *p)
// {
//     p = (int*)p;
//     while(p != nullptr) {
//         cout << *(int*)p << endl;
//         p++;
//     }
    
// }

int main()
{
    vector<int> a(10);
    for(int i = 0; i < 10; i++) {
        a[i] = i;
    }
    for(int i = 0; i < 10; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
    reverse(a.begin(), a.end());
    for(int i = 0; i < 10; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
    cout << a.capacity() << endl;
    a.push_back(10);
    cout << a.capacity() << endl;
    a.reserve(40);
    cout << a.size() << endl;
    cout << a.capacity() << endl;
    a.resize(a.capacity(), 0);
    cout << a.capacity() << endl;
    cout << a.size() << endl;
    cout << a.max_size() << endl;
    return 0;
}