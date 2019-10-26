#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>

using namespace std;
int main() 
{ 
    boost::gregorian::date d(boost::gregorian::day_clock::local_day());
    cout << d.year()<<"." << d.month()<<"." <<d.day() <<endl; 
    getchar();
    return 0;
}