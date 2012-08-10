#include "servlet.h"

void servlet(struct evhttp_request *req, struct evbuffer *evb) {
    size_t len = evbuffer_get_length(evhttp_request_get_input_buffer(req));
    struct evbuffer *in_evb = evhttp_request_get_input_buffer(req);
    char *data = malloc(len);
    evbuffer_copyout(in_evb, data, len);
    time_t now;
    time(&now);
    evbuffer_add_printf(evb, "<html>\n <head>\n"
	    "  <title>%s</title>\n"
	    " </head>\n"
	    " <body>\n"
	    "  <h1>%s</h1>\n"
	    " <p>Current time is: %s</p>" 
	    " <p>received data size: %d</p>\n"
	    " <p>received data: %s</p></body></html>",
	    "C servlet title", 
	    "C servlet", 
	    ctime(&now), 
	    (int)len,
	    data);
    evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Content-Type", "text/html");
	evhttp_send_reply(req, 200, "OK", evb);
    free(data);
}