/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 */
 #include <pictDB.h>
 
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
int do_gbcollect(const struct pictdb_file* original_db_file, const char* original_filename, const char* new_filename) {
	
	struct pictdb_file tmp_db_file; // La base de données intermédiaire
    int errcode = 0;
    size_t visited_sha_num = 0;
    
    if(0 != (errcode = do_create(new_filename, &tmp_db_file, original_db_file.header.max_files, original_db_file.header.res_resized[RES_THUMB],
    db_file.header.res_resized[RES_THUMB + 1], db_file.header.res_resized[RES_SMALL], db_file.header.res_resized[RES_SMALL + 1]))) return errcode;
		
	return 0;
}
