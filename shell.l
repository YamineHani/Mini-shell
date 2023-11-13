
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}

"<"		{
	    return LESS;    //new token for input redirection
	}

">>"	{
	    return GREATGREAT; //new token for append
	}

"&"	   {
	    return AMPERSAND; //new token for background process
	}

"|"	   {
	    return PIPE; //new token for pipe
	}

"exit" {
	   return EXIT;
	}

">>&"	{
		return ERROR;
	}

[^|<>& \t\n][^|<>& \t\n]*   {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}



%%
	
