#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt){
	fputs(prompt,stdout);
	fgets(buffer,2048,stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy,buffer);
	cpy[strlen(cpy)-1]='\0';
	return cpy;
}

void add_history(char* unused){ }

#else
#include <editline/readline.h>
#endif

/* Use operator string to see which operation to perform */
long eval_op(long x, char* op, long y){

	if(strcmp(op,"+")==0) { return x + y; }
	if(strcmp(op,"-")==0) { return x - y; }
	if(strcmp(op,"*")==0) { return x * y; }
	if(strcmp(op,"/")==0) { return x / y; }
	if(strcmp(op,"%")==0) { return x % y; }
	if(strcmp(op,"^")==0) {  
		int f=1;
		for(int i=0 ; i < y; i++){
			f=f*x;
		}
		return f;
	}

	return 0;
}

long eval(mpc_ast_t* t){

	/* Base case for recursion */
	if(strstr(t->tag,"number")){
		return atoi(t->contents);
	}

	/* First child is always "(" so evaluate second child onwards */
	char* op = t->children[1]->contents;

	/* We store the third child in `x` */
	long x = eval(t->children[2]);

	/* Iterate the remaining children and combining. */
	  int i = 3;
	  while (strstr(t->children[i]->tag, "expr")) {
	    x = eval_op(x, op, eval(t->children[i]));
	    i++;
	  }
	  
	  return x;   
}

int main(int argc, char const *argv[]){

	/* Create Some Parsers */
	mpc_parser_t* Number   = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr     = mpc_new("expr");
	mpc_parser_t* Lambda   = mpc_new("lambda");

	/* Define them with the following Language */
	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                                     \
	    number   : /-?[0-9]+/ ;                             \
	    operator : '+' | '-' | '*' | '/'| '%'| '^'|;                  \
	    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
	    lambda    : /^/ <operator> <expr>+ /$/ ;             \
	  ",
	  Number, Operator, Expr, Lambda);

	puts("Lambda Version 0.0.1");
	puts("Press Ctrl+C to exit");

	while(1){
		char* input = readline("lambda> ");

		add_history(input);
		
		/* Attempt to Parse the user Input */
		mpc_result_t r;
		if(mpc_parse("<stdin>",input,Lambda,&r)){
			/* On Success Print the AST */
			  long result = eval(r.output);
      		  printf("%li\n", result);
			  mpc_ast_delete(r.output);
			} else {
				/* Otherwise Print the Error */
				  mpc_err_print(r.error);
				  mpc_err_delete(r.error);
			}
		free(input);
	}
	
	/* Undefine and Delete our Parsers */
	mpc_cleanup(4,Number,Operator,Expr,Lambda);
	return 0;
}