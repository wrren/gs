#include <stdlib.h>
#include <stdio.h>

#include <gs/loader/lib.h>
#include <winternl.h>

typedef 
NTSTATUS
(*GsNtCreateFile)(
    PHANDLE            FileHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK   IoStatusBlock,
    PLARGE_INTEGER     AllocationSize,
    ULONG              FileAttributes,
    ULONG              ShareAccess,
    ULONG              CreateDisposition,
    ULONG              CreateOptions,
    PVOID              EaBuffer,
    ULONG              EaLength
);

typedef
NTSTATUS 
(*GsNtClose)(
    HANDLE Handle
);

int main(int argc, char** argv) {
    if(GsLibraryInit() == FALSE) {
        printf("[ERROR] Library System Initialization Failed!\n");
        return EXIT_FAILURE;
    }

    PGS_LIBRARY Library = GsLibraryLoad("pots.dll");
    if(Library == NULL) {
        printf("[ERROR] Load Library Failed!\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    Library = GsLibraryLoad("ntdll.dll");
    if(Library == NULL) {
        printf("[ERROR] Load Library Failed!\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    GsNtCreateFile ExportedCreateFile = (GsNtCreateFile) GsLibraryGetFunctionAddressByName(
        Library,
        "NtCreateFile"
    );

    GsNtClose ExportedClose = (GsNtClose) GsLibraryGetFunctionAddressByName(
        Library,
        "NtClose"
    );

    if(ExportedCreateFile == NULL) {
        printf("[ERROR] Failed to find NtCreateFile.\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    if(ExportedClose == NULL) {
        printf("[ERROR] Failed to find NtClose.\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    HANDLE              FileHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      FilePath;

    RtlInitUnicodeString(&FilePath, L"\\??\\\\C:\\Windows\\System32\\ntdll.dll");
    InitializeObjectAttributes(&ObjectAttributes, &FilePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS Status = ExportedCreateFile(
        &FileHandle,
        FILE_GENERIC_READ,
        &ObjectAttributes,
        &IoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        0,
        NULL,
        0
    );

    if(!NT_SUCCESS(Status)) {
        printf("[ERROR] Failed to open file.\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    ExportedClose(FileHandle);

    GsLibraryRelease();

    return EXIT_SUCCESS;
}