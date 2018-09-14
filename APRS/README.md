# TJREVERB CubeSat 2018

## Thomas Jefferson High School in collaboration with George Mason University.

This code currently is structured to be a single binary that runs on a Linux machine.  It has functions to read/write to
a serial device.  It will need to broken up later to accommodate running on the flight computer in the CubeSat.
Currently the operating environment is not known.  It could be bare metal, uClinux, or some variant.

The flight control processor is a Microsemi SmartFusion2 SoC with FPGA fabric:
https://www.microsemi.com/products/fpga-soc/soc-fpga/smartfusion2

To build with debug symbols:
    ```$ make debug```

To build release binary:
    ```$ make release```

To execute:
   ```$ ./tjreverb_serial /dev/ttyUSB0```

Make sure you are part of the ```dialout``` group.  Otherwise use ```sudo``` to run the command.

Replace ```/dev/ttyUSB0``` with whatever your serial port is (use ```dmesg``` to find out)
