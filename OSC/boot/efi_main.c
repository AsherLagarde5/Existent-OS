#include "efi.h"

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;

static const CHAR16 boot_message[] = {
    'E','x','i','s','t','a','n','t','O','S',' ','E','F','I',' ','b','o','o','t','\r','\n',0
};

extern void kernel_main(void);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    gImageHandle = ImageHandle;
    gST = SystemTable;
    gBS = SystemTable ? SystemTable->BootServices : NULL;

    if (SystemTable && SystemTable->ConOut && SystemTable->ConOut->OutputString) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, boot_message);
    }

    kernel_main();
    return EFI_SUCCESS;
}
