#/bin/sh -x

if [ $1 -z ]; then
	test_name=$1
else
	test_name="test_list"
fi

make -C ../
#gcc -o $test_name ${test_name}.c -L ../ -lmidiseq -I ../ -lasound
rm test_list; gcc -g -o test_list test_list.c -L ../ -lmidiseq -I ../ -lasound

echo
echo
./test_list
