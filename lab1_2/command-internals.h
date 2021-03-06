// UCLA CS 111 Lab 1 command internals

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

