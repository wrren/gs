#include <gs/pe/pe.h>
#include <gs/util/serializer.h>
#include <gs/loader/lib.h>
#include <stdio.h>

#define GS_RVA_CAST(ImageBase, Type, Offset) (Type)(((PUINT8) ImageBase) + Offset)
#define GS_RVA_IS_VALID(PE, Offset) (Offset <= PE->OptionalHeader.SizeOfImage)

/**
 * @brief Signature for a DLL entry point
 * 
 */
typedef BOOL (*GsLibraryEntryPoint)(
    _In_ HINSTANCE BaseAddress,
    _In_ DWORD     Reason,
    _In_ LPVOID    Reserved
);

/**
 * @brief Apply relocations to the given loaded image.
 * 
 * @param ImageBase     Image base
 * @param PE            PE image struct
 * @return GsPeError    GsPeSuccess on success
 */
_Success_(return == GsPeSuccess)
static GsPeError GsPepApplyRelocations(
    _In_ PVOID  ImageBase,
    _In_ PGS_PE PE
);

/**
 * @brief Traverse the IAT and resolve required import address for the given loaded image.
 * 
 * @param ImageBase     Image base
 * @param PE            PE image struct
 * @return GsPeError    GsPeSuccess on success
 */
_Success_(return == GsPeSuccess)
static GsPeError GsPepResolveImports(
    _In_ PVOID ImageBase,
    _In_ PGS_PE PE
);

/**
 * @brief Find the section header that corresponds to the given RVA
 * 
 * @param PE    PE Image struct
 * @param RVA   Relative virtual address of the target section
 * @return PIMAGE_SECTION_HEADER    Pointer to the corresponding section header or NULL if it could not be found
 */
_Success_(return != NULL)
static PIMAGE_SECTION_HEADER GsPepFindSection(
    _In_ PGS_PE PE,
    _In_ DWORD  RVA
);

/**
 * @brief Traverse the exports of the given image and populate the given list.
 * 
 * @param ImageBase     Image base
 * @param PE            PE image struct
 * @param Exports       Output list of discovered exports
 * @return GsPeError    GsPeSuccess on success
 */
_Success_(return == GsPeSuccess)
static GsPeError GsPepResolveExports(
    _In_ PVOID          ImageBase,
    _In_ PGS_PE         PE,
    _Inout_ PGS_LIST    Exports
);

_Success_(return != NULL)
PGS_PE GsPeReadFromFile(
    _In_z_ LPCWSTR          Path,
    _Outptr_opt_ GsPeError* Error
)
{
    PGS_ARENA Arena = GsArenaWithReservationAndPageProtection(GS_ARENA_DEFAULT_RESERVATION, PAGE_EXECUTE_READWRITE);
    if(Arena == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        return NULL;
    }

    HANDLE FileHandle = CreateFile(
        Path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(FileHandle == INVALID_HANDLE_VALUE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    GsArenaAddCleanupTask(Arena, CloseHandle, FileHandle);

    PGS_PE PE = (PGS_PE) GsArenaAlloc(Arena, sizeof(GS_PE));
    if(PE == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }
    PE->Arena = Arena;

    LARGE_INTEGER FileSize = { 0 };
    DWORD NumberOfBytesRead;

    if(GetFileSizeEx(FileHandle, &FileSize) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeFileReadError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(ReadFile(FileHandle, &(PE->DosHeader), sizeof(PE->DosHeader), &NumberOfBytesRead, NULL) == FALSE || NumberOfBytesRead != sizeof(PE->DosHeader)) {
        if(Error != NULL) {
            *Error = GsPeFileReadError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(PE->DosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(SetFilePointer(FileHandle, PE->DosHeader.e_lfanew, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        if(Error != NULL) {
            *Error = GsPeFileReadError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(ReadFile(FileHandle, &(PE->Signature), sizeof(PE->Signature), &NumberOfBytesRead, NULL) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(PE->Signature != IMAGE_NT_SIGNATURE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(ReadFile(FileHandle, &(PE->FileHeader), sizeof(PE->FileHeader), &NumberOfBytesRead, NULL) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(PE->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        if(Error != NULL) {
            *Error = GsPeUnhandledMachineError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(ReadFile(FileHandle, &(PE->OptionalHeader), PE->FileHeader.SizeOfOptionalHeader, &NumberOfBytesRead, NULL) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    PE->Sections = (PGS_PE_SECTION) GsArenaAlloc(Arena, PE->FileHeader.NumberOfSections * sizeof(GS_PE_SECTION));
    if(PE->Sections == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    for(SIZE_T i = 0; i < PE->FileHeader.NumberOfSections; i++) {
        if(ReadFile(FileHandle, &(PE->Sections[i].Header), sizeof(IMAGE_SECTION_HEADER), &NumberOfBytesRead, NULL) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeFileReadError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        PE->Sections[i].Data = GsArenaAlloc(Arena, PE->Sections[i].Header.SizeOfRawData);
        if(PE->Sections[i].Data == NULL) {
            if(Error != NULL) {
                *Error = GsPeMemoryAllocationError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        DWORD CurrentPointer = SetFilePointer(FileHandle, 0, NULL, FILE_CURRENT);

        if(SetFilePointer(FileHandle, PE->Sections[i].Header.PointerToRawData, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
            if(Error != NULL) {
                *Error = GsPeFileReadError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        if(ReadFile(FileHandle, PE->Sections[i].Data, PE->Sections[i].Header.SizeOfRawData, &NumberOfBytesRead, NULL) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeFileReadError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        if(SetFilePointer(FileHandle, CurrentPointer, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
            if(Error != NULL) {
                *Error = GsPeFileReadError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }
    }

    return PE;
}

_Success_(return != NULL)
PGS_PE GsPeReadFromMemory(
    _In_ PVOID              ImageBase,
    _Inout_ PGS_LIST        Exports,
    _Outptr_opt_ GsPeError* Error    
)
{
    PGS_ARENA Arena = GsArena(GS_ARENA_DEFAULT_RESERVATION, PAGE_EXECUTE_READWRITE);
    if(Arena == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        return NULL;
    }
    
    PGS_PE PE = (PGS_PE) GsArenaAlloc(Arena, sizeof(GS_PE));
    if(PE == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }
    PE->Arena = Arena;
    SIZE_T Offset       = 0;
    SIZE_T ImageSize    = SIZE_MAX;

    if(GsSerialize(&(PE->DosHeader), sizeof(PE->DosHeader), ImageBase, sizeof(PE->DosHeader), &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }
    
    if(PE->DosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }
    Offset = PE->DosHeader.e_lfanew;

    if(GsSerialize(&(PE->Signature), sizeof(PE->Signature), ImageBase, sizeof(PE->Signature), &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(PE->Signature != IMAGE_NT_SIGNATURE) {
        if(Error != NULL) {
            *Error = GsPeInvalidFileFormatError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }
    
    if(GsSerialize(&(PE->FileHeader), sizeof(PE->FileHeader), ImageBase, sizeof(PE->FileHeader), &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    if(GsSerialize(&(PE->OptionalHeader), sizeof(PE->OptionalHeader), ImageBase, sizeof(PE->OptionalHeader), &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    ImageSize = PE->OptionalHeader.SizeOfImage;

    PE->Sections = (PGS_PE_SECTION) GsArenaAlloc(Arena, PE->FileHeader.NumberOfSections * sizeof(GS_PE_SECTION));
    if(PE->Sections == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        GsArenaRelease(Arena);
        return NULL;
    }

    for(SIZE_T i = 0; i < PE->FileHeader.NumberOfSections; i++) {
        if(GsSerialize(&(PE->Sections[i].Header), sizeof(IMAGE_SECTION_HEADER), ImageBase, sizeof(IMAGE_SECTION_HEADER), &Offset) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeSerializationError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        SIZE_T NextHeaderOffset = Offset;
        Offset = PE->Sections[i].Header.VirtualAddress;
        SIZE_T SectionSize = PE->Sections[i].Header.Misc.VirtualSize;
        PE->Sections[i].Data = GsArenaAlloc(Arena, SectionSize);

        if(GsSerialize(PE->Sections[i].Data, SectionSize, ImageBase, SectionSize, &Offset) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeSerializationError;
            }
            GsArenaRelease(Arena);
            return NULL;
        }

        Offset = NextHeaderOffset;
    }

    return PE;
}

_Success_(return != NULL)
PVOID GsPeLoad(
    _In_ PGS_PE             PE,
    _Inout_ PGS_LIST        Exports,
    _Outptr_opt_ GsPeError* Error     
)
{
    SIZE_T ImageSize    = ImageSize = PE->OptionalHeader.SizeOfImage;
    DWORD EntryPointRVA = PE->OptionalHeader.AddressOfEntryPoint;
    
    PVOID ImageBase = GsArenaAlloc(PE->Arena, PE->OptionalHeader.SizeOfImage);
    if(ImageBase == NULL) {
        if(Error != NULL) {
            *Error = GsPeMemoryAllocationError;
        }
        return NULL;
    }

    ZeroMemory(ImageBase, ImageSize);

    SIZE_T Offset = 0;
    if(GsSerialize(ImageBase, ImageSize, &(PE->DosHeader), sizeof(PE->DosHeader), &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        return NULL;
    }

    Offset = PE->DosHeader.e_lfanew;
    if(GsSerialize(ImageBase, ImageSize, &(PE->OptionalHeader), PE->FileHeader.SizeOfOptionalHeader, &Offset) == FALSE) {
        if(Error != NULL) {
            *Error = GsPeSerializationError;
        }
        return NULL;
    }

    for(SIZE_T i = 0; i < PE->FileHeader.NumberOfSections; i++) {
        if(GsSerialize(ImageBase, ImageSize, &(PE->Sections[i].Header), sizeof(IMAGE_SECTION_HEADER), &Offset) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeSerializationError;
            }
            return NULL;
        }
    }

    for(SIZE_T i = 0; i < PE->FileHeader.NumberOfSections; i++) {
        PGS_PE_SECTION Section = &(PE->Sections[i]);

        Offset = Section->Header.VirtualAddress;
        if(GsSerialize(ImageBase, ImageSize, Section->Data, Section->Header.SizeOfRawData, &Offset) == FALSE) {
            if(Error != NULL) {
                *Error = GsPeSerializationError;
            }
            return NULL;
        }
    }

    GsPeError RelocationsResult = GsPepApplyRelocations(ImageBase, PE);
    if(RelocationsResult) {
        if(Error != NULL) {
            *Error = RelocationsResult;
        }
        return NULL;
    }

    GsPeError ExportResolutionResult = GsPepResolveExports(ImageBase, PE, Exports);
    if(ExportResolutionResult) {
        if(Error != NULL) {
            *Error = ExportResolutionResult;
        }
        return NULL;
    }

    GsPeError ImportResolutionResult = GsPepResolveImports(ImageBase, PE);
    if(ImportResolutionResult) {
        if(Error != NULL) {
            *Error = ImportResolutionResult;
        }
        return NULL;
    }

    if(EntryPointRVA != 0 && PE->FileHeader.Characteristics & IMAGE_FILE_DLL) {
        GsLibraryEntryPoint DllMain = GS_RVA_CAST(ImageBase, GsLibraryEntryPoint, EntryPointRVA);

        if(DllMain((HINSTANCE) ImageBase, DLL_PROCESS_ATTACH, NULL) != TRUE){
            if(Error != NULL) {
                *Error = GsPeEntryPointCallError;
            }
            return NULL;
        }
    }

    PE->ImageBase = ImageBase;

    return ImageBase;
}

VOID GsPeUnload(
    _Inout_ PGS_PE PE
)
{
    if(PE == NULL) {
        return;
    }

    DWORD EntryPointRVA = PE->OptionalHeader.AddressOfEntryPoint;
    
    if(PE->ImageBase != NULL && EntryPointRVA != 0 && PE->FileHeader.Characteristics & IMAGE_FILE_DLL) {
        GsLibraryEntryPoint DllMain = GS_RVA_CAST(PE->ImageBase, GsLibraryEntryPoint, EntryPointRVA);
        DllMain((HINSTANCE) PE->ImageBase, DLL_PROCESS_DETACH, NULL);
    }

    GsArenaRelease(PE->Arena);
}

_Success_(return == GsPeSuccess)
GsPeError GsPepApplyRelocations(
    _In_ PVOID  ImageBase,
    _In_ PGS_PE PE
)
{
    IMAGE_DATA_DIRECTORY RelocationDirectory    = PE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    DWORD_PTR PreferredImageBase                = PE->OptionalHeader.ImageBase;
    DWORD BlockOffset                           = 0;
    DWORD RelocationDelta                       = (DWORD)(((DWORD_PTR) ImageBase) - PreferredImageBase);
    PIMAGE_BASE_RELOCATION BaseRelocation       = GS_RVA_CAST(ImageBase, PIMAGE_BASE_RELOCATION, RelocationDirectory.VirtualAddress);

    while(BlockOffset < RelocationDirectory.Size) {
        SIZE_T EntryCount = (BaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        PWORD Entry = (PWORD)(((DWORD_PTR) BaseRelocation) + ((DWORD_PTR) sizeof(IMAGE_BASE_RELOCATION)));
        for(SIZE_T j = 0; j < EntryCount; j++, Entry++) {
            if((*Entry >> 12) & IMAGE_REL_BASED_HIGHLOW) {
                PDWORD Address = (PDWORD)(((DWORD_PTR) ImageBase) + ((DWORD_PTR) BaseRelocation->VirtualAddress) + ((*Entry) & 0xFFF));
                (*Address) += RelocationDelta;
            }
        }
        BlockOffset     += BaseRelocation->SizeOfBlock;
        BaseRelocation  = (PIMAGE_BASE_RELOCATION)(((PUINT8) BaseRelocation) + BaseRelocation->SizeOfBlock);
    }

    return GsPeSuccess;
}

_Success_(return == GsPeSuccess)
GsPeError GsPepResolveImports(
    _In_ PVOID ImageBase,
    _In_ PGS_PE PE
)
{
    IMAGE_DATA_DIRECTORY ImportDirectory        = PE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_DATA_DIRECTORY BoundImportDirectory   = PE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];

    if(ImportDirectory.Size > 0) {
        PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = GS_RVA_CAST(ImageBase, PIMAGE_IMPORT_DESCRIPTOR, ImportDirectory.VirtualAddress);
        IMAGE_IMPORT_DESCRIPTOR Sentinel;
        ZeroMemory(&Sentinel, sizeof(IMAGE_IMPORT_DESCRIPTOR));

        while(memcmp(ImportDescriptor, &Sentinel, sizeof(IMAGE_IMPORT_DESCRIPTOR)) != 0) {
            LPCSTR LibraryName = (LPCSTR)((PUINT8) ImageBase) + ImportDescriptor->Name;
            PGS_LIBRARY Library = GsLibraryLoad(LibraryName);

            if(Library == NULL) {
                return GsPeImportResolutionError;
            }

            PIMAGE_THUNK_DATA OriginalThunk = GS_RVA_CAST(ImageBase, PIMAGE_THUNK_DATA, ImportDescriptor->OriginalFirstThunk);
            PIMAGE_THUNK_DATA Thunk         = GS_RVA_CAST(ImageBase, PIMAGE_THUNK_DATA, ImportDescriptor->FirstThunk);
            while(OriginalThunk->u1.Ordinal != 0 && GS_RVA_IS_VALID(PE, OriginalThunk->u1.AddressOfData)) {
                if(OriginalThunk->u1.AddressOfData & 0x0000000000000001) {
                    WORD Ordinal = OriginalThunk->u1.Ordinal >> 48;

                    PVOID FunctionAddress = GsLibraryGetFunctionAddressByOrdinal(Library, Ordinal);
                    if(FunctionAddress == NULL) {
                        return GsPeImportResolutionError;
                    }

                    Thunk->u1.Function = (ULONGLONG) FunctionAddress;
                } else {
                    PIMAGE_IMPORT_BY_NAME ImportByName = GS_RVA_CAST(ImageBase, PIMAGE_IMPORT_BY_NAME, OriginalThunk->u1.AddressOfData);
                    printf("Function Name: %s\n", (LPCSTR) &(ImportByName->Name));
                    PVOID FunctionAddress = GsLibraryGetFunctionAddressByName(Library, (LPCSTR) &(ImportByName->Name));
                    if(FunctionAddress == NULL) {
                        return GsPeImportResolutionError;
                    }

                    Thunk->u1.Function = (ULONGLONG) FunctionAddress;
                }

                OriginalThunk++;
                Thunk++;
            }

            ImportDescriptor++;
        }    
    }

    if(BoundImportDirectory.Size > 0) {

    }

    return GsPeSuccess;
}

_Success_(return == GsPeSuccess)
GsPeError GsPepResolveExports(
    _In_ PVOID          ImageBase,
    _In_ PGS_PE         PE,
    _Inout_ PGS_LIST    Exports
)
{
    IMAGE_DATA_DIRECTORY ExportDataDirectory = PE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if(ExportDataDirectory.Size == 0) {
        return GsPeSuccess;
    }

    PIMAGE_EXPORT_DIRECTORY ExportDirectory = GS_RVA_CAST(ImageBase, PIMAGE_EXPORT_DIRECTORY, ExportDataDirectory.VirtualAddress);
    DWORD NumberOfNames                     = ExportDirectory->NumberOfNames;
    PWORD NameOrdinalsTable                 = GS_RVA_CAST(ImageBase, PWORD, ExportDirectory->AddressOfNameOrdinals);
    PDWORD ExportAddressTable               = GS_RVA_CAST(ImageBase, PDWORD, ExportDirectory->AddressOfFunctions);
    PDWORD NamesTable                       = GS_RVA_CAST(ImageBase, PDWORD, ExportDirectory->AddressOfNames);

    for(DWORD i = 0; i < NumberOfNames; i++) {
        GS_PE_EXPORT Export;
        Export.Name = GsStringInit(PE->Arena);
        if(Export.Name == NULL) {
            return GsPeMemoryAllocationError;
        }

        DWORD NameRVA = NamesTable[i];

        PCHAR ExportName        = GS_RVA_CAST(ImageBase, PCHAR, NameRVA);
        WORD Ordinal            = NameOrdinalsTable[i];
        PDWORD FunctionAddress  = GS_RVA_CAST(ImageBase, PDWORD, ExportAddressTable[Ordinal]);

        GsStringConcat(Export.Name, ExportName);
        Export.Address  = FunctionAddress;
        Export.Ordinal  = Ordinal;

        GsListError InsertError = GsListInsert(Exports, &Export);
        if(InsertError != GsListSuccess) {
            return GsPeListInsertionError;
        }
    }

    return GsPeSuccess;
}

_Success_(return != NULL)
PIMAGE_SECTION_HEADER GsPepFindSection(
    _In_ PGS_PE PE,
    _In_ DWORD  RVA
)
{
    for(SIZE_T i = 0; i < PE->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER Header = &(PE->Sections[i].Header);
        if(Header->VirtualAddress <= RVA && (Header->VirtualAddress + Header->Misc.VirtualSize) >= RVA) {
            return Header;
        }
    }

    return NULL;
}