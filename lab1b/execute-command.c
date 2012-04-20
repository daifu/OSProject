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
void time_travel_mode(command_stream_t s); // time travle main function
void tt_cmd_analysis(command_t c); // tt stand for time travel, analyze a command with its io
void add_dependencies(command_t c, io_list_t file_list); // add io file of c into dependencies list 

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
		if(c->u.command[0]->status == 0)
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

// add io file of c into dependencies list 
// recursive call to add all the file from sub-command to the dependencies list too
void 
add_dependencies(command_t c, io_list_t file_list) 
{
	if (c->type == SIMPLE_COMMAND)
	{
		if(c->input != 0)
		{
			// TODO
		}
		if(c->output != 0)
		{
			// TODO
		}
		int i = 1;
		while( c->u.word[i]!= NULL)
    {
			i++;
		}
	}
	else if (c->type == SUBSHELL_COMMAND)
	{
		if(c->input != 0)
		{
			// TODO
		}
		if(c->output != 0)
		{
			// TODO
		}
		add_dependencies(c->u.subshell_command, file_list);
	}
	else if (c->type == AND_COMMAND || c->type == OR_COMMAND || c->type == PIPE_COMMAND || c->type == SEQUENCE_COMMAND)
	{
		add_dependencies(c->u.command[0], file_list);
		add_dependencies(c->u.command[1], file_list);
	}
	else // unregconized command type
	{
		error(1, 0, "Error: unregconized command type %d", c->type);
	}
}

void 
tt_cmd_analysis(command_t c) // tt stand for time travel, analyze a command with its io
{
	// create new cmd node
	command_list_t new_cmd = checked_malloc(sizeof(struct command_list));
	new_cmd->c = c;
	new_cmd->file_list = NULL;
	new_cmd->num_of_dependent = 0;
	new_cmd->cmd_num = 0;
	new_cmd->pid = 0;
	new_cmd-> next = NULL;

	// TODO	
}

void 
time_travel_mode(command_stream_t s) // time travle main function
{
	// TODO
}





