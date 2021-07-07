#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char* access_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

   // Cookie: connect.sid=s%3AnlOL8a8_D_nu9MNq3aFy_Squt-Z6lrTt.gqS1K%2FKD8y3p%2Blv6J9AAi%2BN1jVQ9wAZHAgO0isV9SIY

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        int cookies_len = 0;
        for(int i = 0; i < cookies_count; i++) {
            cookies_len += strlen(cookies[i]);
            if(i + 1 < cookies_count) {
                cookies_len++;
            }
        }
        char *mydata = (char *)calloc(cookies_len + 50, sizeof(char));
        strcat(mydata, "Cookie: ");
        for(int i = 0; i < cookies_count; i++) {
            strcat(mydata, cookies[i]);
            if(i + 1  < cookies_count) {
                strcat(mydata, ";");
            }
        }
        compute_message(message, mydata);
    }
    if(access_token) {
        sprintf(line, "Authorization: Bearer %s", access_token);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, const char * body_data,
                            char **cookies, int cookies_count, char* access_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
   if(access_token) {
        sprintf(line, "Authorization: Bearer %s", access_token);
        compute_message(message, line);
    }
    
    sprintf(line, "Content-Type: application/%s", content_type);
    compute_message(message, line);

    int body_data_len = strlen(body_data);

    sprintf(line, "Content-Length: %d", body_data_len);
    compute_message(message, line);


    // Step 4 (optional): add cookies
    if (cookies != NULL) {
       int cookies_len = 0;
        for(int i = 0; i < cookies_count; i++) {
            cookies_len += strlen(cookies[i]);
            if(i + 1 < cookies_count) {
                cookies_len++;
            }
        }
        char *mydata = (char *)calloc(cookies_len + 50, sizeof(char));
        strcat(mydata, "Cookie: ");
        for(int i = 0; i < cookies_count; i++) {
            strcat(mydata, cookies[i]);
            if(i + 1  < cookies_count) {
                strcat(mydata, ";");
            }
        }
        compute_message(message, mydata);
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    compute_message(message, body_data);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char* access_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

   // Cookie: connect.sid=s%3AnlOL8a8_D_nu9MNq3aFy_Squt-Z6lrTt.gqS1K%2FKD8y3p%2Blv6J9AAi%2BN1jVQ9wAZHAgO0isV9SIY

    // Step 3 (optional): add headers and/or cookies, according to the protocol format

    if(access_token) {
        sprintf(line, "Authorization: Bearer %s", access_token);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        int cookies_len = 0;
        for(int i = 0; i < cookies_count; i++) {
            cookies_len += strlen(cookies[i]);
            if(i + 1 < cookies_count) {
                cookies_len++;
            }
        }
        char *mydata = (char *)calloc(cookies_len + 50, sizeof(char));
        strcat(mydata, "Cookie: ");
        for(int i = 0; i < cookies_count; i++) {
            strcat(mydata, cookies[i]);
            if(i + 1  < cookies_count) {
                strcat(mydata, ";");
            }
        }
        compute_message(message, mydata);
    }
    // Step 4: add final new line
    compute_message(message, "");
    return message;
}