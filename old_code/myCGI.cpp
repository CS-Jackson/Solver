#include <iostream>
using namespace std;
 
int main ()
{
    
//    cout << "Content-type:text/html\r\n\r\n";
//    cout << "<html>\n";
//    cout << "<head>\n";
//    cout << "<title>Hello World - 第一个 CGI 程序</title>\n";
//    cout << "</head>\n";
//    cout << "<body>\n";
//    cout << "<h2>Hello World! 这是我的第一个 CGI 程序</h2>\n";
//    cout << "</body>\n";
//    cout << "</html>\n";
    std::cout << "Content-type:text/html;charset=utf-8\r\n\r\n";
    std::cout << "<!DOCTYPE html>";
    std::cout << "<meta charset='utf-8'/>";
    std::cout << "<title>Hello www.twle.cn - 我的 第一个 CGI 程序</title>";
    std::cout << "<h2>Hello www.twle.cn - 我的 第一个 CGI 程序</h2>";

    return 0;
}