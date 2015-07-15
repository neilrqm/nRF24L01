/*
 * main.c
 *
 *  Created on: 27-Aug-2009
 *      Author: nrqm
 */

#include "radio.h"

int main()
{
	return 0;
}


void radio_rxhandler(uint8_t pipe_number)
{
	// This function is called when the radio receives a packet.
	// It is called in the radio's ISR, so it must be kept short.
	// The function may be left empty if the application doesn't need to respond immediately to the interrupt.
}
