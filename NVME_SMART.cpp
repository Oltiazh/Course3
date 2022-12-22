#include "framework.h"
#include "NVME_SMART.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND twnd[3];
HWND sstWnd[10];
RECT rc;
LVITEMW lvi;
LVCOLUMNW lvc;
WCHAR str[20];
std::wstringstream::pos_type pos[4];
std::wstringstream wsDrives;
std::wstringstream wsDriveName;
std::wstringstream wsDriveSize;
std::wstringstream wsDriveBusType;
DriveProperties rgDriveProperties[32];
SmartInfo rgSmartInfo[] =
{
    {L"Critical Warning (AvailableSpaceLow)", 0, L"0 or 1"},
    {L"Critical Warning (TemperatureThreshold)", 0, L"0 or 1"},
    {L"Critical Warning (ReliabilityDegraded)", 0, L"0 or 1"},
    {L"Critical Warning (ReadOnly)", 0, L"0 or 1"},
    {L"Critical Warning (VolatileMemoryBackupDeviceFailed)", 0, L"0 or 1"},
    {L"Temperature", 0, L"Celsius"},
    {L"Available Spare", 0, L"%"},
    {L"Available Spare Threshold", 0, L"%"},
    {L"Percentage Used", 0, L"%"},
    {L"Data Units Read", 0, L"Gib"},
    {L"Data Units Written", 0, L"Gib"},
    {L"Host Read Commands", 0, L"Count"},
    {L"Host Write Commands", 0, L"Count"},
    {L"Controller Busy Time", 0, L"Minutes"},
    {L"Power Cycles", 0, L"Count"},
    {L"Power On Hours", 0, L"Hours"},
    {L"Unsafe Shutdowns", 0, L"Count"},
    {L"Media and Data Integrity Errors", 0, L"Count"},
    {L"Number of Error Information Log Entries", 0, L"Count"}
};

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void NumOfPhysicalDrives();
LPWSTR ConvertToLPWSTR(const std::wstring& s);
int hexadecimalToDecimal(std::wstring hexVal);
int getGbSmartData(UCHAR Smart[16]);
std::wstring ConvertBSTRToWSTR(BSTR bs);
DWORD GetDriveBusType(std::wstring nDriveNumber, int arrIndex);
double RoundVal(double value, double precision);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NVMESMART, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NVMESMART));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    HBRUSH brsh = CreateSolidBrush(RGB(50, 150, 100));

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = brsh;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NVMESMART);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = NULL;

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU,
      0, 0, 980, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::wstring token;
    std::wstring DriveProperty;
    LPCWSTR AppName = L"Attribute Name";
    std::wstring tempstr(AppName);
    LPCWSTR Value = L"Value";
    std::wstring tempstr2(Value);
    LPCWSTR AttMeas = L"Attribute Measure";
    std::wstring tempstr3(AttMeas);
    const std::wstring filler = L"Compiler dont judge pls";
    LPWSTR lstr = ConvertToLPWSTR(filler);
    switch (message)
    {
    case WM_CREATE:
        NumOfPhysicalDrives();
        GetClientRect(hWnd, &rc);
        twnd[0] = CreateWindowW(WC_LISTVIEWW, L"", LVS_REPORT | WS_TABSTOP | WS_BORDER | WS_CHILD | WS_VISIBLE, 0, 100, 800, 330, hWnd, (HMENU)4, hInst, NULL);
        twnd[1] = CreateWindowW(L"button", L"Show", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON, 840, 400, 80, 30, hWnd, (HMENU)5, hInst, NULL);
        twnd[2] = CreateWindowW(WC_LISTBOXW, L"", LBS_NOTIFY | WS_BORDER | WS_VSCROLL | LBS_HASSTRINGS | WS_VISIBLE | WS_CHILD | WS_HSCROLL, 810, 40, 150, 350, hWnd, (HMENU)6, hInst, NULL);
        sstWnd[0] = CreateWindowW(L"static", L"Choose Drive", WS_CHILD | WS_VISIBLE | SS_CENTER, 820, 10, 120, 20, hWnd, NULL, hInst, NULL);
        sstWnd[1] = CreateWindowW(L"static", L"Drive Name", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 10, 240, 20, hWnd, NULL, hInst, NULL);
        sstWnd[3] = CreateWindowW(L"static", L"Size in GiB", WS_CHILD | WS_VISIBLE | SS_CENTER, 240, 10, 180, 20, hWnd, NULL, hInst, NULL);
        sstWnd[4] = CreateWindowW(L"static", L"Interface Type", WS_CHILD | WS_VISIBLE | SS_CENTER, 420, 10, 180, 20, hWnd, NULL, hInst, NULL);
        sstWnd[5] = CreateWindowW(L"static", L"Program messages", WS_CHILD | WS_VISIBLE | SS_CENTER, 600, 10, 200, 20, hWnd, NULL, hInst, NULL);
        sstWnd[6] = CreateWindowW(L"static", L"0", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 30, 240, 50, hWnd, NULL, hInst, NULL);
        sstWnd[7] = CreateWindowW(L"static", L"0", WS_CHILD | WS_VISIBLE | SS_CENTER, 240, 30, 180, 50, hWnd, NULL, hInst, NULL);
        sstWnd[8] = CreateWindowW(L"static", L"0", WS_CHILD | WS_VISIBLE | SS_CENTER, 420, 30, 180, 50, hWnd, NULL, hInst, NULL);
        sstWnd[9] = CreateWindowW(L"static", L"---", WS_CHILD | WS_VISIBLE | SS_CENTER, 600, 30, 200, 50, hWnd, NULL, hInst, NULL);
        //Creating template for SMART info
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = (rc.right - rc.left) / 3.5;
        
        lvc.iSubItem = 0;
        lvc.pszText = &tempstr[0];
        ListView_InsertColumn(twnd[0], 0, &lvc);

        lvc.iSubItem = 1;
        lvc.pszText = &tempstr2[0];
        ListView_InsertColumn(twnd[0], 1, &lvc);

        lvc.iSubItem = 2;
        lvc.pszText = &tempstr3[0];
        ListView_InsertColumn(twnd[0], 2, &lvc);

        lvi.mask = LVIF_TEXT;
        for (int i = 0; i < 18; i++)
        {
            lvi.iItem = i; lvi.iSubItem = 0;
            lvi.pszText = rgSmartInfo[i].AttributeName;
            ListView_InsertItem(twnd[0], &lvi);

            lvi.iItem = i; lvi.iSubItem = 1;
            wsprintf(str, L"%lu", rgSmartInfo[i].Value);
            lvi.pszText = str;
            ListView_SetItem(twnd[0], &lvi);

            lvi.iItem = i; lvi.iSubItem = 2;
            lvi.pszText = rgSmartInfo[i].AttributeMeasure;
            ListView_SetItem(twnd[0], &lvi);
        }
        //Adding physical drive disk names to listbox
        for (int i = 0; rgDriveProperties[i].Drive != L""; i++) {
            lstr = ConvertToLPWSTR(rgDriveProperties[i].Drive);
            int pos = (int)SendMessageW(twnd[2], LB_ADDSTRING, 0, (LPARAM)lstr);
            SendMessageW(twnd[2], LB_SETITEMDATA, pos, (LPARAM)i);
            GetDriveBusType(rgDriveProperties[i].Drive, i); //Getting bus type of every drive
        }
        delete[] lstr;

        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 5: 
            {
                delete[] lstr;
                //Getting Choosen Drive to show
                int numOfDrive;
                int lbItem = (int)SendMessageW(twnd[2], LB_GETCURSEL, 0, 0);
                if (lbItem == -1) {
                    SetWindowTextW(sstWnd[9], L"Select Drive to show");
                    return FALSE;
                }
                int textLen = (int)SendMessage(twnd[2], LB_GETTEXTLEN, (WPARAM)lbItem, 0);
                WCHAR* textBuffer = new WCHAR[textLen + 1];
                SendMessageW(twnd[2], LB_GETTEXT, (WPARAM)lbItem, (LPARAM)textBuffer);

                //Getting index in array of Drives
                for (int i = 0; i < 32; i++) {
                    if (textBuffer == rgDriveProperties[i].Drive) {
                        numOfDrive = i;
                        break;
                    }
                }
                //Getting Drive Properties
                SetWindowTextW(sstWnd[6], rgDriveProperties[numOfDrive].Name.c_str());
                SetWindowTextW(sstWnd[7], rgDriveProperties[numOfDrive].Size.c_str());
                SetWindowTextW(sstWnd[8], BusTypeDecode[rgDriveProperties[numOfDrive].BusType].c_str());


                HANDLE hDevice = INVALID_HANDLE_VALUE;  // handle to the drive to be examined 
                hDevice = CreateFileW(textBuffer,          // drive to open
                    0,                // no access to the drive
                    FILE_SHARE_READ | // share mode
                    FILE_SHARE_WRITE,
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    FILE_FLAG_OVERLAPPED,                // file attributes
                    NULL);            // do not copy file attributes

                if (hDevice == INVALID_HANDLE_VALUE)    // cannot open the drive
                {
                    SetWindowTextW(sstWnd[9], L"Trying to open drive resulted in error");
                    for (int i = 0; i < 19; i++) {
                        rgSmartInfo[i].Value = 0;
                    }
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    return (FALSE);
                }

                BOOL    result;
                PVOID   buffer = NULL;
                ULONG   bufferLength = 0;
                ULONG   returnedLength = 0;

                PSTORAGE_PROPERTY_QUERY query = NULL;
                PSTORAGE_PROTOCOL_SPECIFIC_DATA protocolData = NULL;
                PSTORAGE_PROTOCOL_DATA_DESCRIPTOR protocolDataDescr = NULL;

                //
                // Allocate buffer for use.
                //
                bufferLength = FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
                buffer = malloc(bufferLength);

                if (buffer == NULL) {
                    for (int i = 0; i < 19; i++) {
                        rgSmartInfo[i].Value = 0;
                    }
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    SetWindowTextW(sstWnd[9], L"Allocate buffer failed");
                    return FALSE;
                }

                //
                // Initialize query data structure to get Identify Controller Data.
                //
                ZeroMemory(buffer, bufferLength);

                query = (PSTORAGE_PROPERTY_QUERY)buffer;
                protocolDataDescr = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)buffer;
                protocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)query->AdditionalParameters;

                query->PropertyId = StorageDeviceProtocolSpecificProperty;
                query->QueryType = PropertyStandardQuery;

                protocolData->ProtocolType = ProtocolTypeNvme;
                protocolData->DataType = NVMeDataTypeLogPage;
                protocolData->ProtocolDataRequestValue = NVME_LOG_PAGE_HEALTH_INFO;
                protocolData->ProtocolDataRequestSubValue = 0;  // This will be passed as the lower 32 bit of log page offset if controller supports extended data for the Get Log Page.
                protocolData->ProtocolDataRequestSubValue2 = 0; // This will be passed as the higher 32 bit of log page offset if controller supports extended data for the Get Log Page.
                protocolData->ProtocolDataRequestSubValue3 = 0; // This will be passed as Log Specific Identifier in CDW11.

                protocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
                protocolData->ProtocolDataLength = sizeof(NVME_HEALTH_INFO_LOG);

                //  
                // Send request down.  
                //  
                result = DeviceIoControl(hDevice,
                    IOCTL_STORAGE_QUERY_PROPERTY,
                    buffer,
                    bufferLength,
                    buffer,
                    bufferLength,
                    &returnedLength,
                    NULL
                );

                if (!result || (returnedLength == 0)) {
                    for (int i = 0; i < 19; i++) {
                        rgSmartInfo[i].Value = 0;
                    }
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    SetWindowTextW(sstWnd[9], L"SMART/Health Information Log failed");
                    return FALSE;
                }

                //
                // Validate the returned data.
                //
                if ((protocolDataDescr->Version != sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR)) ||
                    (protocolDataDescr->Size != sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR))) {
                    for (int i = 0; i < 19; i++) {
                        rgSmartInfo[i].Value = 0;
                    }
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    SetWindowTextW(sstWnd[9], L"SMART/Health - data descriptor header not valid");
                    return FALSE;
                }

                protocolData = &protocolDataDescr->ProtocolSpecificData;

                if ((protocolData->ProtocolDataOffset < sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)) ||
                    (protocolData->ProtocolDataLength < sizeof(NVME_HEALTH_INFO_LOG))) {
                    for (int i = 0; i < 19; i++) {
                        rgSmartInfo[i].Value = 0;
                    }
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    SetWindowTextW(sstWnd[9], L"SMART/Health - ProtocolData Offset/Length not valid");
                    return FALSE;
                }

                //
                // SMART/Health Information Log Data 
                //
                {
                    PNVME_HEALTH_INFO_LOG smartInfo = (PNVME_HEALTH_INFO_LOG)((PCHAR)protocolData + protocolData->ProtocolDataOffset);

                    // print the S.M.A.R.T. data which you need
                    //Some decimal(1byte) to hexadecimal to decimal* 512000
                    double SmartReadWritten[4];
                    SmartReadWritten[0] = getGbSmartData(smartInfo->DataUnitRead) * (double)512000 / pow(1024, 3);
                    SmartReadWritten[1] = getGbSmartData(smartInfo->DataUnitWritten) * (double)512000 / pow(1024, 3);
                    SmartReadWritten[2] = getGbSmartData(smartInfo->HostReadCommands);
                    SmartReadWritten[3] = getGbSmartData(smartInfo->HostWrittenCommands);
                    ULONG temp = (ULONG)smartInfo->Temperature[1] << 8 | smartInfo->Temperature[0];
                    temp -= 273;
                    rgSmartInfo[0].Value = (ULONG)smartInfo->CriticalWarning.AvailableSpaceLow;
                    rgSmartInfo[1].Value = (ULONG)smartInfo->CriticalWarning.TemperatureThreshold;
                    rgSmartInfo[2].Value = (ULONG)smartInfo->CriticalWarning.ReliabilityDegraded;
                    rgSmartInfo[3].Value = (ULONG)smartInfo->CriticalWarning.ReadOnly;
                    rgSmartInfo[4].Value = (ULONG)smartInfo->CriticalWarning.VolatileMemoryBackupDeviceFailed;
                    rgSmartInfo[5].Value = temp;
                    rgSmartInfo[6].Value = (ULONG)smartInfo->AvailableSpare;
                    rgSmartInfo[7].Value = (ULONG)smartInfo->AvailableSpareThreshold;
                    rgSmartInfo[8].Value = (ULONG)smartInfo->PercentageUsed;
                    rgSmartInfo[9].Value = SmartReadWritten[0];
                    rgSmartInfo[10].Value = SmartReadWritten[1];
                    rgSmartInfo[11].Value = SmartReadWritten[2];
                    rgSmartInfo[12].Value = SmartReadWritten[3];
                    rgSmartInfo[13].Value = ((ULONG)smartInfo->ControllerBusyTime[1] << 8 | smartInfo->ControllerBusyTime[0]);
                    rgSmartInfo[14].Value = ((ULONG)smartInfo->PowerCycle[1] << 8 | smartInfo->PowerCycle[0]);
                    rgSmartInfo[15].Value = ((ULONG)smartInfo->PowerOnHours[1] << 8 | smartInfo->PowerOnHours[0]);
                    rgSmartInfo[16].Value = ((ULONG)smartInfo->UnsafeShutdowns[1] << 8 | smartInfo->UnsafeShutdowns[0]);
                    rgSmartInfo[17].Value = ((ULONG)smartInfo->MediaErrors[1] << 8 | smartInfo->MediaErrors[0]);
                    rgSmartInfo[18].Value = ((ULONG)smartInfo->ErrorInfoLogEntryCount[1] << 8 | smartInfo->ErrorInfoLogEntryCount[0]);
                    for (int i = 0; i < 19; i++) {
                        lvi.iItem = i; lvi.iSubItem = 1;
                        wsprintf(str, L"%lu", rgSmartInfo[i].Value);
                        lvi.pszText = str;
                        ListView_SetItem(twnd[0], &lvi);
                    }
                    SetWindowTextW(sstWnd[9], L"SMART data processed successfully");
                }
            }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void NumOfPhysicalDrives() {
    HRESULT hr = 0;
    IWbemLocator* locator = NULL;
    IWbemServices* services = NULL;
    IEnumWbemClassObject* results = NULL;

    BSTR resource = SysAllocString(L"ROOT\\CIMV2");
    BSTR language = SysAllocString(L"WQL");
    BSTR query = SysAllocString(L"SELECT * FROM Win32_DiskDrive");

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&locator);
    hr = locator->ConnectServer(resource, NULL, NULL, NULL, 0, NULL, NULL, &services);

    hr = services->ExecQuery(language, query, WBEM_FLAG_BIDIRECTIONAL, NULL, &results);

    if (results != NULL) {
        IWbemClassObject* result = NULL;
        ULONG returnedCount = 0;

        for (int i = 0; (hr = results->Next(WBEM_INFINITE, 1, &result, &returnedCount)) == S_OK; i++) {
            VARIANT name;
            VARIANT deviceID;
            VARIANT size;
            std::wstring namew;
            std::wstring deviceIDw;
            std::wstring sizew;
            double sizeD;
            hr = result->Get(L"Caption", 0, &name, 0, 0);
            hr = result->Get(L"DeviceID", 0, &deviceID, 0, 0);
            hr = result->Get(L"Size", 0, &size, 0, 0);
            namew = ConvertBSTRToWSTR(name.bstrVal);
            deviceIDw = ConvertBSTRToWSTR(deviceID.bstrVal);
            sizew = ConvertBSTRToWSTR((BSTR)size.uintVal);
            sizeD = std::stod(sizew) / pow(1024, 3);
            sizew = std::to_wstring(RoundVal(sizeD, 0.01));
            rgDriveProperties[i].Size = sizew;
            rgDriveProperties[i].Name = namew;
            rgDriveProperties[i].Drive = deviceIDw;
            result->Release();
        }
    }

    results->Release();
    services->Release();
    locator->Release();
    CoUninitialize();

    SysFreeString(query);
    SysFreeString(language);
    SysFreeString(resource);
}
LPWSTR ConvertToLPWSTR(const std::wstring& s) {
    LPWSTR ws = new wchar_t[s.size() + 1];
    copy(s.begin(), s.end(), ws);
    ws[s.size()] = 0;
    return ws;
}
int hexadecimalToDecimal(std::wstring hexVal) {
    const std::wstring hexNums = L"0123456789abcdef";
    int len = hexVal.size();
    long int base = 1;
    int dec_val = 0;
    for (int i = len - 1; i >= 0; i--) {
        int temp = hexNums.rfind(hexVal[i]);
        dec_val += temp * base;
        base *= 16;
    }
    return dec_val;
}
int getGbSmartData(UCHAR Smart[16]) {
    std::wstringstream Attributes;
    Attributes.str(L"");
    for (int i = 7; i > -1; i--) {
        if (Smart[i] != 0) {
            Attributes << std::hex << Smart[i];
        }
    }
    return hexadecimalToDecimal(Attributes.str());
}
std::wstring ConvertBSTRToWSTR(BSTR bs) {
    assert(bs != nullptr);
    std::wstring ws(bs, SysStringLen(bs));
    return ws;
}
DWORD GetDriveBusType(std::wstring nDriveNumber, int arrIndex) {
    DWORD dwRet = NO_ERROR;
    HANDLE hDevice = CreateFileW(nDriveNumber.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == hDevice) {
        return GetLastError();
    }

    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
    storagePropertyQuery.PropertyId = StorageDeviceProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;

    STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
    DWORD dwBytesReturned = 0;
    if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
        &dwBytesReturned, NULL))
    {
        dwRet = GetLastError();
        CloseHandle(hDevice);
        return dwRet;
    }
    const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
    BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
    ZeroMemory(pOutBuffer, dwOutBufferSize);

    if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        pOutBuffer, dwOutBufferSize,
        &dwBytesReturned, NULL))
    {
        dwRet = GetLastError();
        delete[]pOutBuffer;
        CloseHandle(hDevice);
        return dwRet;
    }

    STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
    const DWORD dwBusType = pDeviceDescriptor->BusType;
    rgDriveProperties[arrIndex].BusType = dwBusType;
    delete[]pOutBuffer;
    CloseHandle(hDevice);
    return dwRet;
}
double RoundVal(double value, double precision)
{
    return std::round(value / precision) * precision;
}