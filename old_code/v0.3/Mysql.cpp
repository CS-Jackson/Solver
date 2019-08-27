#include "Mysql.h"


MyDB::MyDB()
{
    connection = mysql_init(NULL);
    if(connection == NULL)
    {
        std::cout << "Error: " << mysql_error(connection);
        exit(1);
    }
}

MyDB::~MyDB()
{
    if(connection != NULL)
    {
        mysql_close(connection);
    }
}

bool MyDB::initDB(std::string host, std::string user, std::string pwd, std::string db_name)
{
    //
    connection = mysql_real_connect(connection, host.c_str(), user.c_str(), 
                                    pwd.c_str(), db_name.c_str(), 0, NULL, 0);
    if(connection == NULL)
    {
        std::cout << "Error:" << mysql_error(connection);
        exit(1);
    }
    return true;
}

bool MyDB::exeSQL(std::string sql)
{
    //query执行成功会返回0，失败返回非0。
    if(mysql_query(connection, sql.c_str()))
    {
        std::cout << "Query Error:" << mysql_error(connection);
    }
    else {
        result = mysql_use_result(connection); //获取结果集
        //field_count 返回connection查询的列数。
        for(int i = 0; i < mysql_field_count(connection); ++i)
        {
            //获取下一行。
            row = mysql_fetch_row(result);
            if(row <= 0)
            {
                break;
            }
            //返回结果集中的字段数
            for(int j = 0; j < mysql_num_fields(result); ++j)
            {
                std::cout << row[j] << " ";
            }
            std::cout << std::endl;
        }
    }
}
