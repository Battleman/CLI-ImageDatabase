/**@file pictDB_server.c
 *
 * @brief implémentation d'un web-serveur exploitant les méthodes sur base de donnée
 */

#include "pictDB.h"
#include "libmongoose/mongoose.h"

static int s_sig_received = 0;
static const char *s_http_port = "8000"; //!<Le port sur lequel on travaille
static struct pictdb_file db_file; //!<La base de donnée "en local"
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num)
{
    signal(sig_num, signal_handler);
    s_sig_received = sig_num;
}

/**@brief gestion d'erreur mongoose
 *
 * Fonction générique qui redirige une page d'erreur en cas de problème
 */
void mg_error(struct mg_connection* nc, int error)
{
    const char* htmlBefore = "<html>\n" //première partie de la page
                             "\t<head>\n"
                             "\t\t<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.10.1/jquery.min.js\"></script>\n"
                             "\t</head>\n"
                             "\t<body>\n"
                             "<h1>Internal Error:  ";
                             
    const char* htmlAfter = "</h1><a href='/index.html'>Back to index</a>\n" //Seconde partie
                            "\t</body>\n"
                            "</html>";

    mg_printf(nc, "HTTP/1.1 500 Internal Error\r\n"
              "ERROR: %s\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n"
              "%s" //htmlBefore
              "%s" //error message
              "%s", //htmlAfter
              ERROR_MESSAGES[error], strlen(htmlBefore) + strlen(htmlAfter) + strlen(ERROR_MESSAGES[error]), htmlBefore, ERROR_MESSAGES[error], htmlAfter);

}

/**@brief Gestion de l'affichage (list) des images*/
static void handle_list_call(struct mg_connection *nc, struct http_message *hm)
{
    const char* buffer = do_list(&db_file, JSON); //récupération de la string JSON
    if(buffer == NULL) {
        mg_error(nc, ERR_IO);
    } else {
        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n\r\n"
                  "%s", strlen(buffer), buffer);
        free((char*)buffer);
    }
}
/**@brief Gestion de l'affichage (read) des images*/
static void handle_read_call(struct mg_connection *nc, struct http_message *hm)
{
    int res = -1; //!<Par défaut, la résolution est "fausse"
    const char* delim = "&="; //!<List des délimiteurs de l'URI
    char pict_id[MAX_PIC_ID+1];
    char* result[MAX_QUERY_PARAM];

    char* tmp = calloc((MAX_PIC_ID + 1) * MAX_QUERY_PARAM, sizeof(char));
    split(result, tmp, hm -> query_string.p, delim, hm -> query_string.len);
    for(size_t i = 0; i < MAX_QUERY_PARAM && result[i] != NULL; ++i) { //dès le premier NULL, il n'y a plus d'arguments
        if(!strcmp(result[i], "res")) {
            i++;
            res = resolution_atoi(result[i]); //récupération de la résolution
        } else if(!strcmp(result[i], "pict_id")) {
            i++;
            strncpy(pict_id, result[i], MAX_PIC_ID); //récupération du nom
            pict_id[MAX_PIC_ID] = '\0';
        }
    }
    free(tmp); //string intermédiaire inutile

    if(res == -1 || pict_id == NULL) { //si la lecture de la résolution ou du nom a échoué
        mg_error(nc, ERR_INVALID_ARGUMENT);
    } else {
        int err = 0;
        char* img_buffer = NULL;//buffer pour l'image
        uint32_t img_size = 0;

        if(0 == (err = do_read(pict_id, (const int)res, &img_buffer, &img_size, &db_file))) {
            //Header de la page
            mg_printf(nc, "HTTP/1.0 200 OK\r\n"
                      "Content-Type: image/jpeg\r\n"
                      "Content-Length: %d\r\n\r\n",
                      img_size);
            //contenu de la page (l'image)
            mg_send(nc, img_buffer, img_size);
        } else {
            mg_error(nc, err);
        }

        free(img_buffer); //alloué dans do_read -> libération
    }
}
/**@brief Gestion de l'insertion des images*/
static void handle_insert_call(struct mg_connection *nc, struct http_message *hm)
{
    char var_name[100]; //dummy variable
    char pic_name[MAX_PIC_ID+1]; pic_name[MAX_PIC_ID] = '\0';
    const char *image = NULL;
    size_t img_len = 0;

    mg_parse_multipart(	hm->body.p, hm->body.len,
                        var_name, sizeof(var_name),
                        pic_name, MAX_PIC_ID, &image, &img_len);
                        
    int err = do_insert(pic_name, (char*)image, img_len, &db_file);
    if(!err) {
        mg_printf(nc, "HTTP/1.1 302 Found\r\n"
                  "Location: http://localhost:%s/index.html\r\n\r\n",
                  s_http_port);
    } else {
        mg_error(nc, err);
    }
}

/**@brief Gestion de la suppression des images*/
static void handle_delete_call(struct mg_connection *nc, struct http_message *hm)
{
    const char* delim = "&=";
    char pict_id[MAX_PIC_ID+1];
    char* result[MAX_QUERY_PARAM];

    char* tmp = calloc((MAX_PIC_ID + 1) * MAX_QUERY_PARAM, sizeof(char));
    split(result, tmp, hm -> query_string.p, delim, hm -> query_string.len);
    for(int i = 0; i < MAX_QUERY_PARAM && result[i] != NULL; ++i) { //stops at first NULL
        if(!strcmp(result[i], "pict_id")) {
            i++;
            strncpy(pict_id, result[i], MAX_PIC_ID);
            pict_id[MAX_PIC_ID] = '\0';
        }
    }
    free(tmp);

    if(pict_id == NULL) {
        mg_error(nc, ERR_INVALID_ARGUMENT);
    } else {
        int err = 0;
        if(0 != (err = do_delete(pict_id, &db_file))) {
            mg_error(nc, err);
        } else {
            mg_printf(nc, "HTTP/1.1 302 Found\r\n"
                      "Location: http://localhost:%s/index.html\r\n\r\n",
                      s_http_port);
        }

    }
}

/**@brief Tri puis appel correct selon la fonction voulue*/
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;
    switch (ev) {

    case MG_EV_HTTP_REQUEST:
        if(mg_vcmp(&hm->uri, "/pictDB/list") == 0) {
            handle_list_call(nc, hm); 	// Handles basic list call
        } else if(mg_vcmp(&hm->uri, "/pictDB/read") == 0) {
            handle_read_call(nc, hm); 	// Handles basic read call
        } else if(mg_vcmp(&hm->uri, "/pictDB/insert") == 0) {
            handle_insert_call(nc, hm); // Handles basic insert call
        } else if(mg_vcmp(&hm->uri, "/pictDB/delete") == 0) {
            handle_delete_call(nc, hm); // Handles basic delete call
        } else {
            mg_serve_http(nc, hm, s_http_server_opts); //Serve static content
        }
        nc->flags |= MG_F_SEND_AND_CLOSE;
        break;
    default:
        break;
    }
}

int main(int argc, char* argv[])
{

    int ret = 0;

    if(argc < 2) {
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

    const char* app_name = argv[0];
    VIPS_INIT(app_name);

    const char* db_name = argv[1];
    if(0 != do_open(db_name, "r+b", &db_file)) {
        ret = ERR_IO;
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        return ret;
    }

    print_header(&db_file.header);
    mg_set_protocol_http_websocket(nc);
    while (!s_sig_received) {
        mg_mgr_poll(&mgr, 300);
    }

    printf("Exiting on signal %d\n", s_sig_received);
    do_close(&db_file);
    vips_shutdown();

    mg_mgr_free(&mgr);

    return 0;
}
