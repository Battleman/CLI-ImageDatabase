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

#include <stdlib.h>
#include <string.h>

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int
do_list_cmd (const char* filename)
{
    struct pictdb_file myfile;

    int fail = do_open(filename, "rb", &myfile);
    if(fail == 0) {
        do_list(&myfile);
        do_close(&myfile);
    }
    return fail;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int
do_create_cmd (const char* filename)
{
    // This will later come from the parsing of command line arguments
    const uint32_t max_files =  10;
    const uint16_t thumb_res =  64;
    const uint16_t small_res = 256;

    puts("Create");
    struct pictdb_file fichier;
    fichier.header = (struct pictdb_header) {
        "", 0, 0, max_files, {thumb_res, thumb_res, small_res, small_res}, 0, 0
    };
    return do_create(filename, &fichier);
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int
help (void)
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n");
    printf("\thelp: displays this help.\n");
    printf("\tlist <dbfilename>: list pictDB content.\n");
    printf("\tcreate <dbfilename>: create a new pictDB.\n");
    printf("\tdelete <dbfilename> <pictID>: delete picture pictID from pictDB.\n");
    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 ********************************************************************* */
int
do_delete_cmd (const char* filename, const char* pictID)
{
    //int errcode = 0;
    if(strlen(pictID) > MAX_PIC_ID || strlen(pictID) == 0) { //first of all, test validity
        return ERR_INVALID_PICID;
    }
    struct pictdb_file myfile;
    /*if((errcode = do_open(filename, "r+b", &myfile)) || (errcode = do_delete(pictID, &myfile))) { //utilisation de la lazy evaluation
        return errcode;
    }
    do_close(&myfile);*/
    int errcode = do_open(filename, "r+b", &myfile);
    if(errcode == 0){
		errcode = do_delete(pictID, &myfile);
		do_close(&myfile);
	}
    return errcode;
}

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 08: THIS PART SHALL BE REVISED THEN (WEEK 09) EXTENDED.
         * **********************************************************************
         */
        argc--;
        argv++; // skips command call name
        if (!strcmp("list", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_list_cmd(argv[1]);
            }
        } else if (!strcmp("create", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_create_cmd(argv[1]);
            }
        } else if (!strcmp("delete", argv[0])) {
            if (argc < 3) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_delete_cmd(argv[1], argv[2]);
            }
        } else if (!strcmp("help", argv[0])) {
            ret = help();
        } else {
            ret = ERR_INVALID_COMMAND;
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help();
    }

    return ret;
}
