# Simple command tests
echo foo

# Output and input
echo hello > test.txt
echo hello2 > test2.txt
echo a > test2.txt
cat < test2.txt

# Pipe command tests
ls -a | echo pipe
echo foo | sort

# And or command
true || echo foo
false && echo foo
echo bar || expr 8 - 5
echo bar && expr 8 - 5

# Subshell command test
(echo subshell)

# combine test
echo yes > yes; echo no | cat < yes

rm test.txt
rm test2.txt
rm yes
