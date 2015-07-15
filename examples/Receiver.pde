/*
 * Receiver.pde
 *
 * Example of using NRF24L01 driver by Neil MacMillan in an Arduino plataform.
 *
 * Based on the examples by Neil MacMillan in http://nrqm.ca/nrf24l01/examples/
 *
 * Adapted by Miguel Moreto
 * Brazil, 2011.
 *
 * This example was tested in a Teensy 2.0++ board (AT90USB1286) with teensyduino.
 *
 * Behavior:
 *  This sketch will wait for a MESSAGE packet from the sender and after received an ACK 
 *  packet will be transmitted to the sender.
 * 	The Led will turn on when a packet is received.
 *	The Led will turn off after successfully transmitting an ACK packet.
 *
 * IMPORTANT:
 *   Make sure you have the correct CE_PIN and CSN_PIN definitions in radio.cpp and
 *   also MISO, MOSI, SCK and SS pins in spi.cpp
 *   
 *   Copy nRF24L01.h, packet.h, radio.cpp, radio.h, spi.cpp and spi.h in the same
 *   folder of your sketch.
 *  
 *   You also have to replace the #include "../arduino/WProgram.h" in radio.cpp, spi.cpp
 *   by #include "WProgram.h".
 *
 *   If you use teensy board, you have to change the spi pins macros in spi.cpp in order
 *   to include the teensy processors: __AVR_ATmega32U4__ (teensy 2.0) or __AVR_AT90USB1286__ (teensy 2.0++)
 */

#include "packet.h"
#include "radio.h"

#define LED_PIN 6 // Change accordingly to your board.

volatile uint8_t rxflag = 0; 
char output[128];

uint8_t station_addr[5] = { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 }; // Receiver address
uint8_t trans_addr[5] = { 0x98, 0x76, 0x54, 0x32, 0x10 };	// Transmitter address

RADIO_RX_STATUS rx_status;

radiopacket_t packet;
 
// setup function is called once at the program's start
void setup()
{
	pinMode(LED_PIN, OUTPUT);
	// start the serial output module at 57600 bps
	Serial.begin(57600);
 
	// initialize the radio, including the SPI module
	Radio_Init();
 
	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, station_addr, ENABLE);
 
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
 
	// print a message to UART to indicate that the program has started up
	snprintf(output, sizeof(output), "STATION START\n\r");
	Serial.print(output);
}
 
// loop function is called over and over while the system runs.
void loop()
{
	// The rxflag is set by radio_rxhandler function below indicating that a
	// new packet is ready to be read.
	if (rxflag)
	{
		rx_status = Radio_Receive(&packet); // Copy received packet to memory and store the result in rx_status.
		if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS) // Check if a packet is available.
		{
			digitalWrite(LED_PIN, HIGH); // Turn on the led.
        
			if (packet.type != MESSAGE)
			{
				snprintf(output, sizeof(output), "Error: wrong packet type: %d. Should be %d\n\r", packet.type, MESSAGE);
				Serial.print(output);
			}            

			// Print out the message, along with the message ID and sender address.
			snprintf(output, sizeof(output), "Message ID %d from 0x%.2X%.2X%.2X%.2X%.2X: '%s'\n\r",
				packet.payload.message.messageid,
				packet.payload.message.address[0],
				packet.payload.message.address[1],
				packet.payload.message.address[2],
				packet.payload.message.address[3],
				packet.payload.message.address[4],
				packet.payload.message.messagecontent);
			Serial.print(output); 

			// Use the commented line below to set the transmit address to the one specified in the received message packet.
			// Radio_Set_Tx_Addr(packet.payload.message.address);
			// 
			Radio_Set_Tx_Addr(packet.payload.message.address);  // or use the address manually informed by trans_addr.
			// Reply to the sender by sending an ACK packet, reusing the packet data structure.
			packet.type = ACK;
			// Se the ack message id:
			packet.payload.ack.messageid = packet.payload.message.messageid;
		

			if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT)
			{
				// If the max retries was reached, the packet was not acknowledged.
				// This usually occurs if the receiver was not configured correctly or
				// if the sender didn't copy its address into the radio packet properly.
				snprintf(output, sizeof(output), "Could not reply to sender.\n\r");
				Serial.print(output);
			}
			else
			{
			// the transmission was completed successfully
			snprintf(output, sizeof(output), "Replied to sender.\n\r");
			Serial.print(output);
			digitalWrite(LED_PIN, LOW); // turn off the led.
			}
		}
		rxflag = 0;  // clear the flag.
	}
} // End of main loop.

// The radio_rxhandler is called by the radio IRQ pin interrupt routine when RX_DR is read in STATUS register.
void radio_rxhandler(uint8_t pipe_number)
{
	rxflag = 1;
}
