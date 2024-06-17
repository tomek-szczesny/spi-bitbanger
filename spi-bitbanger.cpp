#include <chrono>
#include <fstream>
#include <gpiod.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>	// usleep

//---------------------------- Configuration constants -------------------------

// Modify the following to match the desired output GPIO
// Configuration below is for certain GPIO on Odroid M1S
const char *chipname_MISO = "gpiochip2";
const char *chipname_MOSI = "gpiochip0";
const char *chipname_SCK = "gpiochip2";
const char *chipname_CS = "gpiochip2";
const int gpio_line_MISO = 10; // M1S: 32
const int gpio_line_MOSI = 13; // M1S: 33
const int gpio_line_SCK = 5; // M1S: 35
const int gpio_line_CS = 6; // M1S: 36

// SPI mode number 0-3
// See: https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Mode_numbers
const char spimode = 0;

// CLK frequency limit
// It's the limit, not the guaranteed speed. 
const float freq = 200000;

//--------------------------------- Misc stuff ---------------------------------

const std::chrono::nanoseconds halfperiod =
	std::chrono::nanoseconds(static_cast<uint32_t>(500000000/freq));

const bool cpol = (spimode==2 || spimode==3);
const bool cpha = (spimode==1 || spimode==3);

bool io_closing = 0;
bool main_closing = 0;

	

//------------------------------------ IO loop --------------------------------

void IO() {
	// This is a process intended to run as a separate thread
	// for the sake of simplicity and performance.
	// The loop is launched for each half of the clock cycle.
	// In order to kill this thread gracefully, set "io_closing" to 1. 
	
	struct gpiod_chip *cMISO;
	struct gpiod_line *MISO;
	cMISO = gpiod_chip_open_by_name(chipname_MISO);
	MISO = gpiod_chip_get_line(cMISO, gpio_line_MISO);
	gpiod_line_request_input(MISO, "Bitbanged MISO"); 

	struct gpiod_chip *cMOSI;
	struct gpiod_line *MOSI;
	cMOSI = gpiod_chip_open_by_name(chipname_MOSI);
	MOSI = gpiod_chip_get_line(cMOSI, gpio_line_MOSI);
	gpiod_line_request_output(MOSI, "Bitbanged MOSI", 1); 

	struct gpiod_chip *cSCK;
	struct gpiod_line *SCK;
	cSCK = gpiod_chip_open_by_name(chipname_SCK);
	SCK = gpiod_chip_get_line(cSCK, gpio_line_SCK);
	gpiod_line_request_output(SCK, "Bitbanged SCK", cpol); 

	struct gpiod_chip *cCS;
	struct gpiod_line *CS;
	cCS = gpiod_chip_open_by_name(chipname_CS);
	CS = gpiod_chip_get_line(cCS, gpio_line_CS);
	gpiod_line_request_output(CS, "Bitbanged CS", 1); 

	char ibyte, obyte, ioctr;
	bool nsck = !cpol;
	int ctr;

	ctr = 7;
	ibyte = 0;
	std::cin >> obyte;

	auto next_clk_edge = std::chrono::high_resolution_clock::now() + halfperiod;
	gpiod_line_set_value(CS, 0);
	if (!cpha) {
		gpiod_line_set_value(MOSI, ((obyte >> ctr) & (0x1)));
	}

	while (!io_closing) {
		std::this_thread::sleep_until(next_clk_edge);
		next_clk_edge += halfperiod;

		gpiod_line_set_value(SCK, nsck); nsck = !nsck;

		if (cpol ^ cpha ^ nsck) {	// WRITE
			gpiod_line_set_value(MOSI, ((obyte >> ctr) & (0x1)));
			}
		else {				// READ
			ibyte += gpiod_line_get_value(MISO) << ctr;
			ctr -=1;
			if (ctr < 0) {
				std::cout << ibyte; ibyte = 0;
				obyte = std::cin.get();
				if (!std::cin || !std::cout) break;
				ctr = 7;
			}
		}

	}

	main_closing = 1;

	gpiod_line_set_value(CS, 1);


	gpiod_line_release(MISO);
	gpiod_chip_close(cMISO);
	gpiod_line_release(MOSI);
	gpiod_chip_close(cMOSI);
	gpiod_line_release(SCK);
	gpiod_chip_close(cSCK);
	gpiod_line_release(CS);
	gpiod_chip_close(cCS);
}

//--------------------------------- Signal Handle ------------------------------

void signal_handle(const int s) {
	// Handles a few POSIX signals, asking the process to die gracefully
	
	main_closing = 1;

	if (s){};	// Suppress warning about unused, mandatory parameter
}

//----------------------------------- Main Loop --------------------------------

int main(int argc, char **argv)
{

	signal (SIGINT, signal_handle);		// Catches SIGINT (ctrl+c)
	signal (SIGTERM, signal_handle);	// Catches SIGTERM

	// Create IO thread
	std::thread io_thread (IO);

	// Assign Real Time priority to IO thread
	sched_param sch;
	int policy;
	pthread_getschedparam(io_thread.native_handle(), &policy, &sch);
	sch.sched_priority = 99;
	pthread_setschedparam(io_thread.native_handle(), SCHED_FIFO, &sch);


	while (!main_closing) {
		usleep(100000);
	}

	io_closing = 1;
	io_thread.join();
	exit(0);
}
