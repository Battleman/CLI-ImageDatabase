/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 */
 #include "pictDB.h"
 #include "image_content.h"
 /**@brief Effectue un garbage collector sur une base de donnée
  * 
  * Copie les données nécessaires dans un fichier temporaire pour ensuite remplacer l'original
  * 
  * @param db_file La base de donnée en local
  * @param original_file Le nom du fichier original (le fichier correspondant est déjà ouvert !)
  * @param new_file Le nom du nouveau fichier, à ouvrir
  * 
  * @return 0 en cas de succès, un code d'erreur sinon
  */
int do_gbcollect(struct pictdb_file* original_db_file, const char* original_filename, const char* tmp_filename) {
	
	if(	original_db_file == NULL || 
		original_filename == NULL || !strcmp(original_filename, "") || 
		tmp_filename == NULL || !strcmp(tmp_filename, "")) return ERR_INVALID_ARGUMENT;
	struct pictdb_file tmp_db_file; // La base de données intermédiaire
    int errcode = 0;
    
    //création de la DB temporaire 
    if(0 != (errcode = do_create(	tmp_filename, &tmp_db_file, original_db_file->header.max_files,
									original_db_file->header.res_resized[2*RES_THUMB], original_db_file->header.res_resized[2*RES_THUMB + 1], 
									original_db_file->header.res_resized[2*RES_SMALL], original_db_file->header.res_resized[2*RES_SMALL + 1]
								)
			)
		) return errcode;
		
	do_open(tmp_filename, "rb+", &tmp_db_file);
	//pour toutes les images valides
	size_t index_tmp = 0;
	for(size_t i = 0; i < original_db_file->header.max_files; i++) {
		if(original_db_file->metadata[i].is_valid == NON_EMPTY) {
			char* img = NULL;
			uint32_t img_size = 0;
			do_read(original_db_file->metadata[i].pict_id, RES_ORIG, &img, &img_size, original_db_file);
			do_insert(original_db_file->metadata[i].pict_id, img, img_size, &tmp_db_file);
			if(tmp_db_file.metadata[index_tmp].offset[RES_SMALL] != 0 || original_db_file->metadata[i].offset[RES_SMALL] != 0)
				if(0 != (errcode = lazily_resize(RES_SMALL, &tmp_db_file, index_tmp))) return errcode;
			if(tmp_db_file.metadata[index_tmp].offset[RES_THUMB] != 0 || original_db_file->metadata[i].offset[RES_SMALL] != 0)
				if(0 != (errcode = lazily_resize(RES_THUMB, &tmp_db_file, index_tmp))) return errcode;
			index_tmp++;
		}
	}	
	do_close(&tmp_db_file);
	//remplacement du temp par l'original
	if(0 != remove(original_filename) || 0 != rename(tmp_filename, original_filename)) return ERR_IO;
	return 0;
}
