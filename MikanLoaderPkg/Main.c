#include <Library/UefiLib.h>
#include <Uefi.h>

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
	Print(u"Hello, world from Arch Linux!\n");
	while (1) {
	}
	return EFI_SUCCESS;
}
