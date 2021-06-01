#pragma once

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>

#include "frame_buffer_config.h"

void LoadKernel(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL* root_dir, struct FrameBufferConfig* config);
