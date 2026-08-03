#include "efi.h"

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;

static const CHAR16 boot_message[] = {
    'E','x','i','s','t','a','n','t','O','S',' ','E','F','I',' ','b','o','o','t','\r','\n',0
};

typedef enum {
    TERMINAL_MODE_BATCH = 0,
    TERMINAL_MODE_BASH = 1
} TerminalMode;

static const CHAR16 batch_prompt[] = {
    'b','a','t','c','h','>',' ',0
};

static const CHAR16 bash_prompt[] = {
    'b','a','s','h','>',' ',0
};

#define TERMINAL_MAX_ARGS 8
#define TERMINAL_MAX_LINE 128

static TerminalMode terminal_mode = TERMINAL_MODE_BATCH;
static int terminal_echo_enabled = 1;

static const CHAR16 autoexec_batch[] = {
    '@','e','c','h','o',' ','o','f','f','\r','\n',
    'h','e','l','p','\r','\n',
    's','t','a','t','u','s','\r','\n',
    'v','e','r','\r','\n',
    'l','s','\r','\n',
    'm','e','m','\r','\n',
    'c','l','e','a','r','\r','\n',
    0
};

static int terminal_match_command(const CHAR16 *command, const CHAR16 *name)
{
    while (*command && *name && *command == *name) {
        command++;
        name++;
    }

    return (*name == 0) && (*command == 0 || *command == ' ');
}

static const CHAR16 *terminal_command_args(const CHAR16 *command)
{
    while (*command && *command != ' ') {
        command++;
    }

    while (*command == ' ') {
        command++;
    }

    return command;
}

static void terminal_write(const CHAR16 *text)
{
    if (gST && gST->ConOut && gST->ConOut->OutputString) {
        gST->ConOut->OutputString(gST->ConOut, text);
    }
}

static void terminal_write_line(const CHAR16 *text)
{
    terminal_write(text);
    terminal_write((const CHAR16[]){ '\r', '\n', 0 });
}

static const CHAR16 *terminal_current_prompt(void)
{
    return (terminal_mode == TERMINAL_MODE_BASH) ? bash_prompt : batch_prompt;
}

static void terminal_set_mode(TerminalMode mode)
{
    terminal_mode = mode;
    terminal_echo_enabled = 1;
}

static void terminal_write_arg_list(CHAR16 *argv[], int argc, int start_index)
{
    int i;
    for (i = start_index; i < argc; ++i) {
        if (i > start_index) {
            terminal_write((const CHAR16[]){ ' ', 0 });
        }
        terminal_write(argv[i]);
    }
}

static int terminal_parse_command(const CHAR16 *command, CHAR16 *argv[], int max_args)
{
    CHAR16 line[TERMINAL_MAX_LINE];
    size_t i = 0;
    int argc = 0;
    CHAR16 *cursor;

    while (*command && i < (TERMINAL_MAX_LINE - 1)) {
        line[i++] = *command++;
    }
    line[i] = 0;

    cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    while (*cursor && argc < max_args) {
        argv[argc++] = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t') {
            cursor++;
        }

        if (*cursor == 0) {
            break;
        }

        *cursor++ = 0;
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
    }

    return argc;
}

static int terminal_execute_command(const CHAR16 *command)
{
    CHAR16 *argv[TERMINAL_MAX_ARGS];
    int argc = terminal_parse_command(command, argv, TERMINAL_MAX_ARGS);
    const CHAR16 *args = terminal_command_args(command);

    if (argc == 0) {
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ '@','e','c','h','o',0 })) {
        if (argc > 1 && terminal_match_command(argv[1], (const CHAR16[]){ 'o','f','f',0 })) {
            terminal_echo_enabled = 0;
        } else if (argc > 1 && terminal_match_command(argv[1], (const CHAR16[]){ 'o','n',0 })) {
            terminal_echo_enabled = 1;
        }
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'r','e','m',0 })) {
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 's','w','i','t','c','h','l','a','n','g',0 })) {
        if (argc > 1 && terminal_match_command(argv[1], (const CHAR16[]){ 'b','a','s','h',0 })) {
            terminal_set_mode(TERMINAL_MODE_BASH);
            terminal_write_line((const CHAR16[]){ 's','w','i','t','c','h','e','d',' ','t','o',' ','b','a','s','h',0 });
            return 0;
        }
        if (argc > 1 && terminal_match_command(argv[1], (const CHAR16[]){ 'b','a','t','c','h',0 })) {
            terminal_set_mode(TERMINAL_MODE_BATCH);
            terminal_write_line((const CHAR16[]){ 's','w','i','t','c','h','e','d',' ','t','o',' ','b','a','t','c','h',0 });
            return 0;
        }
        terminal_write_line((const CHAR16[]){ 'u','s','a','g','e',':',' ','s','w','i','t','c','h','l','a','n','g',' ','b','a','s','h',' ','|',' ','b','a','t','c','h',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'h','e','l','p',0 })) {
        terminal_write_line((const CHAR16[]){
            'a','v','a','i','l','a','b','l','e',' ','c','o','m','m','a','n','d','s',':',
            ' ','h','e','l','p',',',' ','e','c','h','o',',',' ','s','t','a','t','u','s',',',
            ' ','s','y','s','t','e','m',',',' ','v','e','r',',',' ','d','i','r',',',' ','l','s',',',
            ' ','m','e','m',',',' ','c','l','e','a','r',',',' ','c','l','s',',',' ','p','a','u','s','e',',',
            ' ','e','x','i','t',',',' ','s','h','u','t','d','o','w','n',',',' ','r','e','b','o','o','t',',',
            ' ','t','y','p','e',',',' ','s','e','t',',',' ','t','i','m','e',',',' ','d','a','t','e',',',
            ' ','c','d',',',' ','c','o','p','y',',',' ','d','e','l',',',' ','r','e','n',',',' ','m','d',',',' ','r','d',',',
            ' ','p','a','t','h',',',' ','w','h','o','a','m','i',',',' ','p','w','d',',',' ','s','w','i','t','c','h','l','a','n','g',0
        });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'e','c','h','o',0 })) {
        terminal_write_arg_list(argv, argc, 1);
        terminal_write_line((const CHAR16[]){ 0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 's','t','a','t','u','s',0 })) {
        terminal_write_line((const CHAR16[]){ 'o','k',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 's','y','s','t','e','m',0 })) {
        terminal_write_line(args);
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'v','e','r',0 })) {
        terminal_write_line((const CHAR16[]){ 'E','x','i','s','t','e','n','t','O','S',' ','b','a','t','c','h',' ','t','e','r','m','i','n','a','l',' ','0','.','1',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'd','i','r',0 }) || terminal_match_command(argv[0], (const CHAR16[]){ 'l','s',0 })) {
        terminal_write_line((const CHAR16[]){ 'd','r','i','v','e','r','s',':',' ','a','c','p','i',',',' ','b','a','t','t','e','r','y',',',' ','m','o','u','s','e',',',' ','p','c','i',',',' ','s','t','o','r','a','g','e',',',' ','u','s','b',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'm','e','m',0 })) {
        terminal_write_line((const CHAR16[]){ 'm','e','m','o','r','y',' ','m','a','n','a','g','e','r',' ','r','e','a','d','y',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'c','l','e','a','r',0 }) || terminal_match_command(argv[0], (const CHAR16[]){ 'c','l','s',0 })) {
        terminal_write_line((const CHAR16[]){ ' ',0 });
        terminal_write_line((const CHAR16[]){ ' ',0 });
        terminal_write_line((const CHAR16[]){ ' ',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'p','a','u','s','e',0 })) {
        terminal_write_line((const CHAR16[]){ 'p','r','e','s','s',' ','a','n','y',' ','k','e','y',' ','t','o',' ','c','o','n','t','i','n','u','e',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'e','x','i','t',0 }) || terminal_match_command(argv[0], (const CHAR16[]){ 's','h','u','t','d','o','w','n',0 })) {
        terminal_write_line((const CHAR16[]){ 's','h','u','t','d','o','w','n',' ','c','o','m','m','a','n','d',' ','r','e','c','e','i','v','e','d',0 });
        return 1;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'r','e','b','o','o','t',0 })) {
        terminal_write_line((const CHAR16[]){ 'r','e','b','o','o','t',' ','c','o','m','m','a','n','d',' ','r','e','c','e','i','v','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 't','y','p','e',0 })) {
        terminal_write_line((const CHAR16[]){ 'b','a','t',' ','s','c','r','i','p','t',' ','e','x','e','c','u','t','i','o','n',' ','s','u','p','p','o','r','t',' ','a','c','t','i','v','e',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 's','e','t',0 })) {
        terminal_write_line((const CHAR16[]){ 'e','n','v','i','r','o','n','m','e','n','t',' ','v','a','r','i','a','b','l','e','s',' ','s','e','t',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 't','i','m','e',0 })) {
        terminal_write_line((const CHAR16[]){ '0','0',':','0','0',':','0','0',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'd','a','t','e',0 })) {
        terminal_write_line((const CHAR16[]){ '0','6','/','0','8','/','2','0','2','6',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'c','d',0 })) {
        terminal_write_line((const CHAR16[]){ 'p','r','o','c','e','s','s',' ','w','o','r','k','i','n','g',' ','d','i','r','e','c','t','o','r','y',' ','c','h','a','n','g','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'c','o','p','y',0 })) {
        terminal_write_line((const CHAR16[]){ 'c','o','p','y',' ','o','p','e','r','a','t','i','o','n',' ','s','u','p','p','o','r','t','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'd','e','l',0 })) {
        terminal_write_line((const CHAR16[]){ 'd','e','l','e','t','e',' ','o','p','e','r','a','t','i','o','n',' ','s','u','p','p','o','r','t','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'r','e','n',0 })) {
        terminal_write_line((const CHAR16[]){ 'r','e','n','a','m','e',' ','o','p','e','r','a','t','i','o','n',' ','s','u','p','p','o','r','t','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'm','d',0 })) {
        terminal_write_line((const CHAR16[]){ 'd','i','r','e','c','t','o','r','y',' ','c','r','e','a','t','i','o','n',' ','s','u','p','p','o','r','t','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'r','d',0 })) {
        terminal_write_line((const CHAR16[]){ 'd','i','r','e','c','t','o','r','y',' ','r','e','m','o','v','a','l',' ','s','u','p','p','o','r','t','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'p','a','t','h',0 })) {
        terminal_write_line((const CHAR16[]){ 'p','a','t','h',' ','s','e','t',' ','t','o',' ','E','x','i','s','t','e','n','t','O','S',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'w','h','o','a','m','i',0 })) {
        terminal_write_line((const CHAR16[]){ 'E','x','i','s','t','e','n','t','O','S',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'p','w','d',0 })) {
        terminal_write_line((const CHAR16[]){ '/',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'b','a','s','h',0 })) {
        terminal_set_mode(TERMINAL_MODE_BASH);
        terminal_write_line((const CHAR16[]){ 'b','a','s','h',' ','m','o','d','e',' ','e','n','a','b','l','e','d',0 });
        return 0;
    }

    if (terminal_match_command(argv[0], (const CHAR16[]){ 'b','a','t','c','h',0 })) {
        terminal_set_mode(TERMINAL_MODE_BATCH);
        terminal_write_line((const CHAR16[]){ 'b','a','t','c','h',' ','m','o','d','e',' ','e','n','a','b','l','e','d',0 });
        return 0;
    }

    terminal_write_line((const CHAR16[]){ 'u','n','k','n','o','w','n',' ','c','o','m','m','a','n','d',0 });
    return 0;
}

static void terminal_run_batch_file(const CHAR16 *batch_data)
{
    const CHAR16 *line_start = batch_data;

    while (*line_start) {
        const CHAR16 *line_end = line_start;
        CHAR16 line[128];
        size_t offset = 0;

        while (*line_end && *line_end != '\r' && *line_end != '\n' && offset < (sizeof(line) - 1)) {
            line[offset++] = *line_end++;
        }

        line[offset] = 0;

        if (offset > 0) {
            if (terminal_echo_enabled) {
                terminal_write(terminal_current_prompt());
            }
            terminal_execute_command(line);
        }

        while (*line_end == '\r' || *line_end == '\n') {
            line_end++;
        }

        line_start = line_end;
    }
}

static int terminal_read_line(CHAR16 *line, size_t max_length)
{
    size_t length = 0;
    EFI_INPUT_KEY key;

    while (gST && gST->ConIn && gST->ConIn->ReadKeyStroke) {
        gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

        if (key.UnicodeChar == 0) {
            continue;
        }

        if (key.UnicodeChar == '\r') {
            terminal_write((const CHAR16[]){ '\r', '\n', 0 });
            line[length] = 0;
            return (int)length;
        }

        if (key.UnicodeChar == '\b') {
            if (length > 0) {
                length--;
                line[length] = 0;
                terminal_write((const CHAR16[]){ '\b', ' ', '\b', 0 });
            }
            continue;
        }

        if (length + 1 >= max_length) {
            continue;
        }

        if (terminal_echo_enabled) {
            terminal_write((const CHAR16[]){ key.UnicodeChar, 0 });
        }

        line[length++] = key.UnicodeChar;
    }

    return 0;
}

static void terminal_run_shell(void)
{
    CHAR16 line[TERMINAL_MAX_LINE];

    terminal_write_line((const CHAR16[]){ 'E','x','i','s','t','e','n','t','O','S',' ','s','h','e','l','l',' ','r','e','a','d','y',0 });

    for (;;) {
        size_t length;

        terminal_write(terminal_current_prompt());
        length = (size_t)terminal_read_line(line, sizeof(line) / sizeof(line[0]));

        if (length == 0) {
            continue;
        }

        if (terminal_execute_command(line)) {
            break;
        }
    }
}

static void terminal_run_batch(void)
{
    terminal_write_line((const CHAR16[]){ 'E','x','i','s','t','e','n','t','O','S',' ','b','a','t','c','h',' ','t','e','r','m','i','n','a','l',' ','r','e','a','d','y',0 });
    terminal_run_batch_file(autoexec_batch);
}

extern void kernel_startup(void);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    gImageHandle = ImageHandle;
    gST = SystemTable;
    gBS = SystemTable ? SystemTable->BootServices : NULL;

    terminal_write(boot_message);
    terminal_run_batch();
    terminal_run_shell();

    kernel_startup();
    return EFI_SUCCESS;
}
