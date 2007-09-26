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

%option prefix="irmo_if_spec"
%option outfile="lex.yy.c"
%option noyywrap

%{
//
// Interface Specification File Parser
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
#include "if_spec.h"

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

		irmo_error_report("irmo_interface_spec_new", "%s: %i: %s",
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

static IrmoValueType type_token_to_spec(int i)
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

static void class_var_free(IrmoClassVar *var)
{
	free(var->name);
	free(var);
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

	vartype = type_token_to_spec(token);

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

static void class_free(IrmoClass *spec)
{
	int i;
	int start;

	irmo_hash_table_free(spec->variable_hash);

	// find the start of the range of variables to free
	// (dont free variables from parent class)

	if (spec->parent_class)
		start = spec->parent_class->nvariables;
	else
		start = 0;
	
	for (i=start; i<spec->nvariables; ++i)
		class_var_free(spec->variables[i]);

	free(spec->variables);
	free(spec->name);
	free(spec);
}

static unsigned int rotate(unsigned int i)
{
	return (i << 1) | (i >> 31);
}

static unsigned int class_hash(IrmoClass *spec)
{
	unsigned int hash = 0;
	int i;
	
	for (i=0; i<spec->nvariables; ++i) {
		hash = rotate(hash) +
			+ (spec->variables[i]->type ^
			   irmo_string_hash(spec->variables[i]->name));
	}

	hash ^= irmo_string_hash(spec->name);

	if (spec->parent_class)
		hash = rotate(hash) ^ spec->parent_class->index;
	
	return hash;
}

static IrmoClass *eat_class(IrmoInterfaceSpec *spec)
{
	token_t token;
	IrmoClass *klass;
	int i;

	parse_assert(yylex() == TOKEN_ID, "class name expected");

	parse_assert(irmo_interface_spec_get_class(spec, yytext) == NULL,
		     "multiple definitions of '%s'",
		     yytext);

	klass = irmo_new0(IrmoClass, 1); 
	klass->name = strdup(yytext);
	klass->variables = irmo_new0(IrmoClassVar *, MAX_VARIABLES);
	klass->nvariables = 0;
	klass->variable_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	klass->parent = spec;

	cleanup_push(class_free, klass);

	token = yylex();

	// subclassing

	if (token == TOKEN_COLON) {
		IrmoClass *parent_class;

		token = yylex();

		parse_assert(token == TOKEN_ID, "name of parent class expected");

		parent_class = irmo_interface_spec_get_class(spec, yytext);

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

static void method_arfree(IrmoMethodArg *arg)
{
	free(arg->name);
	free(arg);
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

	argtype = type_token_to_spec(token);

	// argument name

	parse_assert(yylex() == TOKEN_ID, "expecting argument name");

	arg = irmo_new0(IrmoMethodArg, 1); 
	arg->name = strdup(yytext);
	arg->type = argtype;
	arg->index = method->narguments;
	arg->parent = method;

	cleanup_push(method_arfree, arg);

	parse_assert(irmo_method_get_argument(method, yytext) == NULL,
		     "multiple definitions of '%s'", yytext);

	parse_assert(method->narguments < MAX_ARGUMENTS,
		     "maximum of %i arguments to a method", MAX_ARGUMENTS);
		
	method->arguments[method->narguments++] = arg;
	irmo_hash_table_insert(method->argument_hash, arg->name, arg);

	cleanup_pop();

	return arg;
}

static void method_free(IrmoMethod *spec)
{
	int i;
	
	irmo_hash_table_free(spec->argument_hash);

	for (i=0; i<spec->narguments; ++i) 
		method_arfree(spec->arguments[i]);
	
	free(spec->arguments);
	free(spec->name);
	free(spec);
}

static unsigned int method_hash(IrmoMethod *spec)
{
	unsigned int hash = 0;
	int i;

	for (i=0; i<spec->narguments; ++i) {
		hash = rotate(hash)
			+ (spec->arguments[i]->type ^
			   irmo_string_hash(spec->arguments[i]->name));
	}

	hash ^= irmo_string_hash(spec->name);
	
	return hash;
}

static IrmoMethod *eat_method(IrmoInterfaceSpec *spec)
{
	IrmoMethod *method;
	int i;
	
	parse_assert(yylex() == TOKEN_ID, "function name expected");

	parse_assert(irmo_interface_spec_get_method(spec, yytext) == NULL,
		     "multiple definitions of '%s'", yytext);
	
	method = irmo_new0(IrmoMethod, 1); 
	method->name = strdup(yytext);
	method->arguments = irmo_new0(IrmoMethodArg *, MAX_ARGUMENTS);
	method->argument_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	method->narguments = 0;
	method->parent = spec;

	cleanup_push(method_free, method);
	
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

static void irmo_interface_spec_free(IrmoInterfaceSpec *spec)
{
	int i;
	
	irmo_hash_table_free(spec->class_hash);
	irmo_hash_table_free(spec->method_hash);

	// free classes.
	// this must be done in reverse order, so classes lower in the
	// hierarchy are freed first

	for (i=spec->nclasses-1; i>=0; --i)
		class_free(spec->classes[i]);

	free(spec->classes);

	for (i=0; i<spec->nmethods; ++i)
		method_free(spec->methods[i]);

	free(spec->methods);
	free(spec);
}

static unsigned int irmo_interface_spec_hash(IrmoInterfaceSpec *spec)
{
	unsigned int hash = 0;
	int i;

	for (i=0; i<spec->nclasses; ++i)
		hash = rotate(hash) + class_hash(spec->classes[i]);

	for (i=0; i<spec->nmethods; ++i)
		hash = rotate(hash) + method_hash(spec->methods[i]);

	if (hash == 0)
		hash = 1;

	return hash;
}

static IrmoInterfaceSpec *eat_interface()
{
	IrmoInterfaceSpec *spec;
	int token;

	spec = irmo_new0(IrmoInterfaceSpec, 1); 
	spec->classes = irmo_new0(IrmoClass *, MAX_CLASSES);
	spec->methods = irmo_new0(IrmoMethod *, MAX_METHODS);
	spec->nclasses = spec->nmethods = 0;
	spec->refcount = 1;
	
	spec->class_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);
	spec->method_hash = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);

	cleanup_push(irmo_interface_spec_free, spec);

	while ((token = yylex())) {
		if (token == TOKEN_CLASS) {
			IrmoClass *class_spec;
			
			parse_assert(spec->nclasses < MAX_CLASSES,
				     "maximum of %i classes", MAX_CLASSES);

			class_spec = eat_class(spec);

			class_spec->index = spec->nclasses;
			spec->classes[spec->nclasses] = class_spec;

			++spec->nclasses;
			
			irmo_hash_table_insert(spec->class_hash,
					    class_spec->name,
					    class_spec);

		} else if (token == TOKEN_FUNC) {
			IrmoMethod *method;
			parse_assert(spec->nmethods < MAX_METHODS,
				     "maximum of %i methods", MAX_METHODS);

			method = eat_method(spec);

			method->index = spec->nmethods;
			spec->methods[spec->nmethods++] = method;
			irmo_hash_table_insert(spec->method_hash, method->name,
					    method);
		} else {
			parse_assert(0, "expecting class or method definiton");
		}
	}

	if (spec->nclasses < MAX_METHODS)
		spec->classes = irmo_renew(IrmoClass *, spec->classes, spec->nclasses);
	if (spec->nmethods < MAX_METHODS)
		spec->methods = irmo_renew(IrmoMethod *, spec->methods, spec->nmethods);

	spec->hash = irmo_interface_spec_hash(spec);

	// interface spec is complete

	cleanup_pop();
	
	return spec;
}

IrmoInterfaceSpec *irmo_interface_spec_new(char *filename)
{
	IrmoInterfaceSpec *spec;

	irmo_return_val_if_fail(filename != NULL, NULL);

	yyin = fopen(filename, "r");

	if (!yyin) {
		irmo_error_report("irmo_interface_spec_new",
				  "cannot open '%s'", filename);
		return NULL;
	}

	yyin_filename = filename;
	line_num = 1;

	// set up catching errors

	cleanup_stack_pos = 0;

	if (setjmp(parse_error_env)) 
		spec = NULL;		// an error occurred
	else
		spec = eat_interface();

	fclose(yyin);

	return spec;
}

void irmo_interface_spec_ref(IrmoInterfaceSpec *spec)
{
	irmo_return_if_fail(spec != NULL);

	++spec->refcount;
}

void irmo_interface_spec_unref(IrmoInterfaceSpec *spec)
{
	irmo_return_if_fail(spec != NULL);

	--spec->refcount;

	if (spec->refcount <= 0)
		irmo_interface_spec_free(spec);
}

