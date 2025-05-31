# KMDFHelloWorld

This is my first time doing any sort of Windows OS programming, kernel debugging, or driver development. I used KMDF C libraries from the WDK API, Visual Studio, WinDbg, and two laptops (host, target). 

This code is heavily adapted from [this](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver) Microsoft Tutorial and [this](https://github.com/AshleyT3/tutorial-sample-code/blob/main/src/Intro-Windows-Driver-Development-and-Kernel-Debugging-Tutorial/kmdrvhw-3-multi-bug-driver/driver.c) YouTube Tutorial from RicochetTech.

This driver uses kernel-mode printing features to print “Hello, World!”; there is no physical component. 

The first driver bug is a simple use-after-free bug which does not cause any crashes and is not detected by WinDbg, since the memory accesses are still physically valid, albeit logically invalid. The error can be detected using the Windows Driver Verifier. 

![useafterfree](https://github.com/user-attachments/assets/49998b91-52e0-47f9-93d3-3d9807f84e90)

The second driver bug is an invalid memory access, which triggers Bug Check 0x50. A pointer keeps incrementing, moving through memory addresses until it hits an invalid non-paged virtual address. It was quite fun to press g and cause the target to blue-screen even after WinDbg caused a break. 

![bugcheck](https://github.com/user-attachments/assets/129f2291-db62-4f27-8eee-2de5fecd18c9)

## Steps for deploying driver:
Open workspace, attach kernel debugger, deploy driver.

### Case 1: Use-After-Free Driver
Set breakpoints with the following commands:
```
.reload /f KMDFHelloWorld.sys
lm m KMDFHelloWorld*
x KMDFHelloWorld!*

sxe ld KMDFHelloWorld.sys
bm KMDFHelloWorld!DriverEntry
bm KMDFHelloWorld!KMDFHelloWorldEvtDeviceAdd 
bl
```
My printing was not initially working, so I used the following command to enable debug printing:
```
ed nt!Kd_IHVDRIVER_Mask 0xFFFFFFFF 
```
Sometimes, I had to restart my target PC so the symbols could be found or so the breakpoints could be activated. 

### Case 2: Blue-Screen Driver
We could set breakpoints, but this is not needed; WinDbg will break after deployment as Bug Check 0x50 has been called. To learn more about the bug check, use:
```
!analyze -v
```
Continue anyway to cause a visible blue screen with the error code, then reboot the PC: 
```
.reboot
```
