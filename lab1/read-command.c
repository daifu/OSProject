// UCLA CS 111 Lab 1 command reading


#include "command.h"
#include "command-internals.h"
#include "alloc.h"


#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* function declaration */
void init_command_stream(command_stream_t new_stream); // initialize an empty command stream
int is_word(char c); // check if a character is a word (word definition is in the lab manual)
int is_special_token(char); // check if a character is a special character (8 tokens in the lab manual)
int is_comment(char); // check if char is #
int is_white_space(char); // check if white space but NOT new line 
void print_command_stream(command_stream_t command_stream); // For testing
/* end function declaration section */

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // What it does:
  // Create a linked list of command lines, each object contains a line of 
  // command
  // Logic:
  // 1. create a command_stream object using alloc
  // 2. populate the object based on the get_next_byte(get_next_byte_argument)
  // 3. store a command line until \n
  // 4. keep track of the number of lines while looping trough the file_stream
  // 5. Ignored the comments and the empty lines
  int c, line_number = 1, char_count = 0, line_count = 0;
  char line[256][256];
  command_stream_t begin_stream = checked_malloc(sizeof(struct command_stream));
  command_stream_t * p_next_stream;
  // Init all the streams
  init_command_stream(begin_stream);

  while((c = get_next_byte(get_next_byte_argument))) {
    if (c == EOF || c < 0) {
      // End of the file
      /* printf("%c: EOF expected.\n", c); */
      break;
    }
    else if (c == '#') {
      // Ignored the whole line because it is comment
      while(get_next_byte(get_next_byte_argument) != '\n')
        continue;
      /* printf("line number meet #: %d\n", line_number); */
      line_number++;
    }
    else if (c == '\n') {
      // empty new line
      /* printf("line number meet \\n: %d\n", line_number); */
      line_number++;
    }
    else if (is_word(c)) {
      // If it starts with a word then store it
      while((c = get_next_byte(get_next_byte_argument)) != '\n' && c != '#') {
        line[line_count][char_count] = c;
        char_count++;
        continue;
      }
      if (c == '#') {
        // Ignored the comment following the commands
        while(get_next_byte(get_next_byte_argument) != '\n')
          continue;
      }
      line[line_count][char_count] = '\0';// Added null char
      if (begin_stream->size == 0) {
        // For the first time
        begin_stream->head = line[line_count];
        /* printf("line content: %s\n", begin_stream->head); */
        begin_stream->line_num = line_number;
        begin_stream->size = char_count + 1;

        // Appending the next node
        begin_stream->next = checked_malloc(sizeof(struct command_stream));
        p_next_stream = &begin_stream->next;
        init_command_stream(*p_next_stream);
        line_count++;
      } else {
        // Adding new command_stream to the next_command_stream
        (*p_next_stream)->head = line[line_count];
        /* printf("line content: %s\n", (*p_next_stream)->head); */
        (*p_next_stream)->line_num = line_number;
        (*p_next_stream)->size = char_count + 1;

        // Creating new command_stream to the next one
        (*p_next_stream)->next = checked_malloc(sizeof(struct command_stream));
        p_next_stream = &(*p_next_stream)->next;
        init_command_stream(*p_next_stream);
        line_count++;
      }
      char_count = 0; //reset
      line_number++;
    }
  }
  print_command_stream(begin_stream);
  return begin_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  // Logic
  // 1. Check if the command_stream_t s is END or not, if it is, return NULL
  // 2. If the command stream is not END, then process our logic with the stream
  // 3. Return the result
  while(s->next != NULL) {
   
  }
  return 0;
}

void
init_command_stream(command_stream_t new_stream)
{
  new_stream->head = (char*)checked_malloc(sizeof(char));
  new_stream->line_num = 0;
  new_stream->size = 0;
  new_stream->next = NULL;
  new_stream->prev = NULL;
}

int
is_word(char c)
{
  return isalpha(c) || isdigit(c) || (c == '!') || (c == '%') || (c == '+') || (c == ',')
                        || (c == '-') || (c == '.') || (c == '/') || (c == ':') || (c == '@') || (c == '^')
                        || (c == '_');
}

int 
is_special_token(char c) // check if a character is a special character (8 tokens in the lab manual)
{
  return (c == '|') || (c == '&') || (c == ';') || (c == '(') || (c == ')') || (c == '<') || (c == '>');
}

int 
is_comment(char c) // check if char is #
{
  return (c == '#');
}

void
print_command_stream(command_stream_t stream) {
  printf("Trying to pring out the command_stream.\n");
  while(stream->next != NULL) {
    printf("line number: %d, line content: %s\n", stream->line_num, stream->head);
    stream = stream->next;
  }
  return;
}
