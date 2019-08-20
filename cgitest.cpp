/**
 * file: index.cpp
 * author: 简单教程(www.twle.cn)
 *
 * Copyright © 2015-2065 www.twle.cn. All rights reserved.
 */

#include <iostream>

#include <cgicc/CgiDefs.h> 
#include <cgicc/Cgicc.h> 
#include <cgicc/HTTPHTMLHeader.h> 
#include <cgicc/HTMLClasses.h>  

using namespace cgicc;

int main ()
{
    std::cout << "Content-type:text/html;charset=utf-8\r\n\r\n";
    std::cout << "<!DOCTYPE html>\n";
    std::cout << "<html>\n";
    std::cout << "<head>\n";
    std::cout << "<title>Hello World - 第一个 CGI 程序</title>\n";
    std::cout << "</head>\n";
    std::cout << "<body>\n";
    std::cout << "<h2>Hello World! 这是我的第一个 CGI 程序</h2>\n";
    std::cout << "</body>\n";
    std::cout << "</html>\n";

    // Cgicc formData;

    // std::cout << "Content-type:text/html;charset=utf-8\r\n\r\n";
    // std::cout << "<!DOCTYPE html>\n";
    // std::cout << "<html>\n";  
    // std::cout << "<head>\n";
    // std::cout << "<title>Using GET pass the paramters</title>\n";
    // std::cout << "</head>\n";
    // std::cout << "<body>\n";  
    // form_iterator fi = formData.getElement("userbane");  
    // if( !fi->isEmpty() && fi != (*formData).end()) {  
    //     std::cout << "username:" << **fi << std::endl;  
    // }  
    // std::cout << "<br/>\n";
    // std::cout << "</body>\n";
    // std::cout << "</html>\n";
    // std::cout << "<br/>\n";

    // fi = formData.getElement("id");  
    // if( !fi->isEmpty() && fi != (*formData).end()) {  
    //     std::cout << "id:" << **fi << std::endl;  
    // }

    // std::cout << "<br/>\n";

    // fi = formData.getElement("cm_id");  
    // if( !fi->isEmpty() && fi != (*formData).end()) {  
    //     std::cout << "cm_id:" << **fi << std::endl;  
    // }

    // std::cout << "<br/>\n";

    // fi = formData.getElement("abbucket");  
    // if( !fi->isEmpty() && fi != (*formData).end()) {  
    //     std::cout << "abbucket:" << **fi << std::endl;  
    // }

   return 0;
}