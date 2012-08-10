#include <stdio.h>
#include <stdlib.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

void servlet(struct evhttp_request *req, struct evbuffer *evb);