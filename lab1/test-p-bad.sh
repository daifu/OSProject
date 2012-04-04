#! /bin/sh

# UCLA CS 111 Lab 1 - Test that syntax errors are caught.

tmp=$0-$$.tmp
mkdir "$tmp" || exit
(
cd "$tmp" || exit
status=

# Sanity check, to make sure it works with at least one good example.
echo x >test0.sh || exit
../timetrash -p test0.sh >test0.out 2>test0.err || exit
echo '# 1
  x' >test0.exp || exit
diff -u test0.exp test0.out || exit
test ! -s test0.err || {
  cat test0.err
  exit 1
}

n=1
for bad in \
  '`' \
  '>' \
  '<' \
  'a >b <' \
  ';' \
  '; a' \
  'a ||' \
  'a
     || b' \
  'a
     | b' \
  'a
     ; b' \
  'a;;b' \
  'a&&&b' \
  'a|||b' \
  '|a' \
  '< a' \
  '&& a' \
  '||a' \
  '(a|b' \
  'a;b)' \
  '( (a)' \
  'a>>>b'
do
  echo "$bad" >test$n.sh || exit
  ../timetrash -p test$n.sh >test$n.out 2>test$n.err && {
    echo >&2 "test$n: unexpectedly succeeded for: $bad"
    status=1
  }
  test -s test$n.err || {
    echo >&2 "test$n: no error message for: $bad"
    status=1
  }
  n=$((n+1))
done

exit $status
) || exit

rm -fr "$tmp"
