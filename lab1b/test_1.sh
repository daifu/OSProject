cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!
a b<c > d
cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!

