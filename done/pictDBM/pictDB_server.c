#include "pictDB.h"
#include "mongoose.h"

#define MAX_QUERY_PARAM 5

static int s_sig_received = 0;
static const char *s_http_port = "8000";
static struct pictdb_file* db_file;
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);
  s_sig_received = sig_num;
}

void split(char* result[], char* tmp, const char* src, const char* delim, size_t len){
	
	for(int i = 0; i < MAX_QUERY_PARAM; ++i){
		result[i] = 
	}
}

static void handle_list_call(struct mg_connection *nc, struct http_message *hm){
	 
	 if(db_file == NULL){
		 //SEND HTML ERROR MESSAGE
	 } else {
		 const char* buffer = do_list(db_file, JSON);
		 
		 if(buffer == NULL){
			//SEND HTML ERROR MESSAGE
		 }
		 
		 mg_printf(nc, "HTTP/1.0 200 OK\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: %d\r\n\r\n%s",
					(int) strlen(buffer), buffer);
		  nc->flags |= MG_F_SEND_AND_CLOSE;
	}
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;
	
	switch (ev) {
    case MG_EV_HTTP_REQUEST:
      if (mg_vcmp(&hm->uri, "/pictDB/list") == 0) {
        handle_list_call(nc, hm); /* Handle basic call */
      } else {
        mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
      }
      break;
    default:
      break;
  }
}

int main(int argc, char* argv[]){
	
	int ret = 0;
	
	if(argc < 2){
		ret = ERR_NOT_ENOUGH_ARGUMENTS;
		fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
		return ret;
	}
	
	struct mg_mgr mgr;
	struct mg_connection *nc;
	
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	
	mg_mgr_init(&mgr, NULL);
	nc = mg_bind(&mgr, s_http_port, ev_handler);
	
	if (nc == NULL) {
		fprintf(stderr, "Error starting server on port %s\n", s_http_port);
		return ERR_IO;
	}
	
	const char* db_name = argv[1];
    if(0 != do_open(db_name, "r+b", db_file)){
		ret = ERR_IO;
		fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
		return ret;
	}
    print_header(&db_file->header);
	
	while (!s_sig_received) {
		mg_mgr_poll(&mgr, 1000000);
	}
	
	printf("Exiting on signal %d\n", s_sig_received);
	do_close(db_file);

	mg_mgr_free(&mgr);
	
	return 0;
}
