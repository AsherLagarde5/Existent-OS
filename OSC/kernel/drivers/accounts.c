#include "accounts.h"

#define ACCOUNT_TEXT_MAX 31

typedef struct { CHAR16 name[ACCOUNT_TEXT_MAX + 1]; CHAR16 password[ACCOUNT_TEXT_MAX + 1]; int admin; } account_t;
typedef struct { uint32_t magic; uint32_t count; account_t entries[FISIX_MAX_USERS]; } account_store_t;
static account_t accounts[FISIX_MAX_USERS];
static int account_count;
static int current_account = -1;
static const CHAR16 nobody[] = {'n','o','b','o','d','y',0};
#define ACCOUNT_STORE_MAGIC 0x46555352u

static int accounts_open(EFI_FILE_PROTOCOL **file, UINT64 mode)
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = 0;
    EFI_FILE_PROTOCOL *root = 0;
    EFI_STATUS status;
    if (!gBS || !gBS->LocateProtocol) return 0;
    status = gBS->LocateProtocol(&EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, 0, (void **)&fs);
    if (EFI_ERROR(status) || !fs || EFI_ERROR(fs->OpenVolume(fs, &root))) return 0;
    status = root->Open(root, file, (const CHAR16[]){'\\','f','i','s','i','x','_','u','s','e','r','s','.','d','b',0}, mode, 0);
    root->Close(root);
    return !EFI_ERROR(status) && *file;
}

static int accounts_save(void)
{
    EFI_FILE_PROTOCOL *file = 0;
    account_store_t store;
    UINTN bytes = sizeof(store);
    if (!accounts_open(&file, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE)) return 0;
    store.magic = ACCOUNT_STORE_MAGIC;
    store.count = (uint32_t)account_count;
    for (int i = 0; i < FISIX_MAX_USERS; ++i) store.entries[i] = accounts[i];
    if (EFI_ERROR(file->SetPosition(file, 0)) || EFI_ERROR(file->Write(file, &bytes, &store)) || bytes != sizeof(store)) { file->Close(file); return 0; }
    if (file->Flush) file->Flush(file);
    file->Close(file);
    return 1;
}

static void accounts_load(void)
{
    EFI_FILE_PROTOCOL *file = 0;
    account_store_t store;
    UINTN bytes = sizeof(store);
    if (!accounts_open(&file, EFI_FILE_MODE_READ)) return;
    if (!EFI_ERROR(file->Read(file, &bytes, &store)) && bytes == sizeof(store) && store.magic == ACCOUNT_STORE_MAGIC && store.count > 0 && store.count <= FISIX_MAX_USERS) {
        account_count = (int)store.count;
        for (int i = 0; i < FISIX_MAX_USERS; ++i) accounts[i] = store.entries[i];
    }
    file->Close(file);
}

static int equal(const CHAR16 *a, const CHAR16 *b) { while (*a && *b && *a == *b) { ++a; ++b; } return *a == 0 && *b == 0; }
static int valid(const CHAR16 *s) { int n = 0; while (*s) { if (*s == ' ' || ++n > ACCOUNT_TEXT_MAX) return 0; ++s; } return n != 0; }
static void copy(CHAR16 *to, const CHAR16 *from) { int i = 0; while (from[i] && i < ACCOUNT_TEXT_MAX) { to[i] = from[i]; ++i; } to[i] = 0; }

void accounts_init(void)
{
    account_count = 1;
    current_account = -1;
    copy(accounts[0].name, (const CHAR16[]){'a','d','m','i','n',0});
    copy(accounts[0].password, (const CHAR16[]){'f','i','s','i','x',0});
    accounts[0].admin = 1;
    accounts_load();
}

int accounts_login(const CHAR16 *name, const CHAR16 *password)
{
    for (int i = 0; i < account_count; ++i) if (equal(accounts[i].name, name) && equal(accounts[i].password, password)) { current_account = i; return 1; }
    return 0;
}

int accounts_add(const CHAR16 *name, const CHAR16 *password)
{
    if (!accounts_is_admin() || !valid(name) || !valid(password) || account_count == FISIX_MAX_USERS) return 0;
    for (int i = 0; i < account_count; ++i) if (equal(accounts[i].name, name)) return 0;
    copy(accounts[account_count].name, name); copy(accounts[account_count].password, password); accounts[account_count++].admin = 0;
    return accounts_save();
}

int accounts_change_password(const CHAR16 *old_password, const CHAR16 *new_password)
{
    if (current_account < 0 || !equal(accounts[current_account].password, old_password) || !valid(new_password)) return 0;
    copy(accounts[current_account].password, new_password); return accounts_save();
}

int accounts_is_admin(void) { return current_account >= 0 && accounts[current_account].admin; }
const CHAR16 *accounts_current_user(void) { return current_account >= 0 ? accounts[current_account].name : nobody; }
int accounts_count(void) { return account_count; }
const CHAR16 *accounts_user_at(int index) { return index >= 0 && index < account_count ? accounts[index].name : 0; }
