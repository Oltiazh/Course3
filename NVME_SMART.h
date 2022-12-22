#pragma once

#include "resource.h"
struct SmartInfo {
    WCHAR AttributeName[60];
    ULONG Value;
    WCHAR AttributeMeasure[12];
};
struct DriveProperties {
    std::wstring Drive;
    std::wstring Name;
    std::wstring Size;
    DWORD BusType;
};
std::wstring BusTypeDecode[25]{
  L"BusTypeUnknown",
  L"Scsi",
  L"Atapi",
  L"Ata",
  L"1394",
  L"Ssa",
  L"Fibre",
  L"Usb",
  L"RAID",
  L"iScsi",
  L"Sas",
  L"Sata",
  L"Sd",
  L"Mmc",
  L"Virtual",
  L"FileBackedVirtual",
  L"Spaces",
  L"Nvme",
  L"SCM",
  L"Ufs",
  L"BusTypeMax",
  L"BusTypeMaxReserved"
};