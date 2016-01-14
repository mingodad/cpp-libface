#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <vector>
#include <list>
#include <map>
#include <string>
#include <sstream>
namespace nw=std;
#endif // USE_USTL
#include "libuv/include/uv.h"
#include "http-parser/http_parser.h"

typedef nw::map<nw::string, nw::string> headers_t;
typedef nw::map<nw::string, nw::string> query_strings_t;

struct partial_buf_t : uv_buf_t {
    size_t size;
    size_t offset;

    partial_buf_t(uv_buf_t buf, size_t s, size_t o)
        : uv_buf_t(buf), size(s), offset(o) { }
};

struct client_t {
    uv_tcp_t                       handle;
    http_parser                    parser;
    uv_write_t                     write_req;
    nw::vector<nw::string>       resstrs;
    nw::vector<partial_buf_t>     unparsed_data;
    nw::list<client_t*>::iterator cciter;
    nw::string                    url;
    nw::string                    last_header_field;
    headers_t                     req_headers;

    client_t() { }
};

struct parsed_url_t {
    nw::string path;
    query_strings_t query;
};

typedef void (*request_callback_t)(client_t*);

void build_HTTP_response_header(nw::string &response_header,
                                int http_major, int http_minor,
                                int status_code, const char *status_str,
                                headers_t &headers,
                                nw::string const &body);
void write_response(client_t *client,
                    int status_code,
                    const char *status_str,
                    headers_t &headers,
                    nw::string &body);
void on_close(uv_handle_t* handle);
uv_buf_t on_alloc(uv_handle_t* client, size_t suggested_size);
void on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf);
void on_connect(uv_stream_t* server_handle, int status);
void after_write(uv_write_t* req, int status);
void parse_query_string(nw::string &qstr, query_strings_t &query);
void parse_URL(nw::string const &url_str, parsed_url_t &uout);
int on_url(http_parser *parser, const char *data, size_t len);
int on_header_field(http_parser *parser, const char *data, size_t len);
int on_header_value(http_parser *parser, const char *data, size_t len);
int on_message_complete(http_parser* parser);
int httpserver_start(request_callback_t rcb, const char *ip, int port);

#endif // HTTPSERVER_HPP
