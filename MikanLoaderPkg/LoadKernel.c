#include "LoadKernel.h"

#include <Guid/FileInfo.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <elf.h>

#include "Common.h"

static void CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first_out, UINT64* last_out) {
	Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
	UINT64 first = MAX_UINT64;
	UINT64 last = 0;

	for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
		if (phdr[i].p_type == PT_LOAD) {
			first = MIN(first, phdr[i].p_vaddr);
			last = MAX(last, phdr[i].p_vaddr + phdr[i].p_memsz);
		}
	}

	*first_out = first;
	*last_out = last;
}

static void CopyLoadSegments(Elf64_Ehdr* ehdr) {
	Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);

	for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
		if (phdr[i].p_type == PT_LOAD) {
			UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
			CopyMem((VOID*)phdr[i].p_vaddr, (VOID*)segm_in_file, phdr[i].p_filesz);

			UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
			SetMem((VOID*)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
		}
	}
}

static void ExitBootServices(EFI_HANDLE image_handle, struct MemoryMap* memmap) {
	EFI_STATUS status = GetMemoryMap(memmap);
	if (EFI_ERROR(status)) {
		Print(u"Failed to get memory map: %r\n", status);
		Halt();
	}

	status = gBS->ExitBootServices(image_handle, memmap->map_key);
	if (EFI_ERROR(status)) {
		Print(u"Could not exit boot service: %r\n", status);
		Halt();
	}
}

static VOID* LoadKernelToTempMemory(EFI_FILE_PROTOCOL* file, UINTN size) {
	VOID* buf;
	EFI_STATUS status = gBS->AllocatePool(EfiLoaderData, size, &buf);
	if (EFI_ERROR(status)) {
		Print(u"Failed to allocate pool: %r\n", status);
		Halt();
	}

	status = file->Read(file, &size, buf);
	if (EFI_ERROR(status)) {
		Print(u"Failed to read kernel: %r\n", status);
		Halt();
	}

	return buf;
}

void LoadKernel(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL* root_dir, struct FrameBufferConfig* config) {
	EFI_FILE_PROTOCOL* kernel_file;
	EFI_STATUS status = root_dir->Open(root_dir, &kernel_file, u"\\kernel.elf", EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		Print(u"Failed to open file '\\kernel.elf': %r\n", status);
		Halt();
	}

	// kernel.elfのファイルサイズを計算
	UINTN kernel_filesize;
	{
		UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
		UINT8 file_info_buffer[file_info_size];
		status = kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &file_info_size, file_info_buffer);
		if (EFI_ERROR(status)) {
			Print(u"Failed to get file infomation: %r\n", status);
			Halt();
		}

		EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
		kernel_filesize = file_info->FileSize;
		Print(u"Size of kernel.elf: %lubytes\n", kernel_filesize);
	}

	VOID* kernel_buffer = LoadKernelToTempMemory(kernel_file, kernel_filesize);

	UINT64 kernel_first_addr;
	{
		Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;

		// カーネルを配置するべきアドレスを計算
		UINT64 kernel_last_addr;
		CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
		UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;

		// 計算したアドレスを確保
		status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);
		if (EFI_ERROR(status)) {
			Print(u"Failed to allocate pages: %r\n", status);
			Halt();
		}

		// カーネルを配置
		CopyLoadSegments(kernel_ehdr);
		Print(u"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);
	}

	status = gBS->FreePool(kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(u"Failed to free pool: %r\n", status);
		Halt();
	}

	CHAR8 memmap_buf[4096 * 4];
	struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};

	ExitBootServices(image_handle, &memmap);

	// ELFファイルはエントリポイントアドレスがファイルの先頭から24バイト目から8バイト書かれる
	UINT64 entry_addr = *(UINT64*)(kernel_first_addr + 24);

	typedef void EntryPointType(const struct FrameBufferConfig*, const struct MemoryMap*);
	EntryPointType* entry_point = (EntryPointType*)entry_addr;

	// 開始
	entry_point(config, &memmap);
}
