
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token  GREAT LESS GREATGREAT AMPERSAND PIPE EXIT ERROR NEWLINE 

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

	
command: simple_command
		| EXIT{
			printf("GOODBYE!\n");
			exit(0);
		}
        ;

simple_command:	
	command_and_args error_opt io background_opt NEWLINE { //This means that a simple command can have an optional input, output, or append redirection (iomodifier_opt), an optional background process (background_opt), and a mandatory newline symbol (NEWLINE). The action for this rule will print a message and execute the command.
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;


command_and_args:
  command_and_args PIPE command_word arg_list {
    Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
    Command::_currentSimpleCommand = new SimpleCommand();
  }
  | command_word arg_list {
    Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
    Command::_currentSimpleCommand = new SimpleCommand();
  }
  ;



arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

io:
	io iomodifier_opt
	|
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD { // new rule for input redirection
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| GREATGREAT WORD { // new rule for append redirection
		printf("   Yacc: insert append output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1; //set falg append = 1
	}
	 /* can be empty */ 
	;

background_opt: // new rule for background process
	AMPERSAND {
		printf("   Yacc: insert background process\n");
		Command::_currentCommand._background = 1;
	}
	|
	;

error_opt: 
	ERROR WORD{  
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	| 
	;


%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
