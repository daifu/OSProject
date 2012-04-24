// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <unistd.h>

void exec_command_helper (command_t c); // return command success status


// time travel function declaration
//void time_travel_mode(command_stream_t s); // time travle main function
void tt_cmd_analysis(command_t c); // tt stand for time travel, analyze a command with its io
void add_dependencies(command_t c, command_list_t cmd_list); // add io file of c into dependencies list 
void add_file_to_list(char* name, command_list_t cmd_list, enum file_state state); // add io file to list

//io_list_t file_list = NULL;

int
command_status (command_t c) // 0 = no error, other num = error
{
  return c->status;
}

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command execution not yet implemented");
	if (time_travel == 0)
	{
		exec_command_helper(c);
	}
	else
	{
		error(1, 0, "Error: time travel did not implemented");
		// debug
		//tt_cmd_analysis(c);
		
	}
}

void 
exec_command_helper (command_t c)
{
	if (c->type == AND_COMMAND)
	{
		// Need more work
		exec_command_helper (c->u.command[0]);
		if(c->u.command[0]->status == 0)
		{
			printf("type = AND, c[0] exited success\n");
			exec_command_helper(c->u.command[1]);
			c->status = c->u.command[1]->status;
		}
		else
			c->status = c->u.command[0]->status;  	
	}
	else if (c->type == OR_COMMAND)
	{
		// Need more work
		exec_command_helper (c->u.command[0]);
		if(c->u.command[0]->status != 0)
		{
			//printf("type = OR, c[0] exited success\n");
			exec_command_helper(c->u.command[1]);
			c->status = c->u.command[1]->status;
		}
		else
			c->status = c->u.command[0]->status;  	
	}
	else if (c->type == SUBSHELL_COMMAND)
	{
		// io process
		if (c->input != 0)
		{
			int in_fd = open(c->input,O_RDONLY);
			if(in_fd == -1)
			{
				error(1, 0, "Error: open input file %s", c->input);
			}
			if ( dup2(in_fd,0) < 0 )
				error(1, 0, "Error: dup2() on %s error", c->input);
			if ( close(in_fd) < 0 )
				error(1, 0, "Error: close %s", c->input);
		}
		if (c->output != 0)
		{
			int out_fd = open(c->output,O_CREAT | O_WRONLY | O_TRUNC);
			if(out_fd == -1)
			{
				error(1, 0, "Error: open output file %s", c->output);
			}
			if ( dup2(out_fd,1) < 1 )
				error(1, 0, "Error: dup2() on %s error", c->output);
			if ( close(out_fd) < 0 )
				error(1, 0, "Error: close %s", c->output);
		}
		
		// execute
		exec_command_helper (c->u.subshell_command);
		c->status = c->u.subshell_command->status;
	}
	else if (c->type == SEQUENCE_COMMAND)
	{
		// Need more work
		exec_command_helper (c->u.command[0]);
		if(c->u.command[0]->status == 0)
		{
			//printf("type = SEQUENCE, c[0] exited success\n");
			exec_command_helper(c->u.command[1]);
			c->status = c->u.command[1]->status;
		}
		else
			c->status = c->u.command[0]->status;  	
	}
	else if (c->type == SIMPLE_COMMAND)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			// io process
			if (c->input != 0)
			{
				int in_fd = open(c->input,O_RDONLY);
				if(in_fd == -1)
				{
					error(1, 0, "Error: open input file %s", c->input);
				}
				if ( dup2(in_fd,0) < 0 )
					error(1, 0, "Error: dup2() on %s error", c->input);
				if ( close(in_fd) < 0 )
					error(1, 0, "Error: close %s", c->input);
			}
			if (c->output != 0)
			{
				int out_fd = open(c->output,O_CREAT | O_WRONLY | O_TRUNC);
				if(out_fd == -1)
				{
					error(1, 0, "Error: open output file %s", c->output);
				}
				if ( dup2(out_fd,1) < 1 )
					error(1, 0, "Error: dup2() on %s error", c->output);
				if ( close(out_fd) < 0 )
					error(1, 0, "Error: close %s", c->output);
			}
			
			// execute command
			execvp(c->u.word[0], c->u.word );
			c->status == 0;
			error(1, 0, "Error: execute simple command\n"); // if exec returns, means error			
		}
		else if (pid >0) // child not terminated
		{
			int status = 0;
			waitpid(pid, &status, 0); // parent wait for child process pid to terminated
			c->status = status;
		}
		else // error
		{
			error(1, 0, "Fork error");
		}
		// execute command
	}
	else if (c->type == PIPE_COMMAND)
	{
		// TODO
		// this is hard, need careful testing
		// take command[0] -> pipe -> command[1] or [0] is parent
		int pipefd[2];
		if (pipe(pipefd) == -1) // cannot pipe
			error(1, 0, "pipe() error");
		pid_t pid = fork();
		
		if (pid == 0) // child process [1]
		{
			close(pipefd[1]);
      if( dup2(pipefd[0], 0)== -1 )
        error (1, 0, "dup2() error");
      exec_command_helper(c->u.command[1]);

			_exit(1); // no flush
		}
		else if (pid > 0) // parent process [0]
		{
			pid_t pid2 = fork();
			
			if (pid2 == 0)
			{
				close(pipefd[0]);
      	if( dup2(pipefd[1], 1) == -1 )
        	error (1, 0, "dup2 error");
      	exec_command_helper(c->u.command[0]);
			}
			else if (pid2 > 0) // parent process, need to wait
			{
				int status;
				close(pipefd[0]);
				close(pipefd[1]);
				pid_t wait_pid = waitpid(-1, &status, 0); //wait
				if(wait_pid == pid )
      	{
        	c->status = status;
        	waitpid(pid2, &status, 0);
      	}
      	else if(wait_pid == pid2)
      	{
       	 	waitpid(pid, &status, 0);
        	c->status = status;
      	}
				else
					error(1, 0, "Unexpected error");
			}
			else
				error(1, 0, "fork() error");
		}
		else // error
		{
			error(1, 0, "fork() error");
		}

	}
	else
	{
		error(1, 0, "Unrecognizable command type");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// TIME TRAVEL MODE STARTING FROM HERE //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INIT_SIZE 20

// look-up table
// a[1][2] means 2 needs to be completed before 1 or 1 requires 2
int** dependent_array;

void initialize_dependent_array()
{
	dependent_array = (int**)checked_malloc(INIT_SIZE * sizeof(int*));
	int i;
	for (i = 0; i < INIT_SIZE; i++)
	{
		dependent_array[i] = (int*)checked_malloc(INIT_SIZE * sizeof(int));
	}

	int j;
	for (j = 0; j < INIT_SIZE; j++)
	{
		int k;
		for (k = 0; k < INIT_SIZE; k++)
		{
			dependent_array[j][k] = -1;
		}
	}
}

// add io file to list
void
add_file_to_list(char* name, command_list_t cmd_list, enum file_state state)
{
	printf("Add: %s\n", name);
	if(cmd_list->file_list == NULL) // list is empty
	{
		printf("List is NULL\n");
		cmd_list->file_list = checked_malloc(sizeof(struct io_list));
		cmd_list->file_list->name = name;
		cmd_list->file_list->state = state;
		cmd_list->file_list->next = NULL;
		//return file_list;
		printf("List head: %s\n", cmd_list->file_list->name);
	}
	else
	{
		io_list_t curr = cmd_list->file_list;
		io_list_t tmp = curr;
		while(curr != NULL)
		{
			if(strcmp(curr->name, name) == 0) // file already in the list
				return;
			else
			{
				tmp = curr;
				curr = curr->next;
			}
		}
		// go here means file not in the list
		curr = checked_malloc(sizeof(struct io_list));
		curr->name = name;
		curr->state = state;
		curr->next = NULL;
		tmp->next = curr;
		printf("List head: %s\n", cmd_list->file_list->name);
		//return file_list;
	}
}
 
// add io file of c into dependencies list 
// recursive call to add all the file from sub-command to the dependencies list too
void
add_dependencies(command_t c, command_list_t cmd_list) 
{
	if (c->type == SIMPLE_COMMAND)
	{
		if(c->input != 0)
		{
			// TODO
			printf("Input: %s\n", c->input);
			add_file_to_list(c->input, cmd_list, IS_READ);
			if(cmd_list->file_list == NULL)
				printf("NULL in add_dependencies\n");
		}
		if(c->output != 0)
		{
			// TODO
			printf("Output\n");
			add_file_to_list(c->output, cmd_list, IS_WRITTEN);
		}
		int i = 1;
		while( c->u.word[i]!= NULL)
    {
			add_file_to_list(c->u.word[i], cmd_list, IS_READ);
			i++;
		}
	}
	else if (c->type == SUBSHELL_COMMAND)
	{
		if(c->input != 0)
		{
			// TODO
			add_file_to_list(c->input, cmd_list, IS_READ);
		}
		if(c->output != 0)
		{
			// TODO
			add_file_to_list(c->output, cmd_list, IS_WRITTEN);
		}
		add_dependencies(c->u.subshell_command, cmd_list);
	}
	else if (c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND || c->type == SEQUENCE_COMMAND)
	{
		add_dependencies(c->u.command[0], cmd_list);
		add_dependencies(c->u.command[1], cmd_list);
	}
	else // unregconized command type
	{
		error(1, 0, "Error: unregconized command type %d", c->type);
	}
}

// analyze the new node with the current node that already on the list to
// determine the dependency of this new node with the current node
void 
analyze_dependencies(command_list_t new_node, command_list_t current_node)
{
	io_list_t new_file = new_node->file_list;
	
	// traverse through each dependent file on new node and check each for current node
	while (new_file != NULL)
	{
		io_list_t curr_file = current_node->file_list;
		while (curr_file != NULL)
		{
			if (strcmp(curr_file->name, new_file->name) == 0) // same file
			{
				if(new_file->state == IS_READ && curr_file->state == IS_WRITTEN) // new cmd requires input from a file that is written by others
				{
					new_node->num_of_dependent++;
					// TODO
					// need to update the look-up table
					dependent_array[new_node->cmd_num][current_node->cmd_num] = 1;
					return;
				}
				if(new_file->state == IS_WRITTEN)
				{
					new_node->num_of_dependent++;
					// TODO
					// need to update the look-up table
					dependent_array[new_node->cmd_num][current_node->cmd_num] = 1;
					return;
				}
			}
			curr_file = curr_file->next;
		}
		new_file = new_file->next;
	}
	dependent_array[new_node->cmd_num][current_node->cmd_num] = 0;
}

void 
tt_cmd_analysis(command_t c) // tt stand for time travel, analyze a command with its io
{
	// this piece of code should go to next function

	initialize_dependent_array();
	// create new cmd node
	command_list_t new_cmd = checked_malloc(sizeof(struct command_list));
	new_cmd->c = c;
	new_cmd->file_list = NULL;
	new_cmd->num_of_dependent = 0;
	new_cmd->cmd_num = 0;
	new_cmd->pid = 0;
	new_cmd-> next = NULL;

	// TODO	
	// need to update look up table
	add_dependencies(c, new_cmd);
		
	if(new_cmd->file_list == NULL)
			printf("NULL\n");
	printf("Command dependencies list: ");
	io_list_t curr = new_cmd->file_list;
	int i = 0;
	while(curr != NULL && i != 10)
	{
		printf("%s ", curr->name);
		curr = curr->next;
		i++;
	}
	printf("\n");
}

void 
time_travel_mode(command_stream_t command_stream) // time travle main function
{
	// TODO
	command_t command;
	while ((command = read_command_stream (command_stream)))
	{
		tt_cmd_analysis(command);
	}
}





