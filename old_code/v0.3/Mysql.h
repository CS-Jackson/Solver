#pragma once
#include <iostream>
#include <string>
#include <mysql/mysql.h>


class MyDB
{
public:
    MyDB();
    ~MyDB();
    bool initDB(std::string host, std::string user, std::string pwd, std::string db_name);
    bool exeSQL(std::string sql);

private:
    MYSQL *connection;
    MYSQL_RES *result;
    MYSQL_ROW row;
};