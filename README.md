# spi-bitbanger
An universal SPI GPIO bitbanger for any single board computer.
This is a debug tool, mostly.

This program acts as SPI master, thus it controls the transmission and generates SCK.

SPI bus always writes and reads data simultaneously on each clock cycle.
This program writes data from stdin to MOSI, and captures MISO data to stdout. The process continues until either of the pipes becomes invalid.

Most Slave chips expect to receive commands from the Master and then return a response while ignoring additional MOSI data. In this scenario, you'd want to pad the input data with a few dummy bytes to capture the response.

I achieved 200kbps on Odroid M1S (RK3566), albeit with clearly visible duty cycle skew and period jitter. Low speed is due to the libgpiod overhead.

## Installation

First, install the prerequisites:
- g++
- make
- git
- libgpiod-dev, gpiod

Clone this repository:

`git clone https://github.com/tomek-szczesny/spi-bitbanger.git`

Inspect the first 30 lines of `spi-bitbanger.cpp`. Here, you must configure which GPIOs you wish to use.

You can use `sudo gpioinfo` to find the correct gpiochip and line numbers.

Next:
```
make build
```

## Usage

Pipe data into stdin and from stdout to interact with a SPI device.


Here is a sample loopback test (MISO and MOSI shorted together):

```
$ dd if=/dev/urandom bs=1024 count=256 | tee out.bin | sudo ./spi-bitbanger > in.bin; diff -s in.bin out.bin
256+0 records in
256+0 records out
262144 bytes (262 kB, 256 KiB) copied, 7.24745 s, 36.2 kB/s
Files in.bin and out.bin are identical
$  
```
