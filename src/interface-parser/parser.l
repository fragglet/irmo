/*
 * Copyright (C) 2002-2008 Simon Howard
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

#include <irmo/types.h>
#include <irmo/interface.h>
#include <irmo/interface-parser.h>

#include "arch/sysheaders.h"
#include "base/assert.h"
#include "base/error.h"

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

#define YY_INPUT(buf, result, max_size)            \
        read_data(buf, &(result), max_size) 

static void read_data(char *buf, int *result, int max_size);
static void parse_assert(int expr, char *msg, ...);
static jmp_buf parse_error_env;

static char *yyin_filename;
static int line_num;

static IrmoInterfaceReadFunc input_callback;
static void *input_callback_handle;

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


// test a condition and bomb out with an error
// message if it is false

static void parse_assert(int expr, char *msg, ...)
{
	va_list l;

	if (!expr) {
		char *error_string;

		va_start(l, msg);

		error_string = irmo_vasprintf(msg, l);

		irmo_error_report("irmo_interface_new", "%s: %i: %s",
				  yyin_filename, line_num,
				  error_string);

		free(error_string);

		va_end(l);

		longjmp(parse_error_env, 1);
	}
}

// Read data from the input file.

static void read_data(char *buf, int *result, int max_size)
{
	int i;
	int status;

	// Temporary hack; read one character at a time so that
	// we don't lose track of which line we're reading.

	max_size = 1;

	// Invoke the read callback

	status = input_callback(buf, max_size, input_callback_handle);

	parse_assert(status >= 0, "error while reading from input");

	if (status == 0) {
		// End of file

		*result = YY_NULL;
	} else {
		// Check for newlines
		// TODO: stop at the first newline, so we don't 
		// lose track of the line we're on

		for (i=0; i<status; ++i) {
			if (buf[i] == '\n') {
				++line_num;
			}
		}
	
		*result = status;
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
	
                var = irmo_class_new_variable(klass, yytext, vartype);

                if (var == NULL) {
                        parse_assert(0, irmo_error_get());
                }

		token = yylex();

	} while (token == TOKEN_COMMA);

	parse_assert(token == TOKEN_SEMICOLON, "semicolon expected");

	return 1;
}

static IrmoClass *eat_class(IrmoInterface *iface)
{
	token_t token;
	IrmoClass *klass;
        IrmoClass *parent_class;
        char *class_name;

	parse_assert(yylex() == TOKEN_ID, "class name expected");

        class_name = strdup(yytext);

        // Read the next token.  If it is a colon, this is a subclass
        // of another class.  Otherwise, it is a top-level class.

	token = yylex();

	if (token == TOKEN_COLON) {

                // Read the parent class name

		token = yylex();

		parse_assert(token == TOKEN_ID, "name of parent class expected");

		parent_class = irmo_interface_get_class(iface, yytext);

		parse_assert(parent_class != NULL,
			     "unknown class '%s'", yytext);

                // Continue where we left off

		token = yylex();
        } else {
                parent_class = NULL;
        }

        // Create the new class

        klass = irmo_interface_new_class(iface, class_name, parent_class);

        if (klass == NULL) {
                parse_assert(0, irmo_error_get());
        }

        // This is no longer needed

        free(class_name);
	
        // We should have a '{' now.

	parse_assert(token == TOKEN_LCURLY, "expected '{' open parentheses");

        // Read all variable statements.

	while (eat_class_var_statement(klass));

	return klass;
}

static IrmoMethodArg *eat_method_arg(IrmoMethod *method)
{
	token_t token;
	IrmoMethodArg *arg;
	IrmoValueType arg_type;

        token = yylex();

        // End of method definition?

	if (token == TOKEN_RPAREN)
		return NULL;

        // First time, we just start off with the type of the first
        // variable.  After that, we expect a comma.

	if (irmo_method_num_arguments(method) > 0) {
		parse_assert(token == TOKEN_COMMA,
			     "comma expected between arguments");
		token = yylex();
	}
		
	// Argument type?

	parse_assert(is_type_token(token), 
		     "'%s' not a variable type", yytext);

	arg_type = type_token_to_type(token);

	// argument name

	parse_assert(yylex() == TOKEN_ID, "expecting argument name");

        // Create the new argument.

        arg = irmo_method_new_argument(method, yytext, arg_type);

        if (arg == NULL) {
                parse_assert(0, irmo_error_get());
        }

	return arg;
}

static IrmoMethod *eat_method(IrmoInterface *iface)
{
	IrmoMethod *method;
	
	parse_assert(yylex() == TOKEN_ID, "function name expected");

        method = irmo_interface_new_method(iface, yytext);

        if (method == NULL) {
                parse_assert(0, irmo_error_get());
        }

	parse_assert(yylex() == TOKEN_LPAREN,
		     "expecting '(' open parentheses");

	while (eat_method_arg(method));

	parse_assert(yylex() == TOKEN_SEMICOLON,
		     "expecting semicolon at end of function prototype");

	return method;
}

static void eat_interface(IrmoInterface *iface)
{
	int token;

        // Parse the file, reading class and method definitions.

	while ((token = yylex())) {
		if (token == TOKEN_CLASS) {
                        eat_class(iface);
		} else if (token == TOKEN_FUNC) {
                        eat_method(iface);
		} else {
			parse_assert(0, "expecting class or method definiton");
		}
	}
}

IrmoInterface *irmo_interface_parse(IrmoInterfaceReadFunc read_func,
                                    void *handle)
{
	IrmoInterface *iface;

	irmo_return_val_if_fail(read_func != NULL, NULL);

	input_callback = read_func;
	input_callback_handle = handle;
	line_num = 1;

        iface = irmo_interface_new();

	if (setjmp(parse_error_env)) {

                // An error occurred while parsing

                irmo_interface_unref(iface);
                iface = NULL;
	} else {
		eat_interface(iface);
        }

	return iface;
}

