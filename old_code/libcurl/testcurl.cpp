#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    CURL *curl;             //定義CURL型別的指標
    CURLcode res;           //定義CURLcode型別的變數，儲存返回狀態碼
    if(argc!=2)
    {
        printf("Usage : file <url>;/n");
        exit(1);
    }

    curl = curl_easy_init();        //初始化一個CURL型別的指標
    if(curl!=NULL)
    {
        //設定curl選項. 其中CURLOPT_URL是讓使用者指定url. argv[1]中存放的命令列傳進來的網址
        curl_easy_setopt(curl, CURLOPT_URL, argv[1]);       
        //呼叫curl_easy_perform 執行我們的設定.並進行相關的操作. 在這裡只在螢幕上顯示出來.
        res = curl_easy_perform(curl);
        //清除curl操作.
        curl_easy_cleanup(curl);
    }
    return 0;
}