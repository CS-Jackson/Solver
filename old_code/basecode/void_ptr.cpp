#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

#include <stdio.h>
using namespace std;
typedef struct www{
    www(int fir, int sec):a(fir), b(sec){ }
    int a, b;
    // friend bool operator()(www &a, www &b) {
    //     if(a.a == b.a) return a.b > b.b;
    //     return a.a > b.a;
    // }
} int2;

struct cmp {
    bool operator()(www &a, www &b) {
        if(a.a == b.a) return a.b > b.b;
        return a.a > b.a;
    }
};

bool cmpfunc(www &a, www &b) {
    if(a.a == b.a) return a.b > b.b;
    return a.a > b.a;
}
// bool operator< (int2 &a, int2 &b)
// {
//     if(a.a == b.a) return a.b > b.b;
//     return a.a > b.a;
// }
int main()
{
    // vector<int> a(10);
    // for(int i = 0; i < 10; i++) {
    //     a[i] = i;
    // }
    // for(int i = 0; i < 10; i++) {
    //     cout << a[i] << " ";
    // }
    // cout << endl;
    // reverse(a.begin(), a.end());
    // for(int i = 0; i < 10; i++) {
    //     cout << a[i] << " ";
    // }
    // cout << endl;
    // cout << a.capacity() << endl;
    // a.push_back(10);
    // cout << a.capacity() << endl;
    // a.reserve(40);
    // cout << a.size() << endl;
    // cout << a.capacity() << endl;
    // a.resize(a.capacity(), 0);
    // cout << a.capacity() << endl;
    // cout << a.size() << endl;
    // cout << a.max_size() << endl;

    priority_queue<www, vector<www>, cmp> p1;
    //priority_queue<www> p1;
    p1.push(int2(1,2));
	p1.push(int2(3,4));
	p1.push(int2(2,8));
	p1.push(int2(5,0));
    for(int i=0;i<4;i++)
	{
		int2 temp=p1.top();p1.pop();
		printf("(%d,%d)\n",temp.a,temp.b);
	}
    vector<www> p2;
    p2.push_back(int2(1,2));
	p2.push_back(int2(3,4));
	p2.push_back(int2(2,8));
	p2.push_back(int2(5,0));
    make_heap(p2.begin(), p2.end(), cmpfunc);
    int2 temp1 = *(p2.begin());
	printf("(%d,%d)\n",temp1.a,temp1.b);
    sort(p2.begin(), p2.end(), cmpfunc);
    int2 temp2 = *(p2.begin());
	printf("(%d,%d)\n",temp2.a,temp2.b);

    priority_queue<int, vector<int>, greater<int> > pint;
    for(int i = 0; i < 10; ++i) {
        pint.push(rand() % 10);
    }
    while(!pint.empty()){
        int temp = pint.top(); pint.pop();
        cout << temp << " ";
    }
    cout << endl;
    return 0;
}