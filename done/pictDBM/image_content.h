#include "pictDB.h"

int lazily_resize(const int res, struct pictdb_file* file, size_t index);
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size);
