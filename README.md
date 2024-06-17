# spi-bitbanger
An universal SPI GPIO bitbanger for any single board computer.
This is a debug tool, mostly.

This simple program acts as a SPI Master, that sends bytes from stdin and at the same time writes response to stdout, even if the slave does not send any meaningful data.
After stdin is fully sent, the program terminates. If you want to read the following response, add some dummy bytes.

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

Pipe data into stdin and from stdout to interact with an SPI device.

SPI bus always writes and reads data at the same time, only parts of it get ignored by one of the sides. This program does not attempt to make any sense of the data. It writes bytes and reads MISO at the same time. The program terminates when stdin pipe becomes invalid.

Here is a sample loopback test (MISO and MOSI shorted together):

```
$ dd if=/dev/urandom bs=1024 count=256 | tee out.bin | sudo ./spi-bitbanger > in.bin; diff -s in.bin out.bin
256+0 records in
256+0 records out
262144 bytes (262 kB, 256 KiB) copied, 7.24745 s, 36.2 kB/s
Files in.bin and out.bin are identical
$  
```
