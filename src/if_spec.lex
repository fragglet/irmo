%{
//
// Interface Specification File Parser
//

#include <glib.h>
#include <stdarg.h>
#include <string.h>
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
	TOKEN_SEMICOLON,
	TOKEN_COMMA,
} token_t;

#define YY_DECL static token_t yylex()
 
// redefine YY_INPUT to count line numbers

#define YY_INPUT(buf,result,max_size) 					\
	{ 								\
		int c = fgetc(yyin); 					\
		if (c == '\n') ++line_num;				\
		result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); 	\
	}
 
static void parse_assert(int expr, char *msg, ...);
 
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
"string"	return TOKEN_STRING;
{ID}		return TOKEN_ID;
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
		va_start(l, msg);

		fprintf(stderr, "%s: %i: ", yyin_filename, line_num);
		vfprintf(stderr, msg, l);
		fprintf(stderr, "\n");

		va_end(l);

		exit(-1);
	}
}

static inline int is_type_token(int i)
{
	return i == TOKEN_INT8 || i == TOKEN_INT16 
	    || i == TOKEN_INT32 || i == TOKEN_STRING;
}

static inline TypeSpec type_token_to_spec(int i)
{
	switch(i) {
		case TOKEN_INT8: return TYPE_INT8;
		case TOKEN_INT16: return TYPE_INT16;
		case TOKEN_INT32: return TYPE_INT32;
		case TOKEN_STRING: return TYPE_STRING;
		default:
	}

	return 0;
}

static void class_free(ClassSpec *spec)
{
	int i;

	g_hash_table_destroy(spec->variable_hash);
	
	for (i=0; i<spec->nvariables; ++i) {
		free(spec->variables[i]->name);
		free(spec->variables[i]);
	}

	free(spec->variables);
	free(spec->name);
	free(spec);
}

static inline guint rotate(guint i)
{
	return (i << 1) | (i >> 31);
}

static guint class_hash(ClassSpec *spec)
{
	guint hash = 0;
	int i;
	
	for (i=0; i<spec->nvariables; ++i) {
		hash = rotate(hash) +
			+ (spec->variables[i]->type ^
			   g_str_hash(spec->variables[i]->name));
	}

	hash ^= g_str_hash(spec->name);
	
	return hash;
}

static ClassSpec *eat_class()
{
	ClassSpec *spec;
	int i;

	parse_assert(yylex() == TOKEN_ID, "class name expected");

	spec = g_new0(ClassSpec, 1); 
	spec->name = strdup(yytext);
	spec->variables = malloc(sizeof(ClassVarSpec *) * MAX_VARIABLES);

	spec->variable_hash = g_hash_table_new(g_str_hash, g_str_equal);
	
	parse_assert(yylex() == TOKEN_LCURLY, "expected '{' open parentheses");

	for (i=0; ;  ++i) {
		token_t token = yylex();
		ClassVarSpec *var;

		// end of class

		if (token == TOKEN_RCURLY)
			break;

		parse_assert(i < MAX_VARIABLES,
			     "maximum %i variables per class", MAX_VARIABLES);

		var = g_new0(ClassVarSpec, 1);

		// type token
		
		parse_assert(is_type_token(token), 
			     "'%s' not a variable type", yytext);
		var->type = type_token_to_spec(token);
	
		// variable name

		token = yylex();
		parse_assert(token == TOKEN_ID, "variable name expected");

		parse_assert(g_hash_table_lookup(spec->variable_hash, yytext)
			     == NULL,
			     "multiple definitions of '%s'", yytext);
		
		var->name = strdup(yytext);

		// ending semicolon

		parse_assert(yylex() == TOKEN_SEMICOLON, "semicolon expected");

		spec->variables[i] = var;
		g_hash_table_insert(spec->variable_hash, var->name, var);
	}

	spec->nvariables = i;

	if (i < MAX_VARIABLES)
		spec->variables 
		   = realloc(spec->variables, i * sizeof(ClassVarSpec *));

	printf("read class: name '%s', %i variables\n", spec->name, i);

	return spec;
}

static void method_free(MethodSpec *spec)
{
	int i;
	
	g_hash_table_destroy(spec->argument_hash);

	for (i=0; i<spec->narguments; ++i) {
		free(spec->arguments[i]->name);
		free(spec->arguments[i]);
	}
	
	free(spec->arguments);
	free(spec->name);
	free(spec);
}

static guint method_hash(MethodSpec *spec)
{
	guint hash = 0;
	int i;

	for (i=0; i<spec->narguments; ++i) {
		hash = rotate(hash)
			+ (spec->arguments[i]->type ^
			   g_str_hash(spec->arguments[i]->name));
	}

	hash ^= g_str_hash(spec->name);
	
	return hash;
}

static MethodSpec *eat_method()
{
	MethodSpec *spec;
	int i;
	
	parse_assert(yylex() == TOKEN_ID, "function name expected");
	
	spec = g_new0(MethodSpec, 1); 
	spec->name = strdup(yytext);
	spec->arguments = malloc(sizeof(MethodArgSpec *) * MAX_ARGUMENTS);

	spec->argument_hash = g_hash_table_new(g_str_hash, g_str_equal);
	
	parse_assert(yylex() == TOKEN_LPAREN,
		     "expecting '(' open parentheses");

	for (i=0;; ++i) {
		token_t token = yylex();
		MethodArgSpec *arg;

		if (token == TOKEN_RPAREN)
			break;

		if (i > 0) {
			parse_assert(token == TOKEN_COMMA,
				     "comma expected between arguments");
			token = yylex();
		}
		
		// argument type

		parse_assert(is_type_token(token), 
			     "'%s' not a variable type", yytext);

		arg = g_new0(MethodArgSpec, 1); 
		arg->type = type_token_to_spec(token);

		// argument name

		parse_assert(yylex() == TOKEN_ID, "expecting argument name");

		parse_assert(g_hash_table_lookup(spec->argument_hash, yytext)
			     == NULL,
			     "multiple definitions of '%s'", yytext);
		
		arg->name = strdup(yytext);
		
		spec->arguments[i] = arg;
		
		g_hash_table_insert(spec->argument_hash, arg->name, arg);
	}

	spec->narguments = i;
	if (i < MAX_ARGUMENTS) {
		spec->arguments = realloc(spec->arguments,
					  sizeof(MethodArgSpec) * i);
	}

	parse_assert(yylex() == TOKEN_SEMICOLON,
		     "expecting semicolon at end of function prototype");

	printf("read method: name '%s', %i arguments\n", spec->name, i);
	return spec;
}

static void interface_spec_free(InterfaceSpec *spec)
{
	int i;
	
	g_hash_table_destroy(spec->class_hash);
	g_hash_table_destroy(spec->method_hash);

	for (i=0; i<spec->nclasses; ++i)
		class_free(spec->classes[i]);

	free(spec->classes);

	for (i=0; i<spec->nmethods; ++i)
		method_free(spec->methods[i]);

	free(spec->methods);
	free(spec);
}

static guint interface_spec_hash(InterfaceSpec *spec)
{
	guint hash = 0;
	int i;

	for (i=0; i<spec->nclasses; ++i)
		hash = rotate(hash) + class_hash(spec->classes[i]);

	for (i=0; i<spec->nmethods; ++i)
		hash = rotate(hash) + method_hash(spec->methods[i]);

	return hash;
}

static InterfaceSpec *eat_interface()
{
	InterfaceSpec *spec;
	int token;

	spec = g_new0(InterfaceSpec, 1); 
	spec->classes = malloc(sizeof(ClassSpec *) * MAX_CLASSES);
	spec->methods = malloc(sizeof(MethodSpec *) * MAX_METHODS);
	spec->nclasses = spec->nmethods = 0;
	spec->refcount = 1;
	
	spec->class_hash = g_hash_table_new(g_str_hash, g_str_equal);
	spec->method_hash = g_hash_table_new(g_str_hash, g_str_equal);
	
	while ((token = yylex())) {
		if (token == TOKEN_CLASS) {
			ClassSpec *class;
			
			parse_assert(spec->nclasses < MAX_CLASSES,
				     "maximum of %i classes", MAX_CLASSES);

			class = eat_class();

			if (g_hash_table_lookup(spec->class_hash, class->name))
				parse_assert(0, "multiple definitions of '%s'",
					     class->name);
			
			spec->classes[spec->nclasses++] = class;
			g_hash_table_insert(spec->class_hash, class->name,
					    class);

		} else if (token == TOKEN_FUNC) {
			MethodSpec *method;
			parse_assert(spec->nmethods < MAX_METHODS,
				     "maximum of %i methods", MAX_METHODS);

			method = eat_method();
			
			if (g_hash_table_lookup(spec->method_hash,
						method->name))
				parse_assert(0, "multiple definitions of '%s'",
					     method->name);
			
			spec->methods[spec->nmethods++] = method;
			g_hash_table_insert(spec->method_hash, method->name,
					    method);
		} else {
			parse_assert(0, "expecting class or method definiton");
		}
	}

	if (spec->nclasses < MAX_METHODS)
		spec->classes = realloc(spec->classes,
					sizeof(ClassSpec *) * spec->nclasses);
	if (spec->nmethods < MAX_METHODS)
		spec->methods = realloc(spec->methods,
					sizeof(MethodSpec *) * spec->nmethods);

	spec->hash = interface_spec_hash(spec);
	
	return spec;
}

InterfaceSpec *interface_spec_new(char *filename)
{
	InterfaceSpec *spec;

	yyin = fopen(filename, "r");

	if (!yyin) {
		fprintf(stderr, "interface_spec_new: cant open '%s'",
			filename);
		exit(-1);
	}

	yyin_filename = filename;
	line_num = 1;

	spec = eat_interface();

	fclose(yyin);

	return spec;
}

void interface_spec_ref(InterfaceSpec *spec)
{
	++spec->refcount;
}

void interface_spec_unref(InterfaceSpec *spec)
{
	--spec->refcount;

	if (spec->refcount <= 0)
		interface_spec_free(spec);
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2002/10/19 18:57:23  sdh300
// recognise "method" as well as "func"
//
// Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
// initial import
//
