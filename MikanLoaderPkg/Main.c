#include <Guid/FileInfo.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

#include <elf.h>

#include "Protocol/GraphicsOutput.h"
#include "frame_buffer_config.h"

struct MemoryMap {
	UINTN buffer_size;
	VOID* buffer;
	UINTN map_size;
	UINTN map_key;
	UINTN descriptor_size;
	UINT32 descriptor_version;
};

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
	if (map->buffer == NULL) {
		return EFI_BUFFER_TOO_SMALL;
	}

	map->map_size = map->buffer_size;
	return gBS->GetMemoryMap(
		&map->map_size,
		(EFI_MEMORY_DESCRIPTOR*)map->buffer,
		&map->map_key,
		&map->descriptor_size,
		&map->descriptor_version);
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
	switch (type) {
	case EfiReservedMemoryType:
		return u"EfiReservedMemoryType";
	case EfiLoaderCode:
		return u"EfiLoaderCode";
	case EfiLoaderData:
		return u"EfiLoaderData";
	case EfiBootServicesCode:
		return u"EfiBootServicesCode";
	case EfiBootServicesData:
		return u"EfiBootServicesData";
	case EfiRuntimeServicesCode:
		return u"EfiRuntimeServicesCode";
	case EfiRuntimeServicesData:
		return u"EfiRuntimeServicesData";
	case EfiConventionalMemory:
		return u"EfiConventionalMemory";
	case EfiUnusableMemory:
		return u"EfiUnusableMemory";
	case EfiACPIReclaimMemory:
		return u"EfiACPIReclaimMemory";
	case EfiACPIMemoryNVS:
		return u"EfiACPIMemoryNVS";
	case EfiMemoryMappedIO:
		return u"EfiMemoryMappedIO";
	case EfiMemoryMappedIOPortSpace:
		return u"EfiMemoryMappedIOPortSpace";
	case EfiPalCode:
		return u"EfiPalCode";
	case EfiPersistentMemory:
		return u"EfiPersistentMemory";
	case EfiMaxMemoryType:
		return u"EfiMaxMemoryType";
	default:
		return u"InvalidMemoryType";
	}
}

EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
	CHAR8 buf[256];
	UINTN len;

	CHAR8* header = "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
	len = AsciiStrLen(header);
	EFI_STATUS status = file->Write(file, &len, header);
	if (EFI_ERROR(status)) {
		return status;
	}

	Print(u"map->buffer = %08lx, map->map_size = %08lx\n", map->buffer, map->map_size);

	EFI_PHYSICAL_ADDRESS iter = (EFI_PHYSICAL_ADDRESS)map->buffer;
	int i = 0;
	for (; iter < (EFI_PHYSICAL_ADDRESS)(map->buffer + map->map_size); iter += map->descriptor_size, i++) {
		EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
		len = AsciiSPrint(
			buf,
			sizeof(buf),
			"%u, %x, %-ls, %08lx, %lx, %lx\n",
			i,
			desc->Type,
			GetMemoryTypeUnicode(desc->Type),
			desc->PhysicalStart,
			desc->NumberOfPages,
			desc->Attribute & 0xffffflu);
		status = file->Write(file, &len, &buf);
		if (EFI_ERROR(status)) {
			return status;
		}
	}

	return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
	EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

	EFI_STATUS status = gBS->OpenProtocol(
		image_handle,
		&gEfiLoadedImageProtocolGuid,
		(VOID**)&loaded_image,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status)) {
		return status;
	}

	status = gBS->OpenProtocol(
		loaded_image->DeviceHandle,
		&gEfiSimpleFileSystemProtocolGuid,
		(VOID**)&fs,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status)) {
		return status;
	}

	return fs->OpenVolume(fs, root);
}

static void Halt() {
	while (1) {
		__asm__("hlt");
	}
}

static EFI_STATUS OpenGOP(EFI_HANDLE image_handle, EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
	UINTN num_gop_handles = 0;
	EFI_HANDLE* gop_handles = NULL;
	gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &num_gop_handles, &gop_handles);

	EFI_STATUS status = gBS->OpenProtocol(
		gop_handles[0],
		&gEfiGraphicsOutputProtocolGuid,
		(VOID**)gop,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

	if (EFI_ERROR(status)) {
		return status;
	}

	FreePool(gop_handles);
	return EFI_SUCCESS;
}

static const CHAR16* GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
	switch (fmt) {
	case PixelRedGreenBlueReserved8BitPerColor:
		return u"PixelRedGreenBlueReserved8BitPerColor";
	case PixelBlueGreenRedReserved8BitPerColor:
		return u"PixelBlueGreenRedReserved8BitPerColor";
	case PixelBitMask:
		return u"PixelBitMask";
	case PixelBltOnly:
		return u"PixelBltOnly";
	case PixelFormatMax:
		return u"PixelFormatMax";
	default:
		return u"InvalidPixelFormat";
	}
}

static void DrawPixels(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop) {
	Print(
		u"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
		gop->Mode->Info->PixelsPerScanLine);

	Print(
		u"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
		gop->Mode->FrameBufferBase,
		gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
		gop->Mode->FrameBufferSize);

	// 色の書き込み
	UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
	for (UINTN i = 0; i < gop->Mode->FrameBufferSize; ++i) {
		frame_buffer[i] = 0xff;
	}
}

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

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
	Print(u"Hello, world from Arch Linux!\n");

	CHAR8 memmap_buf[4096 * 4];
	struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
	EFI_STATUS status = GetMemoryMap(&memmap);
	if (EFI_ERROR(status)) {
		Print(u"Failed to get memory map: %r\n", status);
		Halt();
	}

	EFI_FILE_PROTOCOL* root_dir;
	status = OpenRootDir(image_handle, &root_dir);
	if (EFI_ERROR(status)) {
		Print(u"Failed to open root directory: %r\n", status);
		Halt();
	}

	EFI_FILE_PROTOCOL* memmap_file;
	status = root_dir->Open(
		root_dir, &memmap_file, u"\\memmap", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

	if (EFI_ERROR(status)) {
		Print(u"Failed to open file '\\memmap': %r\n", status);
		Print(u"Ignored.\n");
	} else {
		status = SaveMemoryMap(&memmap, memmap_file);
		if (EFI_ERROR(status)) {
			Print(u"Failed to save memory map: %r\n", status);
			Halt();
		}
		status = memmap_file->Close(memmap_file);
		if (EFI_ERROR(status)) {
			Print(u"Failed to close memory map: %r\n", status);
			Halt();
		}
	}

	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	status = OpenGOP(image_handle, &gop);
	if (EFI_ERROR(status)) {
		Print(u"Failed to open GOP: %r\n", status);
		Halt();
	}

	DrawPixels(gop);

	// カーネルに渡すフレームバッファの詳細
	struct FrameBufferConfig config = {
		(UINT8*)gop->Mode->FrameBufferBase,
		gop->Mode->Info->PixelsPerScanLine,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution};

	switch (gop->Mode->Info->PixelFormat) {
	case PixelRedGreenBlueReserved8BitPerColor:
		config.pixel_format = kPixelRGBResv8BitPerColor;
		break;
	case PixelBlueGreenRedReserved8BitPerColor:
		config.pixel_format = kPixelBGRResv8BitPerColor;
		break;
	default:
		Print(u"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
		Halt();
	}

	EFI_FILE_PROTOCOL* kernel_file;
	status = root_dir->Open(root_dir, &kernel_file, u"\\kernel.elf", EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		Print(u"Failed to open file '\\kernel.elf': %r\n", status);
		Halt();
	}

	UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
	UINT8 file_info_buffer[file_info_size];
	status = kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &file_info_size, file_info_buffer);
	if (EFI_ERROR(status)) {
		Print(u"Failed to get file infomation: %r\n", status);
		Halt();
	}

	EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
	UINTN kernel_file_size = file_info->FileSize;

	VOID* kernel_buffer;
	status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(u"Failed to allocate pool: %r\n", status);
		Halt();
	}

	status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(u"Failed to read kernel: %r\n", status);
		Halt();
	}

	Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;
	UINT64 kernel_first_addr, kernel_last_addr;
	CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
	UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;

	status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);
	if (EFI_ERROR(status)) {
		Print(u"Failed to allocate pages: %r\n", status);
		Halt();
	}

	CopyLoadSegments(kernel_ehdr);
	Print(u"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

	status = gBS->FreePool(kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(u"Failed to free pool: %r\n", status);
		Halt();
	}

	// ブートサービスの終了
	status = gBS->ExitBootServices(image_handle, memmap.map_key);
	if (EFI_ERROR(status)) {
		status = GetMemoryMap(&memmap);
		if (EFI_ERROR(status)) {
			Print(u"Failed to get memory map: %r\n", status);
			Halt();
		}

		status = gBS->ExitBootServices(image_handle, memmap.map_key);
		if (EFI_ERROR(status)) {
			Print(u"Could not exit boot service: %r\n", status);
			Halt();
		}
	}

	// カーネルの開始
	// ELFファイルはエントリポイントアドレスがファイルの先頭から24バイト目から8バイト書かれる
	UINT64 entry_addr = *(UINT64*)(kernel_first_addr + 24);

	typedef void EntryPointType(const struct FrameBufferConfig*);
	EntryPointType* entry_point = (EntryPointType*)entry_addr;
	entry_point(&config);

	Print(u"All done\n");
	Halt();
	return EFI_SUCCESS;
}
