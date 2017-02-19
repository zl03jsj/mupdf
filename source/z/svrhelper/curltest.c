/*
 * =====================================================================================
 *
 *       Filename:  curlTest.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2017/01/24 14时19分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdlib.h> 
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h> 

size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream)
{
    int cpsize = size * nmemb; 
    if(cpsize > 1024) 
        cpsize = 1024; 
    memcpy(stream, ptr, cpsize);
    printf("http response data-----------------\n");
    printf("%s\n", (char*)ptr);
    return size * nmemb;
}

static void
print_cookies(CURL *curl)
{
    CURLcode res;
    struct curl_slist *cookies;
    struct curl_slist *nc;
    int i;

    printf("Cookies, curl knows:\n");
    res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
    if(res != CURLE_OK) {
        fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
                curl_easy_strerror(res));
        exit(1);
    }
    nc = cookies, i = 1;
    while(nc) {
        printf("[%d]: %s\n", i, nc->data);
        nc = nc->next;
        i++;
    }
    if(i == 1) {
        printf("(none)\n");
    }
    curl_slist_free_all(cookies);
}

int rcvHeaders(void *buffer, size_t size, size_t nmemb, void *userp) {
    const char *cookiehead = "Set-Cookie: JSESSIONID=";
    int tmp = 0;
    char *sessionid = userp;
    tmp = strlen(cookiehead);
    if(0==memcmp(buffer, cookiehead, tmp)) {
        memset(sessionid, 0, 64);
        char *request_cookie_head = "Cookie: JSESSIONID=";
        tmp = strlen(request_cookie_head);
        memcpy(sessionid, request_cookie_head, tmp);
        sessionid += tmp;

        buffer += strlen(cookiehead);
        tmp = 0;

        while( ((char*)buffer)[tmp]!=';' ) {
            sessionid[tmp] = ((char*)buffer)[tmp];
            tmp++;
        }
    } 
    return size*nmemb;
}

//Host: 192.168.3.89:1986
//Connection: keep-alive
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8,en;q=0.6

int main(int argc, char const *argv[])
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));

    char sessionid[64];
    memset(sessionid, 0, sizeof(sessionid));
    char *path_cookie = "/Users/zl03jsj/Documents/pdftest/cookie.txt";

    const char *loginurl = "http://192.168.3.89:1986/ntkoSignServer/login?username=zl&password=111111";
    const char *signsurl = "http://192.168.3.89:1986/ntkoSignServer/listsigns";

    curl_global_init(CURL_GLOBAL_ALL); 
    CURL *curl = curl_easy_init(); 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_URL, loginurl); 
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, rcvHeaders);  
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, sessionid);  
//GET /ntkoSignServer/login?username=zl&password=ntko111111 HTTP/1.1
//Host: 192.168.3.89:1986
    struct curl_slist *httphead = NULL;
//    httphead = curl_slist_append(httphead, "Connection: keep-alive");
//    httphead = curl_slist_append(httphead, "Cache-Control: max-age=0");
//    httphead = curl_slist_append(httphead, "Upgrade-Insecure-Requests: 1");
//    httphead = curl_slist_append(httphead, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36");
//    httphead = curl_slist_append(httphead, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
//    httphead = curl_slist_append(httphead, "Accept-Encoding: gzip, deflate, sdch");
//    httphead = curl_slist_append(httphead, "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6"); 
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httphead);
    curl_easy_perform(curl); 
    curl_easy_cleanup(curl);
    curl_slist_free_all(httphead);
    httphead = NULL;

    printf("%s\n", sessionid);

    memset(buff, 0, sizeof(buff));
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path_cookie); 
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, path_cookie); 
    curl_easy_setopt(curl, CURLOPT_URL, signsurl); 
//GET /ntkoSignServer/listsigns HTTP/1.1
//Host: 192.168.3.89:1986
//    httphead = curl_slist_append(httphead, "Connection: keep-alive");
//    httphead = curl_slist_append(httphead, "Cache-Control: max-age=0");
//    httphead = curl_slist_append(httphead, "Upgrade-Insecure-Requests: 1");
//    httphead = curl_slist_append(httphead, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36");
//    httphead = curl_slist_append(httphead, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
//    httphead = curl_slist_append(httphead, "Accept-Encoding: gzip, deflate, sdch");
//    httphead = curl_slist_append(httphead, "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6");
    httphead = curl_slist_append(httphead, sessionid);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httphead); 

    curl_easy_perform(curl); 

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
