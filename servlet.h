#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#include <dbi/dbi.h>

void servlet(struct evhttp_request *req, struct evbuffer *evb, const char *get_query);
char * http_get_cookie(struct evhttp_request *req, const char *key);