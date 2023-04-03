# Setup of the Nordic nRF52 toolchain for Segger Embedded Studio

For a more detailed Getting Started Guide with other IDE options:  
`https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_gsg_ses%2FUG%2Fgsg%2Fintro.html&cp=1_1_0`  
Go to: - Getting started with nRF5 SDK and SES (nRF51 & nRF52 Series)
			- Setting up your toolchain

### Segger Embedded Studio for ARM (V5.62):  
`https://www.segger.com/downloads/embedded-studio`
- Download and install

### Obtain a free license for SES:  
`https://license.segger.com/Nordic.cgi`
- Fill in the requested information, the licese is sent to your mail
- Open SES
- Click Tools > License Manager...
- Select Activate SEGGER Embedded Studio
- Paste in your license and click on Install License
Note: After click on "install license", it may take a few hours until the license is activated

### Segger J-Link Software and Documentation Pack (V7.56a):  
`https://www.segger.com/downloads/jlink`
- Download and Install

### Nordic nRF Command Line Tools (V10.14.0):  
`https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download`
- Download and install
- Check for path: nrfjprog --version

### Nordic nRF Connect for Desktop (V3.7.1):  
'https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-desktop/Download#infotabs'
- Download and install it
- Open it and install the programmer app (V1.4.11)
- Needed to flash the nRF52 Dongle

### SoftDevice S140 (V7.2.0):  
`https://www.nordicsemi.com/Products/Development-software/S140/Download?lang=en#infotabs`
- Only needed for the nRF52840 Dongle
- For the nRF52832, this is automatically flashed by the IDE together with the compiled code

### Nordic nRF5 SDK (Software Development Kit) (V17.1.0):  
`https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs`
- Download and unpack
- Place the folder close to the root level of your file system (e.g. C:/Nordic/SDK)  
Note: Compilers tend to run into problems with long path names. Therefore, place the folder as close to the root level of your file system as possible (for example, at C:/Nordic/SDK). Also, avoid using spaces in the file path and folder name.
- Copy the folders `ble_peripheral` and `peripheral` to the `examples` folder of the SDK (e.g. copy to C:/Nordic/SDK/examples)
- Inside the copied folders (`ble_peripheral` and `peripheral`), you can find `.emProject` project files for WULPUS and USB-dongle firmware respectively. You can open these projects in Segger Embedded Studio.
