for i in *.c
do
	echo -e"-----------------------\n"
	echo "Making $i sooo stylish"
	astyle -A8 $i
done

for i in *.h
do
	echo -e "-----------------------\n"
	echo "Making $i sooo stylish"
	astyle -A8 $i
done

for i in *.orig
do
	echo -e "-----------------------\nDéplacement de $i dans le dossier des originaux (remplacera le fichier de même nom si existant)"
	mv $i astyle_orig
done
