cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!
cat > /etc/passwd
sort > /etc/passwd
