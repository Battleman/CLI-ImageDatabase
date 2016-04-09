for i in *.c
do
	echo "Making $i sooo stylish"
	astyle -A8 $i
done

for i in *.h
do
	echo "Making $i sooo stylish"
	astyle -A8 $i
done
