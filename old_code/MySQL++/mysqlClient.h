
/*************************************************************************
    > File Name: MyDB.h
    > Author: SongLee
    > E-mail: lisong.shine@qq.com
    > Created Time: 2014年05月04日 星期日 23时25分50秒
    > Personal Blog: http://songlee24.github.io
************************************************************************/

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