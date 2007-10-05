
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <irmo.h>
#include <irmo/interface-parser.h>

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

// Prefix to attach to the default C array names.

#define INTERFACE_PREFIX "interface_"

typedef enum {
        OUTPUT_AUTO,
        OUTPUT_BINARY,
        OUTPUT_C_ARRAY,
} OutputFormat;

OutputFormat output_format = OUTPUT_AUTO;
char *input_filename = NULL;
char *output_filename = NULL;
char *c_array_name = NULL;

void set_output_format(char *str)
{
        if (!strcmp(str, "binary")) {
                output_format = OUTPUT_BINARY;
        } else if (!strcmp(str, "carray")) {
                output_format = OUTPUT_C_ARRAY;
        } else {
                fprintf(stderr, "Unknown output format: '%s'\n", str);
                exit(-1);
        }
}

void usage(char *program)
{
        printf("Usage: %s [ options ] filename\n", program);

        printf("Options:\n"
               "\n"
               "   -f <format>        Specify the format of the output file:\n"
               "                        carray - Source code for a C array\n"
               "                        binary - Binary file (default)\n"
               "   -o <filename>      Specify the output filename.\n"
               "   -a <name>          In C array output format, specifies\n"
               "                      name of the array in the output file.\n"
               "\n"
               );

        exit(-1);
}

int ends_with(char *str, char *stem)
{
        int stem_length = strlen(stem);
        int str_length = strlen(str);

        return str_length >= stem_length
            && !strcmp(str + str_length - stem_length, stem);
}

// Set a default C array name from the input filename if none is specified

void set_c_array_name(void)
{
        char *filename;
        char *p;

        // Get the base filename

        p = strrchr(input_filename, PATH_SEPARATOR);

        if (p == NULL) {
                filename = input_filename;
        } else {
                filename = p + 1;
        }

        // Strdup so we can modify the filename

        c_array_name = malloc(strlen(filename) 
                              + strlen(INTERFACE_PREFIX) + 1);

        strcpy(c_array_name, INTERFACE_PREFIX);
        strcat(c_array_name, filename);

        // Cut off file extension

        p = strchr(c_array_name, '.');

        if (p != NULL) {
                *p = '\0';
        }

        // Convert all characters that are not valid characters for an
        // identifier into _ characters.

        for (p=c_array_name; *p != '\0'; ++p) {
                if (!isalnum(*p) && *p != '_') {
                        *p = '_';
                }
        }
}

void parse_cmd_line(int argc, char *argv[])
{
        char *arg;
        char *param;
        int ignore_args = 0;
        int i;

        for (i=1; i<argc; ++i) {
                arg = argv[i];
                param = NULL;

                if (arg[0] == '-' 
                 && (arg[1] == 'o' || arg[1] == 'f' || arg[1] == 'a')) {
                        if (arg[2] == '\0') {
                                if (i + 1 < argc) {
                                        param = argv[i + 1];
                                        ++i;
                                } else {
                                        usage(argv[0]);
                                }
                        } else {
                                param = arg + 2;
                        }
                }

                if (arg[0] != '-') {
                        if (input_filename == NULL) {
                                input_filename = arg;
                                continue;
                        } else {
                                usage(argv[0]);
                        }
                }

                switch (arg[1]) {
                        case 'a':            // -a <name>
                                c_array_name = param;
                                break;
                        case 'f':            // -f <format>
                                set_output_format(param);
                                break;
                        case 'h':            // -h
                                usage(argv[0]);
                                break;
                        case 'o':            // -o <filename>
                                output_filename = param;
                                break;
                        case '-':            // --
                                ignore_args = 1;
                                break;
                        default:
                                usage(argv[0]);
                                break;
                }
        }

        if (input_filename == NULL) {
                usage(argv[0]);
        }

        if (output_filename == NULL) {
                output_filename = "interface.out";
        }

        if (output_format == OUTPUT_AUTO) {
                if (ends_with(output_filename, ".c") 
                 || ends_with(output_filename, ".cpp")
                 || ends_with(output_filename, ".m")) {
                        output_format = OUTPUT_C_ARRAY;
                } else {
                        output_format = OUTPUT_BINARY;
                }
        }

        if (output_format == OUTPUT_C_ARRAY
         && c_array_name == NULL) {
                set_c_array_name();
        }
}

void write_binary_file(char *filename, void *buf, unsigned int buf_len)
{
        FILE *output;

        output = fopen(filename, "wb");

        fwrite(buf, 1, buf_len, output);

        fclose(output);
}

void write_c_array_file(char *filename, void *buf, unsigned int buf_len)
{
        FILE *output;
        unsigned char *data;
        unsigned int i;

        output = fopen(filename, "w");

        data = buf;

        fprintf(output, "unsigned char %s[] =\n{", c_array_name);

        for (i=0; i<buf_len; ++i) {
                if ((i % 8) == 0) {
                        // Start a new line

                        fprintf(output, "\n\t");
                } 

                fprintf(output, "0x%02x", data[i]);

                if (i != buf_len - 1) {
                        fprintf(output, ", ");
                }
        }

        fprintf(output, "\n};\n\n");

        fprintf(output, "unsigned int %s_length = %i;\n\n",
                        c_array_name, buf_len);

        fclose(output);
}

void do_compile(void)
{
        IrmoInterface *iface;
        void *buf;
        unsigned int buf_len;

        // Parse the input file

        iface = irmo_interface_parse(input_filename);

        if (iface == NULL) {
                fprintf(stderr, "%s:%s\n", input_filename, irmo_error_get());
                exit(-1);
        }

        // Serialize into a buffer

        irmo_interface_dump(iface, &buf, &buf_len);

        // Write to file

        switch (output_format) {
                case OUTPUT_AUTO:
                case OUTPUT_BINARY:
                        write_binary_file(output_filename, buf, buf_len);
                        break;
                case OUTPUT_C_ARRAY:
                        write_c_array_file(output_filename, buf, buf_len);
                        break;
        }

        irmo_interface_unref(iface);
}

int main(int argc, char *argv[])
{
        parse_cmd_line(argc, argv);
        do_compile();

        return 0;
}

