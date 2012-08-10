/*
  * greenhttp server
  * A trivial static http webserver using Libevent's evhttp (for now).
  * The main feature will be to serve C scripts
  * to have fast web applications, low power consumption
  * and money saving 
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#include <time.h>

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

/* Compatibility for possible missing IPv6 declarations */
#include "lib/util-internal.h"
#include "servlet.h"

// headers for http
static void http_error_404(struct evhttp_request *req, int fd);
                                       
char uri_root[512];

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postsript" },
	{ NULL, NULL },
};

/* Try to guess a good content-type for 'path' */
static const char *guess_content_type(const char *path) {
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

/* Callback used for the /dump URI, and for every non-GET request:
 * dumps all information to stdout and gives back a trivial 200 ok */
static void dump_request_cb(struct evhttp_request *req, void *arg) {
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
    	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
    	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
    	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
    	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
    	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
    	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
    	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
    	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
    	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
    	default: cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\nHeaders:\n",
	    cmdtype, evhttp_request_get_uri(req));

	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	buf = evhttp_request_get_input_buffer(req);
	puts("Input data: <<<");
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(buf)-1);
		if (n > 0)
			(void) fwrite(cbuf, 1, n, stdout);
	}
	puts(">>>");

	evhttp_send_reply(req, 200, "OK", NULL);
}

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
static void send_document_cb(struct evhttp_request *req, void *arg) {
	struct evbuffer *evb = NULL;
	const char *docroot = arg;
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;
	const char *default_path = "/index.html";
	char *decoded_path;
	char *whole_path = NULL;
	size_t len;
	int fd = -1;
	struct stat st;
    int disable_directrory_listing = 1;

	/*if ((evhttp_request_get_command(req) != EVHTTP_REQ_GET) || 
	    (evhttp_request_get_command(req) != EVHTTP_REQ_POST)) {
		dump_request_cb(req, arg);
		return;
	}*/

	printf("Got a GET request for <%s>\n",  uri);

	/* Decode the URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Basic path routing. */
	path = evhttp_uri_get_path(decoded);
	if (!path) path = default_path;
	if (strcmp(path, "/") == 0) path = default_path;
	
	// C servlet or static content
	if (strcmp(path, "/servlet") == 0) {
        printf("C servlet engine started\n");
        evb = evbuffer_new();
        servlet(req, evb);

    	if (whole_path)	free(whole_path);
    	if (evb) evbuffer_free(evb);
        return;
	} else {
    	/* We need to decode it, to see what path the user really wanted. */
    	decoded_path = evhttp_uridecode(path, 0, NULL);

    	if (decoded_path == NULL) {
    	    http_error_404(req, fd);
    	    goto done;
    	}

    	/* Don't allow any ".."s in the path, to avoid exposing stuff outside
    	 * of the docroot.  This test is both overzealous and underzealous:
    	 * it forbids aceptable paths like "/this/one..here", but it doesn't
    	 * do anything to prevent symlink following." */
    	if (strstr(decoded_path, "..")) {
    	    http_error_404(req, fd);
    	    goto done;
    	}

    	len = strlen(decoded_path)+strlen(docroot)+2;
    	if (!(whole_path = malloc(len))) {
    		perror("malloc");
    		http_error_404(req, fd);
    		goto done;
    	}
    	evutil_snprintf(whole_path, len, "%s/%s", docroot, decoded_path);

    	if (stat(whole_path, &st)<0) {
    		http_error_404(req, fd);
    		goto done;
    	}

    	/* This holds the content we're sending. */
    	evb = evbuffer_new();
	
	    // directory listing security
    	if (S_ISDIR(st.st_mode) && disable_directrory_listing) {
    	    http_error_404(req, fd);
    	    goto done;
    	}

    	if (S_ISDIR(st.st_mode)) {
    		/* If it's a directory, read the comments and make a little
    		 * index page */
    		DIR *d;
    		struct dirent *ent;
    		const char *trailing_slash = "";

    		if (!strlen(path) || path[strlen(path)-1] != '/')
    			trailing_slash = "/";

    		if (!(d = opendir(whole_path))) {
    		    http_error_404(req, fd);
    		    goto done;
    		}

    		evbuffer_add_printf(evb, "<html>\n <head>\n"
    		    "  <title>%s</title>\n"
    		    "  <base href='%s%s%s'>\n"
    		    " </head>\n"
    		    " <body>\n"
    		    "  <h1>%s</h1>\n"
    		    "  <ul>\n",
    		    decoded_path, /* XXX html-escape this. */
    		    uri_root, path, /* XXX html-escape this? */
    		    trailing_slash,
    		    decoded_path /* XXX html-escape this */);

    		while ((ent = readdir(d))) {
    			const char *name = ent->d_name;
    			evbuffer_add_printf(evb,
    			    "    <li><a href=\"%s\">%s</a>\n",
    			    name, name);/* XXX escape this */
    		}
    		evbuffer_add_printf(evb, "</ul></body></html>\n");
    		closedir(d);
    		evhttp_add_header(evhttp_request_get_output_headers(req),
    		    "Content-Type", "text/html");
        } else {
    		/* Otherwise it's a file; add it to the buffer to get
    		 * sent via sendfile */
    		const char *type = guess_content_type(decoded_path);
    		if ((fd = open(whole_path, O_RDONLY)) < 0) {
    			perror("open");
    			http_error_404(req, fd);
    			goto done;
    		}

    		if (fstat(fd, &st)<0) {
    			/* Make sure the length still matches, now that we
    			 * opened the file :/ */
    			perror("fstat");
        		http_error_404(req, fd);
        		goto done;
    		}
    		evhttp_add_header(evhttp_request_get_output_headers(req),
    		    "Content-Type", type);
    		evbuffer_add_file(evb, fd, 0, st.st_size);
    	}
    }
    
	evhttp_send_reply(req, 200, "OK", evb);

done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (decoded_path)
		free(decoded_path);
	if (whole_path)
		free(whole_path);
	if (evb)
		evbuffer_free(evb);
}

static void http_error_404(struct evhttp_request *req, int fd) {
    evhttp_send_error(req, 404, "Document was not found");
	if (fd >= 0) close(fd);
}

int main(int argc, char **argv) {
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

	unsigned short port = 8080;
	
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return (1);

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* The /dump URI will dump all requests to stdout and say 200 ok. */
	evhttp_set_cb(http, "/dump", dump_request_cb, NULL);

	/* We want to accept arbitrary requests, so we need to set a "generic"
	 * cb.  We can also add callbacks for specific paths. */
	evhttp_set_gencb(http, send_document_cb, "./www");

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
		    (int)port);
		return 1;
	}

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
			fprintf(stderr, "Weird address family %d\n",
			    ss.ss_family);
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
			printf("Listening on %s:%d\n", addr, got_port);
			evutil_snprintf(uri_root, sizeof(uri_root),
			    "http://%s:%d",addr,got_port);
		} else {
			fprintf(stderr, "evutil_inet_ntop failed\n");
			return 1;
		}
	}

	event_base_dispatch(base);

	return 0;
}


