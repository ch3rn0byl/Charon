# Charon  

[![Charon](https://github.com/ch3rn0byl/Charon/actions/workflows/charon.yml/badge.svg)](https://github.com/ch3rn0byl/Charon/actions/workflows/charon.yml)

## Overview  
Charon is a usermode application that will extract the Unified Extensible Firmware Interface, or UEFI, image for x86-64 architecture on Intel-based systems. This tool would be used in conjunction with UEFITool to further the research and development of a vendor's UEFI image. This application only supports Microsoft Windows at the moment. It is important to note Charon uses the software approach of obtaining the UEFI image; therefore, if the image is suspected to be compromised, it is not safe to assume the integrity of this data as it is possible for modified firmware to taint data being transferred to the application. 

## Usage  
```
Options:
  -h, --help            Shows this help message and exits
  -d, --dump-all        Dumps the SPI flash memory in its entirety
  -o, --output          Outputs to a file specified. Default is image.bin
```

![](https://github.com/ch3rn0byl/Charon/blob/master/Images/screenie.png)

## How does it work?
The UEFI image lies in SPI flash memory and is readable by interacting with the SPI controller on the target system. The SPI Base Address, or SPIBAR, could be found at bus 0 device 31 function 5 offset 16. The physical address will then get mapped into the callee's userspace and begin to interact with the SPI controller via SPI memory mapped registers.  

If the `--dump-all` option is selected, the base addresses for all the modules within the SPI flash memory will be determined by reading the flash region registers and then start reading at offset 0. Some of these modules do have access controls so if an attempt to read these protected modules occurs, the SPI controller will set FCERR to 1 and return -1. 

This entire process could be viewed in realtime by monitoring the SPI controller:
![](https://github.com/ch3rn0byl/Charon/blob/master/Images/ExtractingImage.gif)

The limitation for reading the UEFI image is it could only read 64 bytes at a time so it will take a few minutes to extract the UEFI image from the Flash Data registers.

Interested in the more gritty details? Rootkits and Bootkits goes more in depth on what is needed to do to the SPI controller to extract the UEFI image in chapter 19: BIOS/UEFI Forensics: FIrmware Acquisition and Analysis Approaches.  

## Build  
If you are feeling adventurous, making the necessary changes and compiling within the solution would be ideal; otherwise, the CI builds Charon on Windows using the latest Visual Studio 2022 environment. The compiled Windows binary for Charon could be found either in the CI artifacts or in the Release sections.

Shoutout to [@tandasat](https://github.com/tandasat/) and [@matrosov](https://github.com/matrosov) for some of the questions I had -- thank you! 
