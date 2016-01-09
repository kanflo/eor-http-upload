/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Johan Kanflo (github.com/kanflo)
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

#ifndef __HTTP_UPLOAD_H__
#define __HTTP_UPLOAD_H__

#include <stdint.h>
#include <stdbool.h>

// Connect http://host:port for an HTTP file upload
// Returns true if all went well
bool upload_connect(char *host, uint16_t port);

// Send HTTP POST file upload for a file named file_name of size file_size.
// Returns true if all went well
bool upload_begin(char *url, char *file_name, uint32_t file_size);

// Upload data, may be called several times
// Returns true if all went well
bool upload_data(void *data, uint32_t length);

// Finish file upload
// Returns HTTP status code as returned by server or 0 if none found
uint32_t upload_finish(void);

// Close upload socket
void upload_close(void);

#endif // __HTTP_UPLOAD_H__
