# Setup of the TI MSP430 toolchain

### TI Code Composer Studio (CCS) (V11.0.0.00012):
`https://www.ti.com/tool/CCSTUDIO#downloads`
- Download the single file installer, extract it and run the ccs_setup_11.0.0.00012 file
- Choose custom installation
- In "Select Components", select MSP430 ultra-low power MCUs and finish the installation

# Flash the firmware to the MSP430

### To flash the MSP430 firmware:
Open the project in Code Composer Studio (CCS)
- Open CCS, set the workspace to default value
- Select File > Open Projects from File System...
- Select Directory, set it to `/fw/msp430/wulpus_msp430_firmware` and then click Finish
- Select Project > Build Configurations > Set Active > Debug
- Build the project
- Connect the MSP FET programmer to the US probe using the 8-pin Molex connector according to the diagram in the User Guide.
- Power the US probe with a battery or USB (Set the jumper P2 accordingly)
- Flash the code: Click on the Flash icon (if necessary select a binary file called `wulpus_msp430_firmware.out`)