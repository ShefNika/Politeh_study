/*++

Copyright (c) 1999 - 2002  Microsoft Corporation

Module Name:

    nullFilter.c

Abstract:

    This is the main module of the nullFilter mini filter driver.
    It is a simple minifilter that registers itself with the main filter
    for no callback operations.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h> // for RtStringCchCopyW and LengthW
#include <ntifs.h> // for SecLookupAccountSid, SeQueryInformationToken
#pragma comment(lib, "Ksecdd.lib") // also for them
#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//---------------------------------------------------------------------------
//      Global variables
//---------------------------------------------------------------------------


typedef struct _NULL_FILTER_DATA {

    //
    //  The filter handle that results from a call to
    //  FltRegisterFilter.
    //

    PFLT_FILTER FilterHandle;

} NULL_FILTER_DATA, *PNULL_FILTER_DATA;

#define MAX_RULES 32
#define MAX_PATH_LEN 260
#define MAX_USER_LEN 32

typedef struct _ACCESS_RULE {
    WCHAR FilePath[MAX_PATH_LEN];
    WCHAR UserName[MAX_USER_LEN];
    BOOLEAN CanRead;
    BOOLEAN CanWrite;
} ACCESS_RULE, * PACCESS_RULE;

ACCESS_RULE g_Rules[MAX_RULES];
ULONG g_RuleCount = 0;


/*************************************************************************
    Prototypes for the startup and unload routines used for
    this Filter.

    Implementation in nullFilter.c
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
NullUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
NullQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
NullPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

NTSTATUS
LoadAccessRules(
    VOID
);

BOOLEAN
CheckAccessForFile(
    _In_ PUNICODE_STRING FileName,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ PCWSTR UserName
);

BOOLEAN
PathsReferToSameFile(
    _In_ PUNICODE_STRING NtFileName,
    _In_ PCWSTR RuleDosPath
);

BOOLEAN
GetRequestUserName(
    _In_ PFLT_CALLBACK_DATA Data,
    _Out_writes_(MAX_USER_LEN) PWCHAR UserName
);


//
//  Structure that contains all the global data structures
//  used throughout NullFilter.
//

NULL_FILTER_DATA NullFilterData;

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, NullUnload)
#pragma alloc_text(PAGE, NullQueryTeardown)
#endif

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, // перед операцией открытия файла
      0,
      NullPreCreate, // вызвать
      NULL }, // после ничего не вызывать

    { IRP_MJ_OPERATION_END }
};


//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                               //  Operation callbacks

    NullUnload,                         //  FilterUnload

    NULL,                               //  InstanceSetup
    NullQueryTeardown,                  //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};


/*************************************************************************
    Filter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver. This
    registers the miniFilter with FltMgr and initializes all
    its global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.
    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    //
    //  Register with FltMgr
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &NullFilterData.FilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );


    if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //
        status = LoadAccessRules();
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(NullFilterData.FilterHandle);
            return status;
        }
        DbgPrint("NullFilter: rules loaded, count = %lu\n", g_RuleCount);
        status = FltStartFiltering( NullFilterData.FilterHandle );

        if (!NT_SUCCESS( status )) {
            FltUnregisterFilter( NullFilterData.FilterHandle );
        }
    }
    return status;
}

NTSTATUS
NullUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unloaded indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns the final status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    FltUnregisterFilter( NullFilterData.FilterHandle );

    return STATUS_SUCCESS;
}

NTSTATUS
NullQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This is the instance detach routine for this miniFilter driver.
    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Indicating where this detach request came from.

Return Value:

    Returns the status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    return STATUS_SUCCESS;
}

BOOLEAN
PathsReferToSameFile(
    _In_ PUNICODE_STRING NtFileName,
    _In_ PCWSTR RuleDosPath
)
{
    WCHAR ntTail[MAX_PATH_LEN];
    WCHAR ruleTail[MAX_PATH_LEN];
    USHORT i, j;
    USHORT ntChars;
    BOOLEAN found = FALSE;
    if (NtFileName == NULL || NtFileName->Buffer == NULL || RuleDosPath == NULL) 
        return FALSE;
    RtlZeroMemory(ntTail, sizeof(ntTail));
    RtlZeroMemory(ruleTail, sizeof(ruleTail));

    for (i = 0, j = 0; RuleDosPath[i] != L'\0' && j < MAX_PATH_LEN - 1; i++) {
        if (RuleDosPath[i] == L'\\') 
            found = TRUE;
        if (found) 
            ruleTail[j++] = RuleDosPath[i];
    }
    ntChars = NtFileName->Length / sizeof(WCHAR);
    found = FALSE;
    {
        int slashCount = 0;
        for (i = 0, j = 0; i < ntChars && j < MAX_PATH_LEN - 1; i++) {
            if (NtFileName->Buffer[i] == L'\\') {
                slashCount++;
                if (slashCount == 3) 
                    found = TRUE;
            }
            if (found) {
                ntTail[j++] = NtFileName->Buffer[i];
            }
        }
    }
    if (!found) 
        return FALSE;
    if (_wcsicmp(ntTail, ruleTail) == 0) 
        return TRUE;
    return FALSE;
}

NTSTATUS
LoadAccessRules(
    VOID
)
{
    NTSTATUS status;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK ioStatus;
    UNICODE_STRING fileName;
    CHAR buffer[1024];
    LARGE_INTEGER byteOffset;
    ULONG bytesRead;
    g_RuleCount = 0;
    RtlInitUnicodeString(&fileName, L"\\??\\C:\\config.txt"); 
    InitializeObjectAttributes(&objAttr,
        &fileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);
    status = ZwCreateFile(&fileHandle,
        GENERIC_READ,
        &objAttr,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    RtlZeroMemory(buffer, sizeof(buffer));
    byteOffset.QuadPart = 0;
    status = ZwReadFile(fileHandle, 
        NULL,
        NULL,
        NULL,
        &ioStatus,
        buffer,
        sizeof(buffer) - 1,
        &byteOffset,
        NULL);
    ZwClose(fileHandle);
    if (!NT_SUCCESS(status)) 
        return status;
    bytesRead = (ULONG)ioStatus.Information;
    buffer[bytesRead] = '\0';
    {
        CHAR* context = NULL;
        CHAR* line = strtok_s(buffer, "\r\n", &context); 
        while (line != NULL && g_RuleCount < MAX_RULES) {
            CHAR* path = NULL;
            CHAR* user = NULL;
            CHAR* rights = NULL;
            CHAR* context2 = NULL;
            path = strtok_s(line, ";", &context2);
            user = strtok_s(NULL, ";", &context2);
            rights = strtok_s(NULL, ";", &context2);
            if (path != NULL && user != NULL && rights != NULL) {
                ANSI_STRING ansiPath;
                ANSI_STRING ansiUser;
                UNICODE_STRING uniPath;
                UNICODE_STRING uniUser;
                RtlInitAnsiString(&ansiPath, path);
                RtlInitAnsiString(&ansiUser, user);
                uniPath.Buffer = g_Rules[g_RuleCount].FilePath;
                uniPath.Length = 0;
                uniPath.MaximumLength = sizeof(g_Rules[g_RuleCount].FilePath);
                uniUser.Buffer = g_Rules[g_RuleCount].UserName;
                uniUser.Length = 0;
                uniUser.MaximumLength = sizeof(g_Rules[g_RuleCount].UserName);
                if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&uniPath, &ansiPath, FALSE)) &&
                    NT_SUCCESS(RtlAnsiStringToUnicodeString(&uniUser, &ansiUser, FALSE))) { // FALSE - исп-ся уже заданный буфер
                    g_Rules[g_RuleCount].CanRead = (strchr(rights, 'r') != NULL) ? TRUE : FALSE;
                    g_Rules[g_RuleCount].CanWrite = (strchr(rights, 'w') != NULL) ? TRUE : FALSE;
                    g_RuleCount++;
                }
            }
            line = strtok_s(NULL, "\r\n", &context);
        }
    }
    return STATUS_SUCCESS;
}

BOOLEAN
CheckAccessForFile(
    _In_ PUNICODE_STRING FileName,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ PCWSTR UserName
)
{
    ULONG i;
    BOOLEAN needRead = FALSE;
    BOOLEAN needWrite = FALSE;
    BOOLEAN fileFound = FALSE;
    if (DesiredAccess & (FILE_READ_DATA | GENERIC_READ)) 
        needRead = TRUE;
    if (DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | GENERIC_WRITE)) 
        needWrite = TRUE;
    if (!needRead && !needWrite) 
        return TRUE;
    for (i = 0; i < g_RuleCount; i++) {
        if (PathsReferToSameFile(FileName, g_Rules[i].FilePath)) {
            fileFound = TRUE;
            if (_wcsicmp(UserName, g_Rules[i].UserName) == 0) {
                if (needRead && !g_Rules[i].CanRead) 
                    return FALSE;
                if (needWrite && !g_Rules[i].CanWrite) 
                    return FALSE;
                return TRUE;
            }
        }
    }
    if (!fileFound) 
        return TRUE;
    return FALSE;
}

BOOLEAN
GetRequestUserName(
    _In_ PFLT_CALLBACK_DATA Data,
    _Out_writes_(MAX_USER_LEN) PWCHAR UserName
)
{
    PEPROCESS process;
    PACCESS_TOKEN token;
    PTOKEN_USER tokenUser = NULL;
    NTSTATUS status;
    WCHAR nameBuffer[MAX_USER_LEN];
    UNICODE_STRING nameString;
    ULONG nameSize;
    ULONG domainSize = 0;
    SID_NAME_USE nameUse;
    if (Data == NULL || UserName == NULL) 
        return FALSE;
    RtlZeroMemory(UserName, MAX_USER_LEN * sizeof(WCHAR));
    RtlZeroMemory(nameBuffer, sizeof(nameBuffer));
    if (KeGetCurrentIrql() > APC_LEVEL) 
        return FALSE;
    if (Data->Thread == NULL) 
        return FALSE;
    process = IoThreadToProcess(Data->Thread); // получить процесс по потоку
    if (process == NULL) 
        return FALSE;
    token = PsReferencePrimaryToken(process);
    if (token == NULL) 
        return FALSE;
    status = SeQueryInformationToken(token, TokenUser, (PVOID*)&tokenUser);
    PsDereferencePrimaryToken(token);
    if (!NT_SUCCESS(status) || tokenUser == NULL) 
        return FALSE;
    nameString.Buffer = nameBuffer;
    nameString.Length = 0;
    nameString.MaximumLength = sizeof(nameBuffer);
    nameSize = sizeof(nameBuffer);
    status = SecLookupAccountSid(
        tokenUser->User.Sid,
        &nameSize,
        &nameString,
        &domainSize,
        NULL,
        &nameUse
    );
    ExFreePool(tokenUser);
    if (!NT_SUCCESS(status)) 
        return FALSE;
    if (nameString.Buffer == NULL || nameString.Length == 0) 
        return FALSE;
    if (!NT_SUCCESS(RtlStringCchCopyNW(UserName,
        MAX_USER_LEN,
        nameString.Buffer,
        nameString.Length / sizeof(WCHAR)))) {
        return FALSE;
    }
    return TRUE;
}

FLT_PREOP_CALLBACK_STATUS
NullPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION nameInfo;
    ACCESS_MASK desiredAccess;
    WCHAR currentUser[MAX_USER_LEN];
    BOOLEAN gotUser;

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    //DbgPrint("NullFilter: NullPreCreate called\n");

    status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
        &nameInfo
    );
    if (!NT_SUCCESS(status)) 
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    status = FltParseFileNameInformation(nameInfo);
    if (!NT_SUCCESS(status)) {
        FltReleaseFileNameInformation(nameInfo);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    gotUser = GetRequestUserName(Data, currentUser);
    if (!gotUser) {
        FltReleaseFileNameInformation(nameInfo);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    //DbgPrint("NullFilter: current user = %ws\n", currentUser);
    desiredAccess = Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
    //DbgPrint("NullFilter: desired access = 0x%08X\n", desiredAccess);

    if (!CheckAccessForFile(&nameInfo->Name, desiredAccess, currentUser)) {
        DbgPrint("NullFilter: denied for user %ws, file = %wZ\n", currentUser, &nameInfo->Name);
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;
        FltReleaseFileNameInformation(nameInfo);
        return FLT_PREOP_COMPLETE;
    }
    FltReleaseFileNameInformation(nameInfo);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
