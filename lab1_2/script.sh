# Qualitative correct script tests

#0    Check ignoring multiple comments in a row
#1    Sanity Check
a && b || c > files    #comment

#2    Check use of parenthesis
a || ( b && (c || d) || e ) > files #comment

#3    Check use of semicolon and parenthesis 
(cat a; cat b)

#4-5  Check use of semicolon, parenthesis, and pipes
a || b && (c || d) && e; f && g | h || i

#6    Check long token strings and memory allocation method (100 characters long)
abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde ||
abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde
