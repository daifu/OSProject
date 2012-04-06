// UCLA CS 111 Lab 1 command reading


#include "command.h"
#include "command-internals.h"
#include "alloc.h"


#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define debug

/* constant definition */
#define INIT_LENGTH 4

/* function declaration */
void init_command_stream(command_stream_t new_stream); // initialize an empty command stream
int is_word(char c); // check if a character is a word (word definition is in the lab manual)
int is_special_token(char); // check if a character is a special character (8 tokens in the lab manual)
int is_comment(char); // check if char is #
int is_white_space(char); // check if white space but NOT new line 
/* end function declaration section */


/* debug function */
void print_cmd_stream(command_stream_t s);

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // TODO:
  // 1. create a command_stream object using alloc
  // 2. populate the object based on the get_next_byte(get_next_byte_argument)
  // 3. store a command line until \n
  // 4. keep track of the number of lines while looping trough the file_stream
  // 5. Ignored the comments and the empty lines
	command_stream_t head_stream = NULL;
	command_stream_t tail_stream = NULL;
  command_stream_t new_stream; 
	//checked_malloc(sizeof(struct command_stream));
  //init_command_stream(new_stream);


  int line_num = 1;
  char c = get_next_byte(get_next_byte_argument);
  
	printf("c is %c\n", c);
  while (c != EOF)
  {
    // ignoring beginning white space
    while((c == ' ') || (c == '\t'))
		{
			printf("White space \n");
			c = get_next_byte(get_next_byte_argument);
		}
		// ignore comment, read until see an end-of-line
		if (c == '#')
		{
			printf("Comment\n");
			while (c != '\n')
				c = get_next_byte(get_next_byte_argument);
		}

		int max_size = INIT_LENGTH;
		int current_pos = 0;
		size_t resize = max_size;
		// read everything else
		// printf("Reading %c\n", c);
		if (is_word(c))
		{	
			new_stream = checked_malloc(sizeof(struct command_stream));
  		init_command_stream(new_stream);
			while(is_word(c))
			{
				if(current_pos == max_size) // text too big, need grow in size
				{
					max_size = max_size * 2;
					resize = max_size *sizeof(char);
					new_stream->head = checked_grow_alloc(new_stream->head, &resize);
				}
				new_stream->head[current_pos] = c;
				current_pos++;
				c = get_next_byte(get_next_byte_argument);
			}
			
			new_stream->line_num = line_num;
			new_stream->head[current_pos] = 0;
			new_stream->prev = NULL;
			new_stream->next = NULL;
			// add new stream to the end of result stream
			if(head_stream == NULL)
				tail_stream = head_stream = new_stream;
			else
			{
				tail_stream->next = new_stream;
				new_stream->prev = tail_stream;
				tail_stream = new_stream;
			}
			
#ifdef debug
			printf("New stream's text: %s\n", new_stream->head);
#endif
			// reverse len and position
			max_size = INIT_LENGTH;
			current_pos = 0;
		}
		else if(is_special_token(c)) // deal with special token
		{
			new_stream = checked_malloc(sizeof(struct command_stream));
  		init_command_stream(new_stream);
			if(current_pos == max_size) // text too big, need grow in size
			{
				max_size = max_size * 2;
				resize = max_size *sizeof(char);
				new_stream->head = checked_grow_alloc(new_stream->head, &resize);
			}
			new_stream->head[current_pos] = c;
			current_pos++;
			//c = get_next_byte(get_next_byte_argument);
			if(c == '&')
			{
				// read next char, if not another &, output error
				c = get_next_byte(get_next_byte_argument);
				if(c != '&')
				{
					error(1,0, "Syntax error, missing & at linenum %i",line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else
				{
					//c = get_next_byte(get_next_byte_argument);
					new_stream->head[current_pos] = c;
					current_pos++;
				}
			}
			else if(c == '|')
			{
				// read next char, if not another | or not a word, output error
				c = get_next_byte(get_next_byte_argument);
				if(c != '|' && c != ' ' && c != '\n' && c != '\t' && !is_word(c))
				{
					error(1,0, "Syntax error, | at linenum %i",line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else if (c == '|')
				{
					//c = get_next_byte(get_next_byte_argument);
					new_stream->head[current_pos] = c;
					current_pos++;
				} 
			}
			else // other special char
			{
				// read next char, if not a word, output error
				c = get_next_byte(get_next_byte_argument);
				if(c != ' ' && c != '\n' && c != '\t' && !is_word(c))
				{
					error(1,0, "Syntax error, special char %c follow by unrecognized symbol at linenum %i", c,  line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else
				{
					
				}
			}
			new_stream->line_num = line_num;
			new_stream->head[current_pos] = 0;
			new_stream->prev = NULL;
			new_stream->next = NULL;
			// add new stream to the end of result stream
			if(head_stream == NULL)
				tail_stream = head_stream = new_stream;
			else
			{
				tail_stream->next = new_stream;
				new_stream->prev = tail_stream;
				tail_stream = new_stream;
			}

			// reverse len and position
			max_size = INIT_LENGTH;
			current_pos = 0;
		}
		else if (c == '\n') // new line, increase line number
		{
			line_num++;
			c = get_next_byte(get_next_byte_argument);
		}
		else if (!is_word(c) && !is_special_token(c) && c != ' ' && c != '\n' && c != '\t' && c != EOF)
		{
			error(1, 0, "%i: unrecognized symbol %c.", line_num, c);
		}	
  }

	printf("Head stream: %s\n", head_stream->head);
#ifdef debug
	print_cmd_stream(head_stream);
#endif
  return head_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  // TODO: 
  // 1. Check if the command_stream_t s is END or not, if it is, return NULL
  // 2. If the command stream is not END, then process our logic with the stream
  // 3. Return the result
  //printf("Using the read_command_stream.\n");
  /* printf("command_stream max size: %d.\n", s->max_size); */
  return 0;
}

void
init_command_stream(command_stream_t new_stream)
{
  new_stream->head = (char*)checked_malloc(INIT_LENGTH * sizeof(char));
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

/* debug function */
void print_cmd_stream(command_stream_t s)
{
	while (s != NULL)
	{
		printf("%s ",s->head);
		s = s->next;
	}
	printf("\n");
}
