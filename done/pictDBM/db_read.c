#include "pictDB.h"

int do_read(const char pict_id[], const int RES, const char** image_buffer, uint32_t image_size, struct pictdb_file* file)
{
    size_t index = 0;
    int valid = 0, errcode = 0;

    while(valid == 0 && index < file -> header.max_files) {
        if(	1 == file -> metadata[index].is_valid &&
            !strncmp(&pict_id[0], &file -> metadata[index].pict_id[0], MAX_PIC_ID + 1)
          ) //if the image is valid and the the names match
            valid = 1;
        else
            ++index;
    }

    if(valid == 0) {
        errcode = ERR_FILE_NOT_FOUND;
    } else {
        if(file -> metadata[index].size[RES] == 0 && file -> metadata[index].offset[RES] == 0)
            errcode = lazily_resize(RES, file, index);
        image_size = file -> metadata[index].size[RES];
    }

    if(errcode == 0) {
        image_buffer = calloc(image_size, sizeof(char));
        if(image_buffer == NULL) {
            errcode = ERR_OUT_OF_MEMORY;
        } else {
            fseek(file -> fpdb, file -> metadata[index].offset[RES]);
            if(1 != fread(image_buffer, image_size, 1, file -> fpdb)) {
                errcode = ERR_IO;
            }
        }
    }

    return errcode;
}
