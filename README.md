# gb_emulator

A simple Game Boy emulator written in C++ without sound support. Currently, this emulator passes the following tests:
* Blargg's cpu_instr test
* Blargg's instr_timing test

It can run the following games:
* Tetris
* Dr. Mario


# Installation
This emulator requires SDL 2.0:
```
sudo apt-get install libsdl2-dev
```

Clone the repository and build it with cmake and make:
```
git clone https://github.com/MFRIbrahim/gb_emulator.git
cd gb_emulator/
mkdir build
cd build/
cmake ..
make
```
To start the emulator use the path to the ROM file you want to run as a command line argument:
```
./gb_emulator <path>
```
