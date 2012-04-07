// UCLA CS 111 Lab 1 command reading


#include "command.h"
#include "command-internals.h"
#include "alloc.h"


#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* #define debug false */

/* constant definition */
#define INIT_LENGTH 4

/* function declaration */
void init_command_stream(command_stream_t new_stream); // initialize an empty command stream
int is_word(char c); // check if a character is a word (word definition is in the lab manual)
int is_special_token(char); // check if a character is a special character (8 tokens in the lab manual)
int is_comment(char); // check if char is #
int is_white_space(char); // check if white space but NOT new line

// TODO: sub task, should be easy
command_t make_simple_command(command_stream_t* s); // make a simple command from the command stream passed in
command_t make_and_command(command_t a1, command_t a2); // make and command
command_t make_or_command(command_t o1, command_t o2); // make or command
command_t make_pipe_command(command_t p1, command_t p2); // make pipeline command
command_t make_sequence_command(command_t s1, command_t s2); // make sequence command
command_t make_subshell_command(command_t sub); // make subshell command 

// TODO: main tasks, do as the TA said in the discussion 
//			 see more detail in functions implementation below, you can
//			 change the type and number of argument as necessary
command_t read_simple_command(command_stream_t *s);
command_t read_and_or_command(command_stream_t *s);
command_t read_pipeline_command(command_stream_t *s);
command_t read_sequence_command(command_stream_t s);
command_t read_subshell_command(command_stream_t *s);

// helper functions
enum command_type read_command_type(char *command);

/* end function declaration section */


/* debug function */
void print_cmd_stream(command_stream_t s);

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
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
  
	//printf("c is %c\n", c);
  while (c != EOF)
  {
    // ignoring beginning white space
    while((c == ' ') || (c == '\t'))
		{
			c = get_next_byte(get_next_byte_argument);
		}
		// ignore comment, read until see an end-of-line
		if (c == '#')
		{
			//printf("Comment\n");
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
			new_stream->token_size = current_pos+1;
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
			/* printf("New stream's text: %s\n", new_stream->head); */
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
				printf("Read %c\n",c);
				// read next char, if not another &, output error
				c = get_next_byte(get_next_byte_argument);
				printf("Next char %c\n",c);
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
				c = get_next_byte(get_next_byte_argument);
			}
			else if(c == '|')
			{
				printf("Read %c\n",c);
				// read next char, if not another | or not a word, output error
				c = get_next_byte(get_next_byte_argument);
				printf("Next char %c\n",c);
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
					c = get_next_byte(get_next_byte_argument); 
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
			new_stream->token_size = current_pos+1;
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

  // Adding the ending empty node to the tree
  command_stream_t empty_stream = checked_malloc(sizeof(struct command_stream));
  init_command_stream(empty_stream);
  tail_stream->next = empty_stream;
  empty_stream->prev = tail_stream;


	//printf("Head stream: %s\n", head_stream->head);
#ifdef debug
	//print_cmd_stream(head_stream);
	command_stream_t* curr = NULL;
	command_t test = make_simple_command(&head_stream);
	int i = 0;
	while(test->u.word[i] != 0)
	{
		printf("%s ",test->u.word[i]);
		i++;
	}
	printf("\n");
	if(curr == NULL)
		printf("NULL\n");
	else
		head_stream = (*curr);
	printf("Head stream is %s\n", head_stream->head);
#endif
  return head_stream;
}

command_t
read_command_stream (command_stream_t *s)
{
  if ((*s)->head == NULL) {
    return NULL;
  }
  printf("Start reading command stream:.....\n");
  command_stream_t *ps = s;
  command_t comm = read_and_or_command(ps);
  printf("End reading command stream....\n");
  return comm;
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


/* TODO TASK HERE */
command_t
read_and_or_command(command_stream_t *s)
{
	// this should be the top function to call, the right order may be
	// read_pipeline()
	// error checking
	// make_and/make_or command
	// return
  // 1. read the pipeline command until && or || or end
  command_t cmd = read_pipeline_command(s);
  //Debug
  printf("Going back to read_and_or_command: ...\n");
  int is_and_or_cmd = read_command_type((*s)->head);
  if (is_and_or_cmd == AND_COMMAND || is_and_or_cmd == OR_COMMAND) {
    // 2. If the next command is && or ||, then make and_or command
    // Read the next command
    (*s) = (*s)->next;
    command_t next_cmd = read_pipeline_command(s);
    command_t and_or_cmd;
    printf("Going into read_and_or_command\n");
    if (is_and_or_cmd == AND_COMMAND) {
      and_or_cmd = make_and_command(cmd, next_cmd);
    } else if (is_and_or_cmd == OR_COMMAND){
      and_or_cmd = make_or_command(cmd, next_cmd);
    }
    printf("Command Type: %d\n", and_or_cmd->type);
    return and_or_cmd;
  }
  printf("Command Type: %d, Command Word: %s\n", cmd->type, *cmd->u.word);
	return cmd;
}


command_t 
read_pipeline_command(command_stream_t *s)
{
  // read the subshell command
  command_t cmd = read_subshell_command(s);
  // Debug
  if ((*s) != NULL) {
    printf("read_pipeline_command: next head: %d...\n", (*s)->size);
  } else {
    printf("NO COMMAND...\n");
  }
  if (read_command_type((*s)->head) == PIPE_COMMAND) {
    // Read the next command
    (*s) = (*s)->next;
    command_t next_cmd = read_subshell_command(s);
    command_t pipe_cmd = make_pipe_command(cmd, next_cmd);
    printf("Ending the read_pipeline_command...\n");
    return pipe_cmd;
  }
	return cmd;
}
command_t
read_simple_command(command_stream_t *s)
{
	// this is the lowest lever command
  printf("Comming to read_simple_command\n");
  command_t cmd = make_simple_command(s);
  //Debug
  printf("read_simple_command: next words: %s\n", (cmd->u.word)[0]);
	return cmd;
}

command_t 
read_sequence_command(command_stream_t s)
{
	// same as above
	return 0;
}
command_t 
read_subshell_command(command_stream_t *s)
{
  // read the simple command
  command_t cmd = read_simple_command(s);
  // TODO: implement subshell command
	return cmd;
}

// sub tasks
command_t 
make_simple_command(command_stream_t* s) // This one only pass syntax error NOT TESTED yet, use with your own risk
{
  printf("make_simple_command: next head: %s\n", (*s)->head);
  command_t result = (command_t)checked_malloc(sizeof(struct command));
  result->type = SIMPLE_COMMAND;
  result->status = -1;
  result->input = NULL;
  result->output = NULL;
  
  // get the word
  int max_size = INIT_LENGTH;
  int current_pos = 0;
  size_t resize = max_size;
  
  result->u.word = (char**)checked_malloc(sizeof(char*) * max_size);
  while((*s) != NULL)
  {
    //printf("s is not NULL\n");
    char c = (*s)->head[0];
    if(is_word(c))
    {
      //printf("Read %s\n", (*s)->head);
      if(current_pos == max_size) // text too big, need grow in size
        {
          max_size = max_size * 2;
          resize = max_size *sizeof(char);
          result->u.word = checked_grow_alloc(result->u.word, &resize);
        }
      result->u.word[current_pos] = (char*)checked_malloc((*s)->token_size * sizeof(char));
      result->u.word[current_pos] = memcpy(result->u.word[current_pos], (*s)->head, (*s)->token_size);
      current_pos++;
      //printf("Word after copied %s\n",result->u.word[current_pos-1]);
    }
    else // not word, break the loop
    {   
      //printf("Not word: Read %s\n", (*s)->head);
      if(current_pos == max_size) // text too big, need grow in size
        {
          max_size = max_size * 2;
          resize = max_size *sizeof(char);
          result->u.word = checked_grow_alloc(result->u.word, &resize);
        }
      result->u.word[current_pos] = 0;
      break;
    }
    if((*s) != NULL)
      (*s) = (*s)->next;
  }
  return result;
}

  
  
  
command_t 
make_and_command(command_t a1, command_t a2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = AND_COMMAND;
	result->status = -1;
	result->input = 0;
	result->output = 0;
	
	result->u.command[0] = a1;
	result->u.command[1] = a2;
	
	return result;
}
command_t 
make_or_command(command_t o1, command_t o2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = OR_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = o1;
	result->u.command[1] = o2;
	return result;
}
command_t 
make_pipe_command(command_t p1, command_t p2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = PIPE_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = p1;
	result->u.command[1] = p2;
	return result;
}
command_t 
make_sequence_command(command_t s1, command_t s2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = SEQUENCE_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = s1;
	result->u.command[1] = s2;

	return result;
}
command_t 
make_subshell_command(command_t sub)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = AND_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.subshell_command = sub;
	
	return result;
}

enum command_type read_command_type(char *command) {
  if (*command == 0) {
    return -1; //exit
  }
  else if (strcmp(command, "&&") == 0) {
    return AND_COMMAND;
  }
  else if (strcmp(command, "||") == 0) {
    return OR_COMMAND;
  }
  else if (*command == ';') {
    return SEQUENCE_COMMAND;
  }
  else if (*command ==  '|') {
    return PIPE_COMMAND;
  }
  else if (*command == '(') {
    // TODO: improve this, dont know how to match subshell correctly
    return SUBSHELL_COMMAND;
  }

  return SIMPLE_COMMAND;
}
/* END TODO 		 */


/* debug function */
void print_cmd_stream(command_stream_t s)
{
	int line_num = 1;
	while (s != NULL)
	{
		if(s->line_num != line_num)
		{
			printf("\n");
			line_num = s->line_num;
		}
		printf("%s ",s->head);
		s = s->next;
		
	}
	printf("\n");
}
