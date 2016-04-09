echo "Effacement des fichier .o"
rm done/pictDBM/*.o

echo "Reset des fichiers statics"
cp provided/week04/testDB01.pictdb_static done/pictDBM/
cp provided/week04/testDB02.pictdb_static done/pictDBM/

echo "Effacement des fichiers compilés"
rm done/pictDBM/pictDBM

echo "Effacement des fichiers de sauvegarde"
rm *~
rm done/pictDBM/*~
rm .gitignore~


echo -e "Fin du nettoyage du repository :D\n"

