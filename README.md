# General Systems C Library

This is a pure-C utility library, initially for Windows only, that I'm using as a learning project. The first
goal of the project is implementation of a manual image mapper, including recursive import resolution and 
the ability to resolve API sets to their real library names.

## Manual Mapper

This library provides, among other things, functions for manually mapping a given DLL into memory, bypassing the
loader. By default, it will load recursively, loading dependency libraries along with the one requested. The following
is an example of how to load `ntdll.dll` manually, get the address of `NtCreateFile` from the loaded library and call it.

```c

#include <gs/loader/lib.h>
#include <winternl.h>

/// Type definition for NtCreateFile signature
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

/// Type definition for NtClose signature
typedef
NTSTATUS 
(*GsNtClose)(
    HANDLE Handle
);

int LibraryLoadExample() {
    HANDLE              FileHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      FilePath;
    PGS_LIBRARY         Library;
    GsNtCreateFile      ExportedCreateFile;
    GsNtClose           ExportedClose;

    // GsLibraryInit must be called before any calls to GsLibraryLoad
    if(GsLibraryInit() == FALSE) {
        return -1;
    }

    PGS_LIBRARY Library = GsLibraryLoad("ntdll.dll");
    if(Library == NULL) {
        GsLibraryRelease();
        return -1;
    }

    ExportedCreateFile = (GsNtCreateFile) GsLibraryGetFunctionAddressByName(
        Library,
        "NtCreateFile"
    );

    ExportedClose = (GsNtClose) GsLibraryGetFunctionAddressByName(
        Library,
        "NtClose"
    );

    if(ExportedCreateFile == NULL) {
        GsLibraryRelease();
        return -1;
    }

    if(ExportedClose == NULL) {
        GsLibraryRelease();
        return -1;
    }

    // We'll open the original ntdll.dll
    RtlInitUnicodeString(&FilePath, L"\\??\\\\C:\\Windows\\System32\\ntdll.dll");
    InitializeObjectAttributes(&ObjectAttributes, &FilePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Let's call our resolved NtCreateFile
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

    // The file was opened successfully
    if(!NT_SUCCESS(Status)) {
        GsLibraryRelease();
        return -1;
    }

    // Make sure to close the file handle when we're done
    ExportedClose(FileHandle);

    // Make sure you call GsLibraryRelease at the end of your program
    GsLibraryRelease();

    return 0;
}


```