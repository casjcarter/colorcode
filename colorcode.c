/* vim: set ts=4 sw=4 tw=0 noet ft=c :
 *
 * color-code - An ANSI color escape sequence generator
 * Written by Cassandra J Carter
 *
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESC "\033["
#define ESC_CPY "\\033["

#define RESET     "0"
#define BOLD      "1"
#define ITALIC    "3"
#define UNDERLINE "4"
#define REVERSE   "7"

struct CliArgs {
    int  foreground;
    int  background;
    bool reset;
    bool bold;
    bool italic;
    bool underline;
    bool reverse;
};

enum ClipboardTool {
    xsel,
    xclip,
    wl_clipboard,
    no_clipboard,
};

struct CliArgs *
new_cli_args_struct()
{
    struct CliArgs *struct_ptr = (struct CliArgs *)malloc(sizeof(struct CliArgs));
    struct_ptr -> foreground = -1;
    struct_ptr -> background = -1;
    struct_ptr -> reset      = false;
    struct_ptr -> bold       = false;
    struct_ptr -> italic     = false;
    struct_ptr -> underline  = false;
    struct_ptr -> reverse    = false;
    return struct_ptr;
}

void
error(char *message)
{
    printf("\033[1;91m%s\n\033[0m", message);
    exit(1);
}

/*
 * Checks a string array for a certain string.
 * Returns the index if the string is found
 * Returns -1 if not
 */
int
str_arr_contains(char *str, char **arr, int arr_size)
{
    int i;
    for (i = 0; i < arr_size; i++)
        if (strcmp(str, arr[i]) == 0)
            return i;
    return -1;
}

bool
is_integer(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

bool
is_external_program_running(char *cmd_name)
{
    bool is_running = true;

    char *full_cmd = (char *)malloc(64);
    sprintf(full_cmd, "pgrep -x %s > /dev/null", cmd_name);

    if (system(full_cmd) != 0)
        is_running = false;

    return is_running;
}

/* Add more options if necessary */
void
print_help()
{
    printf("color-code - An ANSI color escape code generator\n");
    printf("\n");
    printf("Usage: colorcode [OPTIONS]\n");
    printf("Options:\n");
    printf("  -h, --help\t\t\tPrint this help message and exit.\n");
    printf("  -r --reset\t\t\tReturns the 'reset/normal' escape sequence\n");
    printf("  -f --foreground [0,15]\tSets the foreground color of the escape sequence to the following terminal color.\n");
    printf("  -b --background [0,15]\tSets the background color of the escape sequence to the following terminal color.\n");
    printf("  -B --bold\t\t\tSets the escape sequence to contain the 'bold'attribute.\n");
    printf("  -I --italic\t\t\tSets the escape sequence to contain the 'italic' attribute.\n");
    printf("  -U --underline\t\tSets the escape sequence to contain the 'underline' attribute.\n");
    printf("  -R --reverse\t\t\tSets the escape sequence to contain the 'reverse' attribute.\n");

}

enum ClipboardTool
get_clipboard_tool()
{
    if (is_external_program_running("xclip"))
        return xclip;
    if (is_external_program_running("xsel"))
        return xsel;
    if (is_external_program_running("wl-clipboard"))
        return wl_clipboard;
    return no_clipboard;
}

void
copy_to_clipboard(enum ClipboardTool clip, char *str)
{

    char *copy_cmd = (char *)malloc(64);
    if (clip == xclip) {
        strcat(copy_cmd, "xclip -i -selection clipboard");
    } else if (clip == xsel) {
        strcat(copy_cmd, "xsel -i -b");
    } else if (clip == wl_clipboard) {
        strcat(copy_cmd, "wl-copy");
    } else {
        return;
    }

    FILE *fp;

    fp = popen(copy_cmd, "w");
    fprintf(fp, "%s", str);
    fclose(fp);
}

char *
generate_escape_sequence(struct CliArgs *args, bool for_copy)
{
    char *seq_str = (char *)malloc(32);

    if (for_copy) {
        strcat(seq_str, ESC_CPY);
    } else {
        strcat(seq_str, ESC);
    }
    if (args -> reset) {
        strcat(seq_str, RESET);
        strcat(seq_str, "m");
        return seq_str;
    }
    if (args -> bold) {
        strcat(seq_str, BOLD);
        strcat(seq_str, ";");
    }
    if (args -> italic) {
        strcat(seq_str, ITALIC);
        strcat(seq_str, ";");
    }
    if (args -> underline) {
        strcat(seq_str, UNDERLINE);
        strcat(seq_str, ";");
    }
    if (args -> reverse) {
        strcat(seq_str, REVERSE);
        strcat(seq_str, ";");
    }
    if (args -> foreground != -1) {
        int fg_val;
        if (args -> foreground < 8) {
            fg_val = args -> foreground + 30;
        } else {
            fg_val = args -> foreground + 82;
        }
        char fg_val_str[(fg_val / 10) + 1];
        sprintf(fg_val_str, "%d", fg_val);
        strcat(seq_str, fg_val_str);
        strcat(seq_str, ";");
    }
    if (args -> background != -1) {
        int bg_val;
        if (args -> background < 8) {
            bg_val = args -> background + 40;
        } else {
            bg_val = args -> background + 92;
        }
        char bg_val_str[(bg_val / 10) + 1];
        sprintf(bg_val_str, "%d", bg_val);
        strcat(seq_str, bg_val_str);
        strcat(seq_str, ";");
    }

    seq_str[strlen(seq_str) - 1] = 'm';
    return seq_str;
}

int
main(int argc, char **argv)
{
    /* FIXME: Throw error for invalid arguments */
    struct CliArgs *args_struct = new_cli_args_struct();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0) {
            print_help();
        } else if (strcmp(argv[i], "-r") == 0
            || strcmp(argv[i], "--reset") == 0) {
            args_struct -> reset = true;
        } else if (strcmp(argv[i], "-B") == 0
            || strcmp(argv[i], "--bold") == 0) {
            args_struct -> bold = true;
        } else if (strcmp(argv[i], "-I") == 0
            || strcmp(argv[i], "--italic") == 0) {
            args_struct -> italic = true;
        } else if (strcmp(argv[i], "-U") == 0
            || strcmp(argv[i], "--underline") == 0) {
            args_struct -> underline = true;
        } else if (strcmp(argv[i], "-R") == 0
            || strcmp(argv[i], "--reverse") == 0) {
            args_struct -> reverse = true;
        } else if (strcmp(argv[i], "-f") == 0
            || strcmp(argv[i], "--foreground") == 0) {
            if (is_integer(argv[i + 1])) {
                int fg_col = atoi(argv[i + 1]);
                if (fg_col >= 0 && fg_col <= 15) {
                    args_struct -> foreground = fg_col;
                    i++;
                } else {
                    error("Invalid argument. Colors must be a number between 0 and 15");
                }
            } else {
                error("Invalid argument. Colors must be a number between 0 and 15");
            }
        } else if (strcmp(argv[i], "-b") == 0
            || strcmp(argv[i], "--background") == 0) {
            if (is_integer(argv[i + 1])) {
                int bg_col = atoi(argv[i + 1]);
                if (bg_col >= 0 && bg_col <= 15) {
                    args_struct -> background = bg_col;
                    i++;
                } else {
                    error("Invalid argument. Colors must be a number between 0 and 15");
                }
            } else {
                error("Invalid argument. Colors must be a number between 0 and 15");
            }
        } else {
            printf("Invalid argument. %s\n", argv[i]);
        }
    }

    char * copy_esc_seq = generate_escape_sequence(args_struct, true);
    char * message_esc_seq = generate_escape_sequence(args_struct, false);

    copy_to_clipboard(get_clipboard_tool(), copy_esc_seq);
    printf("%sCopied to clipboard!\033[0m\n", message_esc_seq);
    return 0;
}
