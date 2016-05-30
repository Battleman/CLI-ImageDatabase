/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"
#include "image_content.h"
#include <vips/vips.h> // for obvious reasons ;-)
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "pictDBM_tools.h"

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int
do_list_cmd (int argc, char *argv[])
{
    int errcode = ERR_NOT_ENOUGH_ARGUMENTS; //sera retourné si le premier if échoue
    struct pictdb_file db_file; //!<La base de donnée locale

    if(argc > 0) {
        errcode = do_open(argv[0], "rb", &db_file);
        if(errcode == 0) {
            const char* listed = do_list(&db_file, STDOUT);
            if(listed != NULL) printf("%s", listed);
            free((void*)listed);
        }

        do_close(&db_file);
    }
    return errcode;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int
do_create_cmd (int argc, char *argv[])
{
    //Valeurs par défaut
    uint32_t max_files = 10;
    uint16_t thumb_res_X = 64, thumb_res_Y = 64;
    uint16_t small_res_X = 256, small_res_Y = 256;

    puts("Create");
    struct pictdb_file db_file; //!<La base de donnée locale

    //argv = "db_name"
    const char* db_name = argv[0];
    argc--;
    argv++;

    while(argc != 0) { //itération jusqu'à la fin des arguments
        if(!strcmp(argv[0], "-max_files")) {
            //Parsing des arguments de -max_files
            if(argc > 1) {
                max_files = atouint32(argv[1]);
                if(max_files <= 0 || max_files > MAX_MAX_FILES) return ERR_INVALID_ARGUMENT;
                argc -= 2;
                argv += 2;
            } else return ERR_NOT_ENOUGH_ARGUMENTS;
        } else if(!strcmp(argv[0], "-thumb_res")) {
            //Parsing des arguments de -thumb_res
            if(argc > 2) {
                thumb_res_X = atouint16(argv[1]);
                thumb_res_Y = atouint16(argv[2]);
                if(thumb_res_X <= 0 || thumb_res_Y <= 0 || thumb_res_X > MAX_THUMB_SIZE || thumb_res_Y > MAX_THUMB_SIZE) return ERR_INVALID_ARGUMENT;
                argc -= 3;
                argv += 3;
            } else return ERR_NOT_ENOUGH_ARGUMENTS;
        } else if(!strcmp(argv[0], "-small_res")) {
            //Parsing des arguments de -small_res
            if(argc > 2) {
                small_res_X = atouint16(argv[1]);
                small_res_Y = atouint16(argv[2]);
                if(small_res_X <= 0 || small_res_Y <= 0 || small_res_X > MAX_SMALL_SIZE || small_res_Y > MAX_SMALL_SIZE) return ERR_INVALID_ARGUMENT;
                argc -= 3;
                argv += 3;
            } else return ERR_NOT_ENOUGH_ARGUMENTS;
        } else return ERR_INVALID_ARGUMENT;
    }

    // Création d'une base de données avec les arguments fournis (ou uniquement les valeurs par défaut si elles n'ont pas été modifiées)
    int errcode = do_create(db_name, &db_file, max_files, thumb_res_X, thumb_res_Y, small_res_X, small_res_Y);
    if(errcode == 0) {
        print_header(&db_file.header);
    }
    return errcode;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int
help (int argc, char *argv[])
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n");
    printf("\thelp: displays this help.\n");
    printf("\tlist <dbfilename>: list pictDB content.\n");
    printf("\tcreate <dbfilename> [options]: create a new pictDB.\n");
    printf("\t\t\toptions are:\n");
    printf("\t\t\t-max_files <MAX_FILES>: maximum number of files.\n");
    printf("\t\t\t\t\t\tdefault value is 10\n");
    printf("\t\t\t\t\t\tmaximum value is 100000\n");
    printf("\t\t\t-thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
    printf("\t\t\t\t\t\tdefault value is 64x64\n");
    printf("\t\t\t\t\t\tmaximum value is 128x128\n");
    printf("\t\t\t-small_res <X_RES> <Y_RES>: resolution for small images.\n");
    printf("\t\t\t\t\t\tdefault value is 256x256\n");
    printf("\t\t\t\t\t\tmaximum value is 512x512\n");
    printf("\tread  <dbfilename> <pictID> [original|orig|thumbnail|thumb|small]:\n");
    printf("\t   read an image from the pictDB and save it to a file.\n");
    printf("\t   default resolution is 'original'.\n");
    printf("\tinsert <dbfilename> <pictID> <filename>: insert a new image in the pictDB.\n");
    printf("\tdelete <dbfilename> <pictID>: delete picture pictID from pictDB.\n");
    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 ********************************************************************* */
int
do_delete_cmd (int argc, char *argv[])
{
    if(argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    // Test de la validité des arguments
    if(strlen(argv[1]) > MAX_PIC_ID || strlen(argv[1]) == 0) {
        return ERR_INVALID_PICID;
    }

    // Traitement de la commande avec la gestion d'erreur appropriées
    struct pictdb_file db_file;//!<La base de donnée locale
    int errcode = do_open(argv[0], "r+b", &db_file); //ouverture de la base de données
    if(errcode == 0) {
        errcode = do_delete(argv[1], &db_file); //suppression de l'élement
        do_close(&db_file); //fermeture du flot nécessaire (indépendante du succès de la suppression)
    }
    return errcode;
}

/********************************************************************//**
 * Insert a picture in the database.
 ********************************************************************* */
int do_insert_cmd(int argc, char *argv[])
{
    if(argc < 3) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    struct pictdb_file db_file;//!<La base de donnée locale
    do_open(argv[0], "r+b", &db_file);

    int errcode = 0;
    //Test remplissage
    if(db_file.header.num_files >= db_file.header.max_files) {
        errcode = ERR_FULL_DATABASE;
    } else {
        size_t size = 0;
        void* buffer = NULL; //emplacement pour l'image
        errcode = read_disk_image((const char*)argv[2], &buffer, &size); //on met l'image dans le buffer
        if(errcode == 0) {
            if(size !=  0 && buffer != NULL) {
                errcode = do_insert(argv[1], (char *)buffer, size, &db_file); //puis on insère le buffer
            } else {
                errcode = ERR_IO;
            }
        }
    }
    do_close(&db_file);

    return errcode;
}

/********************************************************************//**
 * Read a database from the disk.
 ********************************************************************* */
int do_read_cmd(int argc, char *argv[])
{
    if(argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    //Traitement arguments et des données nécessaires à la lecture

    int errcode = 0;
    struct pictdb_file db_file;//!<La base de donnée locale
    if(0 != (errcode = do_open(argv[0], "r+b", &db_file))) return errcode;
    int res = RES_ORIG;
    if (argc >= 3) {
        if (-1 == (res = resolution_atoi(argv[2]))) {
            return ERR_RESOLUTIONS;
        }
    }

    // Lecture à proprement parler

    char* image_buffer = NULL; //emplacement pour l'image
    uint32_t image_size = 0; //emplacement pour la taille
    errcode = do_read(argv[1], (const int)res, &image_buffer, &image_size, &db_file);
    if(errcode == 0) {
        char* filename = calloc(MAX_PIC_ID + 11, sizeof(char)); //11 pour le max des small/.jpg/...
        if(filename == NULL) errcode = ERR_OUT_OF_MEMORY;
        else {
            if(0 == (errcode = create_name((const char*)argv[1], filename, res))) {
                FILE* image_file = fopen(filename, "wb"); //ouverture de l'image
                if(image_file == NULL) {
                    errcode = ERR_IO;
                } else {
                    errcode = write_disk_image(image_file, image_buffer, image_size);
                    fclose(image_file); //fermeture de l'image
                }
            }
            free(filename);
        }

    }
    free(image_buffer);
    do_close(&db_file);
    return errcode;
}


/********************************************************************//**
 * Clean the database and resize the file containing potential 
 * useless pictures.
 ********************************************************************* */
int do_gbcollect(int argc, char *argv[])
{
}

/********************************************************************//**
 * Definition of the types allowing us to modularise the main.
 ********************************************************************** */

typedef int (*command)(int args, char *argv[]);

typedef struct {
    const char* name;
    command cmd;
} command_mapping;


/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    command_mapping commands[NB_COMMANDS] = {(command_mapping){"list", do_list_cmd},
    (command_mapping){"create", do_create_cmd},
    (command_mapping){"help", help},
    (command_mapping){"delete", do_delete_cmd},
    (command_mapping){"read", do_read_cmd},
    (command_mapping){"insert", do_insert_cmd}
    (command_mapping){"gc", do_gbcollect}
                                            };
    int ret = 0;
    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        const char* app_name = argv[0]; //pour vips
        argc--;
        argv++; // skips command call name

        int index = 0, valid = 0;
        VIPS_INIT(app_name);
        do {
            if(!strcmp(commands[index].name, argv[0])) {
                if (argc < 1) { //au moins 1, pour help
                    ret = ERR_NOT_ENOUGH_ARGUMENTS;
                } else {
                    argc--;
                    argv++;
                    ret = commands[index].cmd(argc, argv);
                }
                valid = 1;
            } else {
                ++index;
            }
        } while(index < NB_COMMANDS && valid == 0);
        vips_shutdown();
        if(valid == 0) {
            ret = ERR_INVALID_COMMAND;
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help(argc, argv);
    }
    return ret;
}
