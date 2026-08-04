#include "accounts.h"

#define ACCOUNT_TEXT_MAX 31

typedef struct { CHAR16 name[ACCOUNT_TEXT_MAX + 1]; CHAR16 password[ACCOUNT_TEXT_MAX + 1]; int admin; } account_t;
static account_t accounts[FISIX_MAX_USERS];
static int account_count;
static int current_account = -1;
static const CHAR16 nobody[] = {'n','o','b','o','d','y',0};

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
    copy(accounts[account_count].name, name); copy(accounts[account_count].password, password); accounts[account_count++].admin = 0; return 1;
}

int accounts_change_password(const CHAR16 *old_password, const CHAR16 *new_password)
{
    if (current_account < 0 || !equal(accounts[current_account].password, old_password) || !valid(new_password)) return 0;
    copy(accounts[current_account].password, new_password); return 1;
}

int accounts_is_admin(void) { return current_account >= 0 && accounts[current_account].admin; }
const CHAR16 *accounts_current_user(void) { return current_account >= 0 ? accounts[current_account].name : nobody; }
int accounts_count(void) { return account_count; }
const CHAR16 *accounts_user_at(int index) { return index >= 0 && index < account_count ? accounts[index].name : 0; }
