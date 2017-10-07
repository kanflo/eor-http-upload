/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Johan Kanflo (github.com/kanflo)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>


#define POST_HEADER \
        "POST /%s HTTP/1.1\r\n" \
        "User-Agent: simpleupload v0.1\r\n" \
        "Host: localhost\r\n" \
        "Referer: 127.0.0.1\r\n" \
        "Accept: */*\r\n" \
        "Expect: 100-continue\r\n" \
        "Content-Length: %u\r\n" \
        "Content-Type: multipart/form-data; boundary=foo\r\n" \
        "\r\n"

#define POST_DATA \
        "--foo\r\n" \
        "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n" \
        "Content-Type: application/octet-stream\r\n" \
        "\r\n"

// Length of data with room for file name (excluding actual file contents)
#define POST_DATA_LENGTH (strlen(POST_DATA))

#define END_MARKER \
        "\r\n--foo--\r\n"


// Verbose printfs
//#define CONFIG_UPLOAD_DEBUG

#ifdef CONFIG_UPLOAD_DEBUG
 #define ULDBG(x) x
#else
 #define ULDBG(x)
#endif // CONFIG_UPLOAD_DEBUG

static int sockfd;

static uint32_t parse_http_status(char *response);

bool upload_connect(char *host, uint16_t port)
{
    bool success = false;
    struct addrinfo *res;
    char sport[7];
    snprintf(sport, sizeof(sport)-1, "%d", port);
#if 1
    do {
        const struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };

        ULDBG(printf("Running DNS lookup for %s...\n", host));
        int err = getaddrinfo(host, sport, &hints, &res);

        if(err != 0 || res == NULL) {
            ULDBG(printf("DNS lookup failed err=%d res=%p\n", err, res));
            break;
        }
        /* Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        ULDBG(struct in_addr *addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr);
        ULDBG(printf("DNS lookup succeeded. IP=%s\n", inet_ntoa(*addr)));

        sockfd = socket(res->ai_family, res->ai_socktype, 0);
        if(sockfd < 0) {
            ULDBG(printf("... Failed to allocate socket.\n"));
            break;
        }

        ULDBG(printf("... allocated socket\n"));

        if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
            lwip_close(sockfd);
            ULDBG(printf("... socket connect failed.\n"));
            break;
        }

        ULDBG(printf("... connected\n"));
        success = true;
    } while(0);

    if(res) {
        freeaddrinfo(res);
    }
#else
    struct sockaddr_in serv_addr;
    do {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            ULDBG(printf("Error: failed to create socket\n"));
            break;
        } 

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if(inet_pton(AF_INET, host, &serv_addr.sin_addr)<=0) {
            ULDBG(printf("Error: inet_pton failed\n"));
            break;
        } 

        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            ULDBG(printf("Error: failed to connect to %s:%d\n", host, port));
            break;
        } 

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval)) < 0) {
            ULDBG(printf("Error: failed set socket timeout\n"));
            break;
        }
        success = true;
    } while(0);
#endif
    return success;
}

bool upload_begin(char *url, char *file_name, uint32_t file_size)
{
    bool success = false;
    int n;
    uint32_t post_length, post_hdr_len;
    post_hdr_len = strlen(POST_HEADER) + strlen(url) + strlen(file_name) + 1 + 10;
    // 1 + 10: null terminator + content-length field
    char *header = (char*) malloc(post_hdr_len);
    char *data = (char*) malloc(POST_DATA_LENGTH + strlen(file_name));
    if (header && data) {
        snprintf(data, POST_DATA_LENGTH + strlen(file_name), POST_DATA, file_name);
        post_length = strlen(data) + file_size + strlen(END_MARKER);
        snprintf(header, post_hdr_len - 1, POST_HEADER, url, post_length);

        n = lwip_write(sockfd, (void*) header, strlen(header));
        ULDBG(printf("Sent HTTP header, %d bytes\n", n));
        n = lwip_write(sockfd, (void*) data, strlen(data));
        ULDBG(printf("Sent file upload header, %d bytes\n", n));
        (void) n; // Compiles complains if not ULDBG

        success = true;
    }
    if (header) free(header);
    if (data) free(data);
    return success;
}

bool upload_data(void *data, uint32_t length)
{
    if (lwip_write(sockfd, data, length) < 0) {
        ULDBG(printf("Error: socket write failed\n"));
        return false;
    }
    return true;
}

uint32_t upload_finish(void)
{
    char buffer[32];
    int n;
    uint32_t http_status = 0;

    if (lwip_write(sockfd, (void*) END_MARKER, strlen(END_MARKER)) < 0) {
        ULDBG(printf("Error: socket write failed for end marker\n"));
        return false;
    }

    while ( (n = lwip_read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
        buffer[n] = 0;
        ULDBG(printf("%s", buffer));
        uint32_t temp = parse_http_status(buffer);
        if (temp > 0 && http_status == 0) {
            http_status = temp;
        }
        if (http_status > 0) {
            return http_status; // Break early
        }
    }
    return http_status;
}

void upload_close(void)
{
    if (sockfd) {
        lwip_close(sockfd);
        sockfd = 0;        
    }
}

// Poor man's HTTP status parser.
// Return HTTP status code or 0 if none found
// Note. Modifies response
static uint32_t parse_http_status(char *response)
{
    uint32_t http_response = 0;
    char *s1, *s2, *s3;
    s1 = response;
    do {
        // Look for "HTTP/1.1 XXX"
        s1 = strstr(s1, "HTTP/");
        if (!s1) 
            break;
        s2 = strstr(s1, " "); // Allow for other versions than HTTP/1.1
        if (!s2) 
            break;
        s2++;
        s3 = strstr(s2, " ");
        if (!s3)
            break;
        s1 = s3+1;
        *s3 = 0;
        http_response = atoi(s2);
#if 0
        if (http_response >= 100 && http_response <= 199) {
            // We don't care about "1xx Informational"
            // See https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
            http_response = 0;
        }
#endif
    } while(http_response == 0 && s1);
    return http_response;
}
