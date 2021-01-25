/**
 * This is a helper class to handle communication to Domoticz
 *
 * September 2018, Karl Johan Lisby (kjlisby @ Github)
 */
#ifndef GETREQUEST_H
#define GETREQUEST_H

#include <curl/curl.h>


struct MemoryStruct {
  char *memory;
  size_t size;
};


class GetRequest
{
    public:
        GetRequest(char *URL);
        virtual ~GetRequest();
        char *getResponse();
    protected:
    private:
        CURL *curl_handle;
        CURLcode res;
        struct MemoryStruct chunk;
};

#endif // GETREQUEST_H
