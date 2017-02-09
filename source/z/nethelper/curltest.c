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
    printf("%s\n", (const char*)stream);
    return size * nmemb;
}

int main(int argc, char const *argv[])
{
   char buff[1024];
   memset(buff, 0, 1024); 
   CURL * curl; 
   curl_global_init(CURL_GLOBAL_DEFAULT); 
   curl = curl_easy_init(); 
   // char * data = "hello";    // post到server的内容
   // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);          // 设置POST的方式
   curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com/");// 设置server的URL 
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);           // 设置write_data函数的最后一个参数的地址
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);  // 设置server的返回的数据的接收方式 
   curl_easy_perform(curl);
   curl_easy_cleanup(curl); 
   printf("\nbuff : %s\n", buff);

  CURLcode res;
   return 0;
}

#if 0
int main(void)
{
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com/"); 
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}
#endif

