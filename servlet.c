#include "servlet.h"
char *trim(char *str);

void servlet(struct evhttp_request *req, struct evbuffer *evb, const char *get_query) {
    /*dbi_conn conn;
    dbi_result result;           
    unsigned int idnumber;
    char *fullname;
    
    dbi_initialize(NULL);
    conn = dbi_conn_new("mysql");    
    dbi_conn_set_option(conn, "host", "localhost");
    dbi_conn_set_option(conn, "username", "root");
    dbi_conn_set_option(conn, "password", "root");
    dbi_conn_set_option(conn, "dbname", "test");
    dbi_conn_set_option(conn, "encoding", "UTF-8");
    if (dbi_conn_connect(conn) < 0) {
      printf("Could not connect");
    } else {
        result = dbi_conn_queryf(conn, "SELECT id, name FROM test");
        
        if (result) {
            printf("dbi_result_get_numrows: %u", dbi_result_get_numrows(result));
            
            while (dbi_result_next_row(result)) {
                idnumber = dbi_result_get_uint(result, "id");
                fullname = dbi_result_get_string(result, "name");
                printf("%i. %s", idnumber, fullname);
            }
        
            dbi_result_free(result);
        }
        
        dbi_conn_close(conn);      
    }*/
    

    
    time_t now;
    time(&now);
    const char *value;
    struct evkeyvalq params;
    char *data;
    const char *get_data;
    
    if (evhttp_request_get_command(req) == EVHTTP_REQ_POST) {
        size_t len = evbuffer_get_length(evhttp_request_get_input_buffer(req));
        struct evbuffer *in_evb = evhttp_request_get_input_buffer(req);
        data = malloc(len + 1);
        evbuffer_copyout(in_evb, data, len);
        data[len] = '\0';
        evhttp_parse_query_str(data, &params);
    } else if (evhttp_request_get_command(req) == EVHTTP_REQ_GET) {
        //data = malloc(strlen(data) + 1);
        get_data = get_query;
        //data[strlen(data)] = '\0';
        evhttp_parse_query_str(get_data, &params);
    } else {
        data = "\0";
        evhttp_parse_query_str(data, &params);
    }
    
    // read cookies

    // end cookies
    
    // Parse the query string
    value = evhttp_find_header(&params, "first_name");
    
    evbuffer_add_printf(evb, "<html>\n <head>\n"
	    "  <title>%s</title>\n"
	    " </head>\n"
	    " <body>\n"
	    "  <h1>%s</h1>\n"
	    " <p>Current time is: %s (GMT)</p>" 
	    " <p>First name is: %s</p>\n<hr>\n"
	    "<form action=\"/servlet\" method=\"POST\">\n"
	    "First name <input type=\"text\" name=\"first_name\">"
	    "<input type=\"submit\" value=\"send\">"
	    "</form>\n"
	    "<p>COOKIE: %s</p>"
	    "</body><html>",
	    "C servlet title", 
	    "C servlet", 
	    ctime(&now),
	    value,
	    http_get_cookie(req, "CSESSID"));
    evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Content-Type", "text/html");
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Set-Cookie", "CSESSID=435J43L523J4TTYYY; Path=/; Domain=intranet2.local");
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Set-Cookie", "email=rt@example.com; Path=/; Domain=intranet2.local");
	evhttp_add_header(evhttp_request_get_output_headers(req),
	    "Server", "green httpd");
	evhttp_send_reply(req, 200, "OK", evb);
	
	if (evhttp_request_get_command(req) == EVHTTP_REQ_POST) {
	    evhttp_clear_headers(&params);
        free(data);
	} else if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        free(data);
	}
}

char *http_get_cookie(struct evhttp_request *req, const char *key) {
    struct evkeyvalq *headers;
    struct evkeyval *header;
    headers = evhttp_request_get_input_headers(req);

	for (header = headers->tqh_first; header; header = header->next.tqe_next) {
	        // find cookie header
            if (strcmp(header->key, "Cookie") == 0) {
                return header->value;
            }
	}
	
    return " ";
}
/*
struct evkeyval *tokenize() {
    char s[] = "CSESSID=435J43L523J4TTYYY; email=rt@example.com";
    char* kv;
    struct evkeyval *token;

    kv = strtok(s, ";");
    int i = 0;

    while (kv != NULL) {
      printf("%s\n", trim(kv));

      kv = strtok(NULL, ";");
    }
}

char *trim(char *str) {
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
*/