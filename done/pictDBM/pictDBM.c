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
#include "pictDBM_tools.h"

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int
do_list_cmd (int argc, char *argv[])
{
    int fail = 1;
    struct pictdb_file myfile;

    if(argc > 0) {
        fail = do_open(argv[0], "rb", &myfile);
        if(fail == 0) {
            do_list(&myfile);
            lazily_resize(RES_SMALL, &myfile, 1);
        }
        do_close(&myfile);
    }
    return fail;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int
do_create_cmd (int argc, char *argv[])
{
    uint32_t max_files = 10;
    uint16_t thumb_res_X = 64, thumb_res_Y = 64;
    uint16_t small_res_X = 256, small_res_Y = 256;

    puts("Create");
    struct pictdb_file myfile;

    //argv = "filename"
    const char* filename = argv[0];
    argc--;
    argv++;
	
    while(argc != 0) {
        if(strcmp("-max_files", argv[0])) {
            if(argc > 1) {
                max_files = (atouint32(argv[1]) < MAX_MAX_FILES) ? atouint32(argv[1]) : MAX_MAX_FILES;
                argc -= 2;
                argv += 2;
            } else {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
        } else if(strcmp("-thumb_res", argv[0])) {
            if(argc > 3) {
                thumb_res_X = (atouint16(argv[1]) < thumb_res_X) ? atouint16(argv[1]) : MAX_THUMB_SIZE;
                thumb_res_Y = (atouint16(argv[2]) < thumb_res_Y) ? atouint16(argv[2]) : MAX_THUMB_SIZE;
                argc -= 3;
                argv += 3;
            } else {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
        } else if(strcmp("-small_res", argv[0])) {
            if(argc > 3) {
                small_res_X = (atouint16(argv[1]) < small_res_X) ? atouint16(argv[1]) : MAX_SMALL_SIZE;
                small_res_Y = (atouint16(argv[2]) < small_res_Y) ? atouint16(argv[2]) : MAX_SMALL_SIZE;
                argc -= 3;
                argv += 3;
            } else {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }

    int errcode = do_create(filename, &myfile, max_files, thumb_res_X, thumb_res_Y, small_res_X, small_res_Y);
    if(errcode == 0) {
        print_header(&myfile.header);
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
    printf("\t\t\t\t\t-max_files <MAX_FILES>: maximum number of files.\n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdefault value is 10\n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tmaximum value is 100000\n");
    printf("\t\t\t\t\t-thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdefault value is 64x64n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tmaximum value is 128x128n");
    printf("\t\t\t\t\t-small_res <X_RES> <Y_RES>: resolution for small images.n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdefault value is 256x256\n");
    printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tmaximum value is 512x512\n");
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

    if(strlen(argv[1]) > MAX_PIC_ID || strlen(argv[1]) == 0) { //first of all, test validity
        return ERR_INVALID_PICID;
    }
    struct pictdb_file myfile;
    int errcode = do_open(argv[0], "r+b", &myfile); //try to open
    if(errcode == 0) {
        errcode = do_delete(argv[1], &myfile); //if opening worked, try to delete
        do_close(&myfile); //indepently of delete, always close the stream
    }
    return errcode;
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
    (command_mapping) {
        "delete", do_delete_cmd
    }
                                            };
    int ret = 0;
    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 08: THIS PART SHALL BE REVISED THEN (WEEK 09) EXTENDED.
         * **********************************************************************
         */
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
