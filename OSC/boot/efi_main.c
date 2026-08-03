#include "efi.h"

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;

static const CHAR16 boot_message[] = {
    'E','x','i','s','t','a','n','t','O','S',' ','E','F','I',' ','b','o','o','t','\r','\n',0
};

static const CHAR16 prompt[] = {
    'b','a','t','c','h','>',' ',0
};

static const CHAR16 batch_lines[][64] = {
    { 'h','e','l','p',0 },
    { 'e','c','h','o',' ','E','x','i','s','t','e','n','t','O','S',' ','b','a','t','c','h',' ','t','e','r','m','i','n','a','l',0 },
    { 's','t','a','t','u','s',0 },
    { 's','y','s','t','e','m',' ','k','e','r','n','e','l',' ','r','e','a','d','y',0 }
};

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

static void terminal_execute_command(const CHAR16 *command)
{
    const CHAR16 *cursor = command;
    const CHAR16 *command_name = command;

    while (*cursor && *cursor != ' ') {
        cursor++;
    }

    if (*cursor == ' ') {
        /* Treat anything after the command as a payload string. */
        command_name = command;
    }

    if (command_name[0] == 'h' && command_name[1] == 'e' && command_name[2] == 'l' && command_name[3] == 'p' && command_name[4] == 0) {
        terminal_write_line((const CHAR16[]){ 'a','v','a','i','l','a','b','l','e',' ','b','a','t','c','h',' ','c','o','m','m','a','n','d','s',':',' ','h','e','l','p',',',' ','e','c','h','o',',',' ','s','t','a','t','u','s',',',' ','s','y','s','t','e','m',0 });
        return;
    }

    if (command_name[0] == 'e' && command_name[1] == 'c' && command_name[2] == 'h' && command_name[3] == 'o' && command_name[4] == ' ')
    {
        terminal_write_line(command);
        return;
    }

    if (command_name[0] == 's' && command_name[1] == 't' && command_name[2] == 'a' && command_name[3] == 't' && command_name[4] == 'u' && command_name[5] == 's' && command_name[6] == 0) {
        terminal_write_line((const CHAR16[]){ 'o','k',0 });
        return;
    }

    if (command_name[0] == 's' && command_name[1] == 'y' && command_name[2] == 's' && command_name[3] == 't' && command_name[4] == 'e' && command_name[5] == 'm' && command_name[6] == ' ')
    {
        terminal_write_line(command);
        return;
    }

    terminal_write_line((const CHAR16[]){ 'u','n','k','n','o','w','n',' ','c','o','m','m','a','n','d',0 });
}

static void terminal_run_batch(void)
{
    size_t i;

    terminal_write_line((const CHAR16[]){ 'E','x','i','s','t','e','n','t','O','S',' ','b','a','t','c','h',' ','t','e','r','m','i','n','a','l',' ','r','e','a','d','y',0 });

    for (i = 0; i < sizeof(batch_lines) / sizeof(batch_lines[0]); ++i) {
        terminal_write(prompt);
        terminal_execute_command(batch_lines[i]);
    }
}

extern void kernel_main(void);

void kernel_main(void)
{
    /* Temporary placeholder until the kernel entry is fully implemented. */
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    gImageHandle = ImageHandle;
    gST = SystemTable;
    gBS = SystemTable ? SystemTable->BootServices : NULL;

    terminal_write(boot_message);
    terminal_run_batch();

    kernel_main();
    return EFI_SUCCESS;
}
