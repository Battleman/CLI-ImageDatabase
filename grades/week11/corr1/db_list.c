/**
 * @file db_list.c
 * @brief pictDB library: do_list implementation.
 */
#include "pictDB.h"
#include <json-c/json.h>
/********************************************************************//**
 * Iterates through the database and prints the header and each
 * valid database entry. Different behaviour considering the mode.
 */
const char* do_list(const struct pictdb_file* db_file, enum do_list_mode mode)
{
    //correcteur: utiliser un switch plutot, c'est presque fait pour les enums.
    if(mode == STDOUT) {
        //correcteur: il aurait étéplus logique (et juste) de mettre cette condition en dehors, elle devrait aussi etre vraie en mode JSON
        if(db_file != NULL) {
            print_header(&db_file->header); //anyway : print the header

            if(db_file->header.num_files == 0) {
                printf("<<empty database>>\n");
            }
            //correcteur: utiliser un else pour ne pas faire cette boucle s'il n'y a pas besoin. (ou return dans le if)
            for(size_t i = 0; i < db_file -> header.max_files; ++i) { //go through the whole database
                if(db_file -> metadata[i].is_valid == NON_EMPTY) {
                    print_metadata(&db_file -> metadata[i]);
                }
            }
        }
        return NULL;
    } else if(mode == JSON) {
        const char name_str[] = "Pictures";
        struct json_object* array = json_object_new_array(); //création d'un array

        for(size_t i = 0; i < db_file->header.max_files; i++) { //pour chaque image :
            if(db_file->metadata[i].is_valid == NON_EMPTY) {
                struct json_object* pict_name = json_object_new_string(db_file->metadata[i].pict_id); //on crée une string
                json_object_array_add(array, pict_name); //qu'on ajoute dans l'array
            }
        }

        struct json_object* object = json_object_new_object(); //création d'un JSON object
        json_object_object_add(object, name_str, array); //dans lequel on ajoute le couple Picture : array[]
        const char* string = json_object_to_json_string (object); //transformation en string

        char* indep_string = malloc(strlen(string)+1);
        if(indep_string == NULL) return NULL;
        strcpy(indep_string, string);

        if(1 != json_object_put(object)) {
            free(indep_string);
            return NULL;
        }
        return indep_string; //retour de l'objet en string

    } else {
        //correcteur: il faudrait l'allouer car sinon on le free apres l'appel a do_list alors que ca n'a pas été alloué.
        return "unimplemented do_list mode";
    }
}
