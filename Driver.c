#include <ntddk.h>
#include <wdf.h>

#define MY_TAG '1gaT' // Tag1 in little-endian
#define CAUSE_USE_AFTER_FREE
// #define CAUSE_BSOD

// Initializes driver and creates driver object
DRIVER_INITIALIZE DriverEntry;

// Initializes devices and creates device objects
EVT_WDF_DRIVER_DEVICE_ADD KMDFHelloWorldEvtDeviceAdd;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

    // Allocate driver configuration object
    WDF_DRIVER_CONFIG config;

    // Print 
    // Note: this routine has no effect in release config
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello, World! DriverEntry has been called.\n"));

    // Initialize driver configuration object to register
    // entry point for EvtDeviceAdd callback, KMDFHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(&config,
        KMDFHelloWorldEvtDeviceAdd
    );

    // Create driver object
    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        WDF_NO_HANDLE
    );

    return status;
}

NTSTATUS
KMDFHelloWorldEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

    // Mark driver object as unreferenced
    UNREFERENCED_PARAMETER(Driver);

    // Allocate device object
    WDFDEVICE hDevice;

    // Print 
    // Note: this routine has no effect in release config
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Hello, World! KMDFHelloWorldEvtDeviceAdd has been called.\n"));

    // Create device object
    status = WdfDeviceCreate(&DeviceInit,
        WDF_NO_OBJECT_ATTRIBUTES,
        &hDevice
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }  

    #ifdef CAUSE_USE_AFTER_FREE
        // Allocate 1024 bytes from non-paged memory pool with tag 
        char* pSilent = (char*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 1024, MY_TAG);
        if (!pSilent) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        // Look in memory with dv, db [Pointer Address] to see value 
        *pSilent = 'A';
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KMDFHelloWorld: Valid assignment complete.\n"));

        // Free
        ExFreePool(pSilent);

        // Use-after-free. Not detected unless Windows Driver Verifier is used, as this is still "physically valid"
        // Look in memory with dv, db [Pointer Address] to see updated value. Crazy stuff
        // I've been told if release deadlines are near, bugs that can only be found with WDV aren't as bad so code can still be released
        *pSilent = 'B';
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KMDFHelloWorld: Invalid assignment complete.\n"));
    #endif

    #ifdef CAUSE_BSOD
        char* pBad = (char*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 1024, MY_TAG);
        if (!pBad) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        *pBad = 'A';
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KMDFHelloWorld: Valid assignment complete.\n"));

        // Infinite loop will find an invalid non-paged address due to missing PTE 
        // Which then leads to a Bug Check 0x50
        // No need to free pBad because this program will crash first
        // Impossible for a single byte to equal 256 so this is an infinite loop
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KMDFHelloWorld: Looking for an invalid non-paged address.\n"));
        while (*pBad++ != 256);
    #endif

    return status;
}