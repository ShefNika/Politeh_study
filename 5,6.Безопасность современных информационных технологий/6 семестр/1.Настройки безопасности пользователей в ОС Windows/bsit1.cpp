#include <windows.h>
#include <lm.h> // NetAPI
#include <ntsecapi.h> // lsa
#include <sddl.h> // SID to str
#include <iostream>
#include <iomanip>
#include <ntstatus.h>

typedef BOOL(WINAPI* pConvertSidToStringSid)(PSID, LPWSTR*);
typedef BOOL(WINAPI* pConvertStringSidToSid)(LPCWSTR, PSID*);
typedef BOOL(WINAPI* pLookupAccountName)(LPCWSTR, LPCWSTR, PSID, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetLocalGroupEnum)(LPCWSTR, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, PDWORD_PTR);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetUserEnum)(LPCWSTR, DWORD, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, PDWORD);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetApiBufferFree)(_Frees_ptr_opt_ LPVOID);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetUserAdd)(LPCWSTR, DWORD, LPBYTE, LPDWORD);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetUserDel)(LPCWSTR, LPCWSTR);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetUserChangePassword)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetLocalGroupAdd)(LPCWSTR, DWORD, LPBYTE, LPDWORD);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetLocalGroupDel)(LPCWSTR, LPCWSTR);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetLocalGroupAddMembers)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, DWORD);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetLocalGroupDelMembers)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, DWORD);
typedef NET_API_STATUS(NET_API_FUNCTION* pNetUserGetLocalGroups)(LPCWSTR, LPCWSTR, DWORD, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD);
typedef NTSTATUS(WINAPI* pLsaAddAccountRights)(LSA_HANDLE, PSID, PLSA_UNICODE_STRING, ULONG);
typedef NTSTATUS(WINAPI* pLsaRemoveAccountRights)(LSA_HANDLE, PSID, BOOLEAN, PLSA_UNICODE_STRING, ULONG);
typedef NTSTATUS(WINAPI* pLsaOpenPolicy)(PLSA_UNICODE_STRING, PLSA_OBJECT_ATTRIBUTES, ACCESS_MASK, PLSA_HANDLE);
typedef NTSTATUS(WINAPI* pLsaEnumerateAccountRights)(LSA_HANDLE, PSID, PLSA_UNICODE_STRING*, PULONG);
typedef ULONG(WINAPI* pLsaNtStatusToWinError)(NTSTATUS);


struct API_FUNCTIONS {
    HMODULE hNetApi32;
    HMODULE hAdvApi32;

    pConvertSidToStringSid ConvertSidToStringSid;
    pConvertStringSidToSid ConvertStringSidToSid;
    pLookupAccountName LookupAccountName;
    pNetLocalGroupEnum NetLocalGroupEnum;
    pNetUserEnum NetUserEnum;
    pNetApiBufferFree NetApiBufferFree;
    pNetUserAdd NetUserAdd;
    pNetUserDel NetUserDel;
    pNetUserChangePassword NetUserChangePassword;
    pNetLocalGroupAdd NetLocalGroupAdd;
    pNetLocalGroupDel NetLocalGroupDel;
    pNetLocalGroupAddMembers NetLocalGroupAddMembers;
    pNetLocalGroupDelMembers NetLocalGroupDelMembers;
    pNetUserGetLocalGroups NetUserGetLocalGroups;
    pLsaAddAccountRights LsaAddAccountRights;
    pLsaRemoveAccountRights LsaRemoveAccountRights;
    pLsaOpenPolicy LsaOpenPolicy;
    pLsaEnumerateAccountRights LsaEnumerateAccountRights;
    pLsaNtStatusToWinError LsaNtStatusToWinError;
};


API_FUNCTIONS g_api;


BOOL InitApi() {
    g_api.hNetApi32 = NULL;
    g_api.hAdvApi32 = NULL;

    g_api.hNetApi32 = LoadLibrary(L"netapi32.dll");
    g_api.hAdvApi32 = LoadLibrary(L"advapi32.dll");

    if (!g_api.hNetApi32 || !g_api.hAdvApi32) {
        std::cerr << "Error! Failed to load required libraries!" << std::endl;
        return FALSE;
    }

    g_api.ConvertSidToStringSid = (pConvertSidToStringSid)GetProcAddress(g_api.hAdvApi32, "ConvertSidToStringSidW");
    g_api.ConvertStringSidToSid = (pConvertStringSidToSid)GetProcAddress(g_api.hAdvApi32, "ConvertStringSidToSidW");
    g_api.LookupAccountName = (pLookupAccountName)GetProcAddress(g_api.hAdvApi32, "LookupAccountNameW");
    g_api.LsaOpenPolicy = (pLsaOpenPolicy)GetProcAddress(g_api.hAdvApi32, "LsaOpenPolicy");
    g_api.LsaEnumerateAccountRights = (pLsaEnumerateAccountRights)GetProcAddress(g_api.hAdvApi32, "LsaEnumerateAccountRights");
    g_api.LsaNtStatusToWinError = (pLsaNtStatusToWinError)GetProcAddress(g_api.hAdvApi32, "LsaNtStatusToWinError");
    g_api.LsaRemoveAccountRights = (pLsaRemoveAccountRights)GetProcAddress(g_api.hAdvApi32, "LsaRemoveAccountRights");
    g_api.LsaAddAccountRights = (pLsaAddAccountRights)GetProcAddress(g_api.hAdvApi32, "LsaAddAccountRights");

   
    g_api.NetApiBufferFree = (pNetApiBufferFree)GetProcAddress(g_api.hNetApi32, "NetApiBufferFree");
    g_api.NetUserAdd = (pNetUserAdd)GetProcAddress(g_api.hNetApi32, "NetUserAdd");
    g_api.NetUserDel = (pNetUserDel)GetProcAddress(g_api.hNetApi32, "NetUserDel");
    g_api.NetUserGetLocalGroups = (pNetUserGetLocalGroups)GetProcAddress(g_api.hNetApi32, "NetUserGetLocalGroups");
    g_api.NetLocalGroupDel = (pNetLocalGroupDel)GetProcAddress(g_api.hNetApi32, "NetLocalGroupDel");
    g_api.NetLocalGroupAdd = (pNetLocalGroupAdd)GetProcAddress(g_api.hNetApi32, "NetLocalGroupAdd");
    g_api.NetLocalGroupAddMembers = (pNetLocalGroupAddMembers)GetProcAddress(g_api.hNetApi32, "NetLocalGroupAddMembers");
    g_api.NetLocalGroupEnum = (pNetLocalGroupEnum)GetProcAddress(g_api.hNetApi32, "NetLocalGroupEnum");
    g_api.NetUserEnum = (pNetUserEnum)GetProcAddress(g_api.hNetApi32, "NetUserEnum");
    g_api.NetLocalGroupDelMembers = (pNetLocalGroupDelMembers)GetProcAddress(g_api.hNetApi32, "NetLocalGroupDelMembers");

    return TRUE;
}

BOOL FreeApi() {
    BOOL result = TRUE;
    if (g_api.hNetApi32) result &= FreeLibrary(g_api.hNetApi32);
    if (g_api.hAdvApi32) result &= FreeLibrary(g_api.hAdvApi32);
    return result;
}


LPWSTR GetStringSID(SID_NAME_USE sid_name_use, LPCWSTR name) {
    PSID sid = NULL;
    DWORD cbSid = 0;
    DWORD name_size = 0;
    LPWSTR domain = NULL;
    LPWSTR strsid = NULL;


    g_api.LookupAccountName(NULL, name, NULL, &cbSid, NULL, &name_size, &sid_name_use);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return NULL;

    sid = (PSID)malloc(cbSid);
    domain = (LPWSTR)malloc(sizeof(WCHAR) * name_size);
    if (!sid || !domain) {
        free(sid);
        free(domain);
        return NULL;
    }

    ZeroMemory(sid, cbSid);
    ZeroMemory(domain, sizeof(WCHAR) * name_size);

    if (g_api.LookupAccountName(NULL, name, sid, &cbSid, domain, &name_size, &sid_name_use))
        g_api.ConvertSidToStringSid(sid, &strsid);

    free(sid);
    free(domain);
    return strsid;
}


void DisplayUserGroups(LPWSTR username) {
    LPLOCALGROUP_USERS_INFO_0 groups_info = NULL;
    DWORD entries_read = 0;
    DWORD total_entries = 0;

    NET_API_STATUS status = g_api.NetUserGetLocalGroups(NULL, username, 0, LG_INCLUDE_INDIRECT,
        (LPBYTE*)&groups_info, MAX_PREFERRED_LENGTH,
        &entries_read, &total_entries);

    if (status == NERR_Success && groups_info != NULL) {
        for (DWORD i = 0; i < entries_read; i++) {
            std::wcout << L"    Group: " << groups_info[i].lgrui0_name << std::endl;

            LPWSTR group_sid_str = GetStringSID(SidTypeGroup, groups_info[i].lgrui0_name);
            if (group_sid_str) {
                PSID group_sid = NULL;
                if (g_api.ConvertStringSidToSid(group_sid_str, &group_sid)) {
                    LSA_OBJECT_ATTRIBUTES obj_attrs;
                    ZeroMemory(&obj_attrs, sizeof(obj_attrs));

                    LSA_HANDLE hLsa = NULL;
                    NTSTATUS lsa_status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
                        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
                        &hLsa);

                    if (lsa_status == STATUS_SUCCESS) {
                        PLSA_UNICODE_STRING rights = NULL;
                        ULONG rights_count = 0;

                        NTSTATUS enum_status = g_api.LsaEnumerateAccountRights(hLsa, group_sid, &rights, &rights_count);
                        if (enum_status == STATUS_SUCCESS) {
                            for (ULONG j = 0; j < rights_count; j++) {
                                std::wcout << L"      - " << rights[j].Buffer << L" (inherited)" << std::endl;
                            }
                        }
                    }
                }
                LocalFree(group_sid);
            }
        }
    }

    if (groups_info != NULL) {
        g_api.NetApiBufferFree(groups_info);
    }
}


void ListAllUsers() {
    LPUSER_INFO_0 users = NULL;
    DWORD entries_read = 0;
    DWORD total_entries = 0;
    DWORD resume_handle = 0;
    NET_API_STATUS status;

    do {
        status = g_api.NetUserEnum(NULL, 0, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&users,
            MAX_PREFERRED_LENGTH, &entries_read, &total_entries, &resume_handle);

        if ((status == NERR_Success || status == ERROR_MORE_DATA) && users != NULL) {
            for (DWORD i = 0; i < entries_read; i++) {
                std::wcout << L"\nUser: " << users[i].usri0_name << std::endl;

                LPWSTR sid_str = GetStringSID(SidTypeUser, users[i].usri0_name);
                if (sid_str) {
                    std::wcout << L"SID: " << sid_str << std::endl;

                    PSID user_sid = NULL;
                    if (g_api.ConvertStringSidToSid(sid_str, &user_sid)) {
                        LSA_OBJECT_ATTRIBUTES obj_attrs;
                        ZeroMemory(&obj_attrs, sizeof(obj_attrs));

                        LSA_HANDLE hLsa = NULL;
                        NTSTATUS lsa_status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
                            POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
                            &hLsa);

                        if (lsa_status == STATUS_SUCCESS) {
                            PLSA_UNICODE_STRING rights = NULL;
                            ULONG rights_count = 0;

                            std::wcout << L"Direct Privileges:" << std::endl;
                            NTSTATUS enum_status = g_api.LsaEnumerateAccountRights(hLsa, user_sid, &rights, &rights_count);
                            if (enum_status == STATUS_SUCCESS) {
                                for (ULONG j = 0; j < rights_count; j++) {
                                    std::wcout << L"  - " << rights[j].Buffer << std::endl;
                                }
                            }
                            else {
                                std::wcout << L"  (none)" << std::endl;
                            }

                            std::wcout << L"Group Memberships (with inherited privileges):" << std::endl;
                            DisplayUserGroups(users[i].usri0_name);
                        }
                    }
                }
                LocalFree(sid_str);
            }
        }

        if (users != NULL) {
            g_api.NetApiBufferFree(users);
            users = NULL;
        }
    } while (status == ERROR_MORE_DATA);
}

void ListAllGroups() {

    LPGROUP_INFO_1 groups = NULL;
    DWORD entries_read = 0;
    DWORD total_entries = 0;
    DWORD_PTR resume_handle = 0;
    NET_API_STATUS status;

    do {
        status = g_api.NetLocalGroupEnum(NULL, 1, (LPBYTE*)&groups,
            MAX_PREFERRED_LENGTH, &entries_read, &total_entries, &resume_handle);

        if ((status == NERR_Success || status == ERROR_MORE_DATA) && groups != NULL) {
            for (DWORD i = 0; i < entries_read; i++) {
                std::wcout << L"\nGroup: " << groups[i].grpi1_name << std::endl;

                LPWSTR sid_str = GetStringSID(SidTypeGroup, groups[i].grpi1_name);
                if (sid_str) {
                    std::wcout << L"SID: " << sid_str << std::endl;

                    PSID group_sid = NULL;
                    if (g_api.ConvertStringSidToSid(sid_str, &group_sid)) {
                        LSA_OBJECT_ATTRIBUTES obj_attrs;
                        ZeroMemory(&obj_attrs, sizeof(obj_attrs));

                        LSA_HANDLE hLsa = NULL;
                        NTSTATUS lsa_status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
                            POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
                            &hLsa);

                        if (lsa_status == STATUS_SUCCESS) {
                            PLSA_UNICODE_STRING rights = NULL;
                            ULONG rights_count = 0;

                            std::wcout << L"Privileges:" << std::endl;
                            NTSTATUS enum_status = g_api.LsaEnumerateAccountRights(hLsa, group_sid, &rights, &rights_count);
                            if (enum_status == STATUS_SUCCESS) {
                                for (ULONG j = 0; j < rights_count; j++) {
                                    std::wcout << L"   " << rights[j].Buffer << std::endl;
                                }
                            }
                            else {
                                std::wcout << L"  (none)" << std::endl;
                            }
                        }
                    }
                }
                LocalFree(sid_str);
            }
        }

        if (groups != NULL) {
            g_api.NetApiBufferFree(groups);
            groups = NULL;
        }
    } while (status == ERROR_MORE_DATA);
}


void CreateUser() {
    WCHAR username[256];
    WCHAR password[256];

    std::wcout << L"Enter username: ";
    std::wcin >> username;
    std::wcout << L"Enter password: ";
    std::wcin >> password;

    USER_INFO_1 user_info;
    ZeroMemory(&user_info, sizeof(user_info));

    user_info.usri1_name = username;
    user_info.usri1_password = password;
    user_info.usri1_priv = USER_PRIV_USER;
    user_info.usri1_flags = UF_SCRIPT | UF_NORMAL_ACCOUNT;
    user_info.usri1_home_dir = NULL;
    user_info.usri1_comment = NULL;
    user_info.usri1_script_path = NULL;

    DWORD error = 0;
    NET_API_STATUS status = g_api.NetUserAdd(NULL, 1, (LPBYTE)&user_info, &error);

    if (status == NERR_Success)
        std::wcout << L"User '" << username << L"' created successfully!" << std::endl;
    else
        std::wcerr << L"Error " << status << L" creating user '" << username << L"'!" << std::endl;
}


void DeleteUser() {
    WCHAR username[256];
    std::wcout << L"Enter username to delete: ";
    std::wcin >> username;

    NET_API_STATUS status = g_api.NetUserDel(NULL, username);

    if (status == NERR_Success)
        std::wcout << L"User '" << username << L"' deleted successfully!" << std::endl;
    else
        std::wcerr << L"Error " << status << L" deleting user '" << username << L"'!" << std::endl;
}

void AddPrivilegeToUser() {
    WCHAR username[256];
    WCHAR privilege[256];

    std::wcout << L"Enter username: ";
    std::wcin >> username;
    std::wcout << L"Enter privilege: ";
    std::wcin >> privilege;

    // конвертация привилегии в формат LSA
    LSA_UNICODE_STRING lsa_priv;
    ULONG length = wcslen(privilege) * sizeof(WCHAR);
    lsa_priv.Length = (USHORT)length;
    lsa_priv.MaximumLength = (USHORT)(length + sizeof(WCHAR));
    lsa_priv.Buffer = privilege;

    LPWSTR sid_str = GetStringSID(SidTypeUser, username);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for user '" << username << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LSA_OBJECT_ATTRIBUTES obj_attrs;
    ZeroMemory(&obj_attrs, sizeof(obj_attrs));

    LSA_HANDLE hLsa = NULL;
    NTSTATUS status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
        &hLsa);

    if (status == STATUS_SUCCESS) {
        NTSTATUS add_status = g_api.LsaAddAccountRights(hLsa, sid, &lsa_priv, 1);
        if (add_status == STATUS_SUCCESS)
            std::wcout << L"Privilege added successfully to user '" << username << L"'!" << std::endl;
        else {
            ULONG win_error = g_api.LsaNtStatusToWinError(add_status);
            std::wcerr << L"Error " << win_error << L" adding privilege!" << std::endl;
        }
    }

    LocalFree(sid);
    LocalFree(sid_str);
}


void RemovePrivilegeFromUser() {
    WCHAR username[256];
    WCHAR privilege[256];

    std::wcout << L"Enter username: ";
    std::wcin >> username;
    std::wcout << L"Enter privilege to remove: ";
    std::wcin >> privilege;

    LSA_UNICODE_STRING lsa_priv;
    ULONG length = wcslen(privilege) * sizeof(WCHAR);
    lsa_priv.Length = (USHORT)length;
    lsa_priv.MaximumLength = (USHORT)(length + sizeof(WCHAR));
    lsa_priv.Buffer = privilege;

    LPWSTR sid_str = GetStringSID(SidTypeUser, username);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for user '" << username << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LSA_OBJECT_ATTRIBUTES obj_attrs;
    ZeroMemory(&obj_attrs, sizeof(obj_attrs));

    LSA_HANDLE hLsa = NULL;
    NTSTATUS status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
        &hLsa);

    if (status == STATUS_SUCCESS) {
        NTSTATUS remove_status = g_api.LsaRemoveAccountRights(hLsa, sid, FALSE, &lsa_priv, 1);

        if (remove_status == STATUS_SUCCESS)
            std::wcout << L"Privilege removed successfully from user '" << username << L"'!" << std::endl;
        else {
            ULONG win_error = g_api.LsaNtStatusToWinError(remove_status);
            std::wcerr << L"Error " << win_error << L" removing privilege!" << std::endl;
        }
    }

    LocalFree(sid);
    LocalFree(sid_str);
}


void CreateGroup() {
    WCHAR groupname[256];

    std::wcout << L"Enter group name: ";
    std::wcin >> groupname;

    LOCALGROUP_INFO_0 group_info;
    group_info.lgrpi0_name = groupname;

    NET_API_STATUS status = g_api.NetLocalGroupAdd(NULL, 0, (LPBYTE)&group_info, NULL);

    if (status == NERR_Success)
        std::wcout << L"Group '" << groupname << L"' created successfully!" << std::endl;
    else {
        ULONG win_error = g_api.LsaNtStatusToWinError(status);
        std::wcerr << L"Error " << win_error << L" creating group '" << groupname << L"'!" << std::endl;
    }
}


void DeleteGroup() {
    WCHAR groupname[256];

    std::wcout << L"Enter group name to delete: ";
    std::wcin >> groupname;

    NET_API_STATUS status = g_api.NetLocalGroupDel(NULL, groupname);

    if (status == NERR_Success)
        std::wcout << L"Group '" << groupname << L"' deleted successfully!" << std::endl;
    else {
        ULONG win_error = g_api.LsaNtStatusToWinError(status);
        std::wcerr << L"Error " << win_error << L" deleting group '" << groupname << L"'!" << std::endl;
    }
}


void AddPrivilegeToGroup() {
    WCHAR groupname[256];
    WCHAR privilege[256];

    std::wcout << L"Enter group name: ";
    std::wcin >> groupname;
    std::wcout << L"Enter privilege: ";
    std::wcin >> privilege;

    LSA_UNICODE_STRING lsa_priv;
    ULONG length = wcslen(privilege) * sizeof(WCHAR);
    lsa_priv.Length = (USHORT)length;
    lsa_priv.MaximumLength = (USHORT)(length + sizeof(WCHAR));
    lsa_priv.Buffer = privilege;

    LPWSTR sid_str = GetStringSID(SidTypeGroup, groupname);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for group '" << groupname << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LSA_OBJECT_ATTRIBUTES obj_attrs;
    ZeroMemory(&obj_attrs, sizeof(obj_attrs));

    LSA_HANDLE hLsa = NULL;
    NTSTATUS status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
        &hLsa);

    if (status == STATUS_SUCCESS) {
        NTSTATUS add_status = g_api.LsaAddAccountRights(hLsa, sid, &lsa_priv, 1);

        if (add_status == STATUS_SUCCESS)
            std::wcout << L"Privilege added successfully to group '" << groupname << L"'!" << std::endl;
        else {
            ULONG win_error = g_api.LsaNtStatusToWinError(add_status);
            std::wcerr << L"Error " << win_error << L" adding privilege to group!" << std::endl;
        }
    }

    LocalFree(sid);
    LocalFree(sid_str);
}


void RemovePrivilegeFromGroup() {
    WCHAR groupname[256];
    WCHAR privilege[256];

    std::wcout << L"Enter group name: ";
    std::wcin >> groupname;
    std::wcout << L"Enter privilege to remove: ";
    std::wcin >> privilege;

    LSA_UNICODE_STRING lsa_priv;
    ULONG length = wcslen(privilege) * sizeof(WCHAR);
    lsa_priv.Length = (USHORT)length;
    lsa_priv.MaximumLength = (USHORT)(length + sizeof(WCHAR));
    lsa_priv.Buffer = privilege;

    LPWSTR sid_str = GetStringSID(SidTypeGroup, groupname);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for group '" << groupname << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LSA_OBJECT_ATTRIBUTES obj_attrs;
    ZeroMemory(&obj_attrs, sizeof(obj_attrs));

    LSA_HANDLE hLsa = NULL;
    NTSTATUS status = g_api.LsaOpenPolicy(NULL, &obj_attrs,
        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
        &hLsa);

    if (status == STATUS_SUCCESS) {
        NTSTATUS remove_status = g_api.LsaRemoveAccountRights(hLsa, sid, FALSE, &lsa_priv, 1);

        if (remove_status == STATUS_SUCCESS)
            std::wcout << L"Privilege removed successfully from group '" << groupname << L"'!" << std::endl;
        else {
            ULONG win_error = g_api.LsaNtStatusToWinError(remove_status);
            std::wcerr << L"Error " << win_error << L" removing privilege from group!" << std::endl;
        }
    }

    LocalFree(sid);
    LocalFree(sid_str);
}

void AddUserToGroup() {
    WCHAR groupname[256];
    WCHAR username[256];

    std::wcout << L"Enter group name: ";
    std::wcin >> groupname;
    std::wcout << L"Enter username: ";
    std::wcin >> username;

    LPWSTR sid_str = GetStringSID(SidTypeUser, username);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for user '" << username << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LOCALGROUP_MEMBERS_INFO_0 member_info;
    member_info.lgrmi0_sid = sid;

    NET_API_STATUS status = g_api.NetLocalGroupAddMembers(NULL, groupname, 0, (LPBYTE)&member_info, 1);

    if (status == NERR_Success) 
        std::wcout << L"User '" << username << L"' added to group '" << groupname << L"' successfully!" << std::endl;
    else {
        ULONG win_error = g_api.LsaNtStatusToWinError(status);
        std::wcerr << L"Error " << win_error << L" adding user to group!" << std::endl;
    }

    LocalFree(sid);
    LocalFree(sid_str);
}


void RemoveUserFromGroup() {
    WCHAR groupname[256];
    WCHAR username[256];

    std::wcout << L"Enter group name: ";
    std::wcin >> groupname;
    std::wcout << L"Enter username: ";
    std::wcin >> username;

    LPWSTR sid_str = GetStringSID(SidTypeUser, username);
    if (!sid_str) {
        std::wcerr << L"Failed to get SID for user '" << username << L"'!" << std::endl;
        return;
    }

    PSID sid = NULL;
    if (!g_api.ConvertStringSidToSid(sid_str, &sid)) {
        std::wcerr << L"Failed to convert SID string!" << std::endl;
        LocalFree(sid_str);
        return;
    }

    LOCALGROUP_MEMBERS_INFO_0 member_info;
    member_info.lgrmi0_sid = sid;

    NET_API_STATUS status = g_api.NetLocalGroupDelMembers(NULL, groupname, 0, (LPBYTE)&member_info, 1);

    if (status == NERR_Success)
        std::wcout << L"User '" << username << L"' removed from group '" << groupname << L"' successfully!" << std::endl;
    else {
        ULONG win_error = g_api.LsaNtStatusToWinError(status);
        std::wcerr << L"Error " << win_error << L" removing user from group!" << std::endl;
    }

    LocalFree(sid);
    LocalFree(sid_str);
}

void DisplayMenu() {
    std::cout << "\n=========================================" << std::endl;
    std::cout << "1. List all users" << std::endl;
    std::cout << "2. List all groups" << std::endl;
    std::cout << "3. Create user" << std::endl;
    std::cout << "4. Create group" << std::endl;
    std::cout << "5. Add privilege to user" << std::endl;
    std::cout << "6. Remove privilege from user" << std::endl;
    std::cout << "7. Delete user" << std::endl;
    std::cout << "8. Delete group" << std::endl;
    std::cout << "9. Add privilege to group" << std::endl;
    std::cout << "10. Remove privilege from group" << std::endl;
    std::cout << "11. Add user to group" << std::endl;
    std::cout << "12. Remove user from group" << std::endl;
    std::cout << "13. Exit" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Select option: ";
}


int main() {
    setlocale(LC_ALL, "");

    if (!InitApi()) {
        std::cerr << "Error! Failed to initialize API functions!" << std::endl;
        return 1;
    }

    int choice = 0;
    while (true) {
        DisplayMenu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            ListAllUsers();
            break;
        case 2:
            ListAllGroups();
            break;
        case 3:
            CreateUser();
            break;
        case 4:
            CreateGroup();
            break;
        case 5:
            AddPrivilegeToUser();
            break;
        case 6:
            RemovePrivilegeFromUser();
            break;
        case 7:
            DeleteUser();
            break;
        case 8:
            DeleteGroup();
            break;
        case 9:
            AddPrivilegeToGroup();
            break;
        case 10:
            RemovePrivilegeFromGroup();
            break;
        case 11:
            AddUserToGroup();
            break;
        case 12:
            RemoveUserFromGroup();
            break;
        case 13:
            FreeApi();
            return 0;
        default:
            std::cout << "Error! Invalid option" << std::endl;
            break;
        }
    }

    return 0;
}