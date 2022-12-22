// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <nvme.h>
#include <winioctl.h>
#include <sstream>
#include <CommCtrl.h>
#include <Setupapi.h>
#include <Ntddstor.h>
#include <assert.h>
#include <atlstr.h>
#include <wbemidl.h>
#pragma comment( lib, "setupapi.lib")
#pragma comment(lib, "wbemuuid.lib")
