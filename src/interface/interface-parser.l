/*
 * Copyright (C) 2002-3 Simon Howard
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

%option prefix="irmo_interface_parser"
%option outfile="lex.yy.c"
%option noyywrap

%{
//
// Interface File Parser
//
// Comments for options above:
//
// prefix:  Change the prefix for the flex variables and function names.
//          this is to avoid conflict with other flex parsers
// outfile: automake expects the output file to be the default 'lex.yy.c'
//          changing the prefix changes the default output filename, so
//          this changes it back
// noyywrap: Only read one file at a time
// 

#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"
#include "interface.h"

typedef enum {
	TOKEN_NONE,
	TOKEN_CLASS,
	TOKEN_FUNC,
	TOKEN_INT8,
	TOKEN_INT16,
	TOKEN_INT32,
	TOKEN_STRING,
	TOKEN_ID,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LCURLY,
	TOKEN_RCURLY,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_COMMA,
} token_t;

#define YY_NO_UNPUT 
#define YY_DECL static token_t yylex()
 
// redefine YY_INPUT to count line numbers

#define YY_INPUT(buf,result,max_size) 					\
	{ 								\
		int c = fgetc(yyin); 					\
		if (c == '\n') ++line_num;				\
		result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); 	\
	}
 
static void parse_assert(int expr, char *msg, ...);
static jmp_buf parse_error_env;

typedef void (*cleanup_func)(void *data);

#define CLEANUP_STACK_SIZE 10

static struct {
	cleanup_func func;
	void *data;
} cleanup_stack[CLEANUP_STACK_SIZE];
static int cleanup_stack_pos;
 
static char *yyin_filename;
static int line_num;

%}



ID [[:alpha:]_][[:alnum:]_]*
COMMENT1 "//".*\n
COMMENT2 "/*"([^*]|("*"[^/]))*"*/"
COMMENT {COMMENT1}|{COMMENT2}
WHITESPACE [[:space:]]+

%%

{WHITESPACE} 	;
{COMMENT}	;
"class"		return TOKEN_CLASS;
"func"|"method"	return TOKEN_FUNC;
"int8"		return TOKEN_INT8;
"int16"		return TOKEN_INT16;
"int32"		return TOKEN_INT32;
"IrmoObjectID"  return TOKEN_INT16;
"string"	return TOKEN_STRING;
{ID}		return TOKEN_ID;
":"		return TOKEN_COLON;
";"		return TOKEN_SEMICOLON;
","		return TOKEN_COMMA;
"("		return TOKEN_LPAREN;
")"		return TOKEN_RPAREN;
"{"		return TOKEN_LCURLY;
"}"		return TOKEN_RCURLY;
.		parse_assert(0, "parse error");

%%

// add some data to the stack of things to free back

static void cleanup_push(void *func, void *data)
{
	assert(cleanup_stack_pos < CLEANUP_STACK_SIZE);

	cleanup_stack[cleanup_stack_pos].func = (cleanup_func) func;
	cleanup_stack[cleanup_stack_pos].data = data;
	++cleanup_stack_pos;
}

static void cleanup_pop()
{
	--cleanup_stack_pos;
}

// test a condition and bomb out with an error
// message if it is false

static void parse_assert(int expr, char *msg, ...)
{
	va_list l;

	if (!expr) {
		char *error_string;
		int i;

		va_start(l, msg);

		error_string = irmo_vasprintf(msg, l);

		irmo_error_report("irmo_interface_new", "%s: %i: %s",
				  yyin_filename, line_num,
				  error_string);

		free(error_string);

		va_end(l);

		// now free back all the data we were working on to 
		// clean up

		for (i=0; i<cleanup_stack_pos; ++i)
			cleanup_stack[i].func(cleanup_stack[i].data);

		longjmp(parse_error_env, 1);
	}
}

static int is_type_token(int i)
{
	return i == TOKEN_INT8 || i == TOKEN_INT16 
	    || i == TOKEN_INT32 || i == TOKEN_STRING;
}

static IrmoValueType type_token_to_type(int i)
{
	switch(i) {
		case TOKEN_INT8: return IRMO_TYPE_INT8;
		case TOKEN_INT16: return IRMO_TYPE_INT16;
		case TOKEN_INT32: return IRMO_TYPE_INT32;
		case TOKEN_STRING: return IRMO_TYPE_STRING;
		default:
			return IRMO_TYPE_UNKNOWN;
	}
}

static int eat_class_var_statement(IrmoClass *klass)
{
	IrmoValueType vartype;
	token_t token;

	token = yylex();

	// end of class

	if (token == TOKEN_RCURLY)
		return 0;

	// type token
		
	parse_assert(is_type_token(token), 
		     "'%s' not a variable type", yytext);

	vartype = type_token_to_type(token);

	// read comma separated variable names

	do {
		IrmoClassVar *var;

		// variable name

		token = yylex();
		parse_assert(token == TOKEN_ID, "variable name expected");
	
		parse_assert(irmo_class_get_variable(klass, yytext) == NULL,
			     "multiple definitions of '%s'", yytext);
		parse_assert(klass->nvariables < MAX_VARIABLES,
			     "maximum of %i variables per class", MAX_VARIABLES);

		var = irmo_new0(IrmoClassVar, 1);
		var->type = vartype;
		var->index = klass->nvariables;
		var->name = strdup(yytext);
		var->parent = klass;

		// insert into class

		klass->variables[klass->nvariables++] = var;
		irmo_hash_table_insert(klass->variable_hash, var->name, var);

		token = yylex();

	} while (token == TOKEN_COMMA);

	parse_assert(token == TOKEN_SEMICOLON, "semicolon expected");

	return 1;
}

static IrmoClass *eat_class(IrmoInterface *iface)
{
	token_t token;
	IrmoClass *klass;
	int i;

	parse_assert(yylex() == TOKEN_ID, "class name expected");

	parse_assert(irmo_interface_get_class(iface, yytext) == NULL,
		     "multiple definitions of '%s'",
		     yytext);

	klass = irmo_new0(IrmoClass, 1); 
	klass->name = strdup(yytext);
	klass->variables = irmo_new0(IrmoClassVar *, MAX_VARIABLES);
	klass->nvariables = 0;
	klass->variable_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	klass->parent = iface;

	cleanup_push(_irmo_class_free, klass);

	token = yylex();

	// subclassing

	if (token == TOKEN_COLON) {
		IrmoClass *parent_class;

		token = yylex();

		parse_assert(token == TOKEN_ID, "name of parent class expected");

		parent_class = irmo_interface_get_class(iface, yytext);

		parse_assert(parent_class != NULL,
			     "unknown class '%s'", yytext);

		klass->parent_class = parent_class;

		// copy all variables

		for (i=0; i<parent_class->nvariables; ++i) {
			IrmoClassVar *var = parent_class->variables[i];

			klass->variables[klass->nvariables++] = var;
			irmo_hash_table_insert(klass->variable_hash, 
					    var->name, var);
		}

		// continue where we left off

		token = yylex();
	}
	
	parse_assert(token == TOKEN_LCURLY, "expected '{' open parentheses");

	while (eat_class_var_statement(klass));

	if (klass->nvariables < MAX_VARIABLES)
		klass->variables 
                   = irmo_renew(IrmoClassVar *, klass->variables, klass->nvariables);

	//printf("read class: name '%s', %i variables\n", klass->name, i);

	// this class is complete

	cleanup_pop();

	return klass;
}

static IrmoMethodArg *eat_method_arg(IrmoMethod *method)
{
	token_t token = yylex();
	IrmoMethodArg *arg;
	IrmoValueType argtype;

	if (token == TOKEN_RPAREN)
		return NULL;

	if (method->narguments > 0) {
		parse_assert(token == TOKEN_COMMA,
			     "comma expected between arguments");
		token = yylex();
	}
		
	// argument type

	parse_assert(is_type_token(token), 
		     "'%s' not a variable type", yytext);

	argtype = type_token_to_type(token);

	// argument name

	parse_assert(yylex() == TOKEN_ID, "expecting argument name");

	arg = irmo_new0(IrmoMethodArg, 1); 
	arg->name = strdup(yytext);
	arg->type = argtype;
	arg->index = method->narguments;
	arg->parent = method;

	cleanup_push(_irmo_method_arg_free, arg);

	parse_assert(irmo_method_get_argument(method, yytext) == NULL,
		     "multiple definitions of '%s'", yytext);

	parse_assert(method->narguments < MAX_ARGUMENTS,
		     "maximum of %i arguments to a method", MAX_ARGUMENTS);
		
	method->arguments[method->narguments++] = arg;
	irmo_hash_table_insert(method->argument_hash, arg->name, arg);

	cleanup_pop();

	return arg;
}

static IrmoMethod *eat_method(IrmoInterface *iface)
{
	IrmoMethod *method;
	
	parse_assert(yylex() == TOKEN_ID, "function name expected");

	parse_assert(irmo_interface_get_method(iface, yytext) == NULL,
		     "multiple definitions of '%s'", yytext);
	
	method = irmo_new0(IrmoMethod, 1); 
	method->name = strdup(yytext);
	method->arguments = irmo_new0(IrmoMethodArg *, MAX_ARGUMENTS);
	method->argument_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	method->narguments = 0;
	method->parent = iface;

	cleanup_push(_irmo_method_free, method);
	
	parse_assert(yylex() == TOKEN_LPAREN,
		     "expecting '(' open parentheses");

	while (eat_method_arg(method));

	if (method->narguments < MAX_ARGUMENTS) {
		method->arguments = irmo_renew(IrmoMethodArg *, 
                                               method->arguments,
                                               method->narguments);
	}

	parse_assert(yylex() == TOKEN_SEMICOLON,
		     "expecting semicolon at end of function prototype");

	// this method is complete

	cleanup_pop();

	//printf("read method: name '%s', %i arguments\n", method->name, i);
	return method;
}

static IrmoInterface *eat_interface()
{
	IrmoInterface *iface;
	int token;

	iface = irmo_new0(IrmoInterface, 1); 
	iface->classes = irmo_new0(IrmoClass *, MAX_CLASSES);
	iface->methods = irmo_new0(IrmoMethod *, MAX_METHODS);
	iface->nclasses = iface->nmethods = 0;
	iface->refcount = 1;
	
	iface->class_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	iface->method_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);

	cleanup_push(_irmo_interface_free, iface);

	while ((token = yylex())) {
		if (token == TOKEN_CLASS) {
			IrmoClass *klass;
			
			parse_assert(iface->nclasses < MAX_CLASSES,
				     "maximum of %i classes", MAX_CLASSES);

			klass = eat_class(iface);

			klass->index = iface->nclasses;
			iface->classes[iface->nclasses] = klass;

			++iface->nclasses;
			
			irmo_hash_table_insert(iface->class_hash,
					    klass->name,
					    klass);

		} else if (token == TOKEN_FUNC) {
			IrmoMethod *method;
			parse_assert(iface->nmethods < MAX_METHODS,
				     "maximum of %i methods", MAX_METHODS);

			method = eat_method(iface);

			method->index = iface->nmethods;
			iface->methods[iface->nmethods++] = method;
			irmo_hash_table_insert(iface->method_hash, method->name,
					    method);
		} else {
			parse_assert(0, "expecting class or method definiton");
		}
	}

	if (iface->nclasses < MAX_CLASSES)
		iface->classes = irmo_renew(IrmoClass *, iface->classes, iface->nclasses);
	if (iface->nmethods < MAX_METHODS)
		iface->methods = irmo_renew(IrmoMethod *, iface->methods, iface->nmethods);

	iface->hash = irmo_interface_hash(iface);

	// interface is complete

	cleanup_pop();
	
	return iface;
}

IrmoInterface *irmo_interface_new(char *filename)
{
	IrmoInterface *iface;

	irmo_return_val_if_fail(filename != NULL, NULL);

	yyin = fopen(filename, "r");

	if (!yyin) {
		irmo_error_report("irmo_interface_new",
				  "cannot open '%s'", filename);
		return NULL;
	}

	yyin_filename = filename;
	line_num = 1;

	// set up catching errors

	cleanup_stack_pos = 0;

	if (setjmp(parse_error_env)) 
		iface = NULL;		// an error occurred
	else
		iface = eat_interface();

	fclose(yyin);

	return iface;
}

