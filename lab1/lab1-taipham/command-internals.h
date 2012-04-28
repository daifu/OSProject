// UCLA CS 111 Lab 1 command internals
#include <stdlib.h>
enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
	// Not sure what is the differences between input and output
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};

// Data associated with a command stream
struct c_stream
{
  char* head; // head pointer of the stream
  unsigned int size;
  int token_size; 
  int line_num; // the line # of the command, use for error detection
  struct c_stream* next; // pointer to next command
  struct c_stream* prev; // pointer to previous command
	//char* next;
	//char* prev;
};

struct command_stream
{
	struct c_stream* s;
};

// data for time travel shell
enum file_state
{
	IS_READ, // file is read by others 
	IS_WRITTEN, // file is written by others
	IS_R_AND_W, // file is both read and written
	FREE, // file is free to process
};

// input, output list
struct io_list
{
	char* name;
	enum file_state state;
	struct io_list* next;
};
typedef struct io_list* io_list_t;

// struct that contain the require command to be executed before current command
struct require_list
{
	int cmd_num;
	int is_required; // 0 = no, 1 = yes
	struct requre_list* next;
};
typedef struct require_list* require_list_t;

// command list
struct command_list
{
	command_t c;
	
	io_list_t file_list;
	
	int num_of_dependent;
	int cmd_num; // command number, for testing purpose

	pid_t pid;
	struct command_list* next;
};
typedef struct command_list* command_list_t;


