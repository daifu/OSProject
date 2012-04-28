# the output maynot be the same as sequential code
echo foo
echo hello > test.txt
echo hello2 > test2.txt
echo a > test2.txt
cat < test2.txt

# some more tests
echo t1 > test.txt
cat < test.txt
echo t2 > test.txt
cat < test.txt

rm test.txt
rm test2.txt
