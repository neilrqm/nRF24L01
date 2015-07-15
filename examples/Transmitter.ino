/*
 * Transmitter.pde
 *
 * Example of using NRF24L01 driver by Neil MacMillan in an Arduino plataform.
 *
 * Based on the examples by Neil MacMillan in http://nrqm.ca/nrf24l01/examples/
 *
 * Adapted by Miguel Moreto
 * Brazil, 2011.
 *
 * This example was tested in a Teensy 2.0 board (ATMEGA32U4) with teensyduino.
 *
 * Behavior:
 *  This sketch will wait transmit MESSAGE packet to the receiver (station_addr) at 
 *  a periodic rate (in this example is 1 second).
 * 	The Led will turn on right before the packet sending. It will turn off
 *  only after receiving the ACK packet from the remote station.
 *
 * Notes:
 *   You have to install the metro library in order to run this sketch as it is.
 *   If you don't want to use metro, you can replace it by another kind of timing check.
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
 */
 
#include "packet.h"
#include "radio.h"
//#include <Metro.h> // Include the Metro library to create a periodic task for sending messages.

#define LED_PIN 13 // Change accordingly to your board.

// Slowing down the clock because the teensy board running on 3.3V (16MHz in this case is not recommended).
// If you power your board with 5V, comment the following lines.
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_8MHz        0x01

 
volatile uint8_t rxflag = 0;
 
 
uint8_t station_addr[5] = { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 }; // Receiver address
uint8_t my_addr[5] = { 0x98, 0x76, 0x54, 0x32, 0x11 }; // Transmitter address
 
radiopacket_t packet;

//Metro DisplayMetro = Metro(500); // 1 second in 8MHz clock
 
void setup()
{
  //for (;;);
	//CPU_PRESCALE(CPU_8MHz);
	Serial.begin(57600);
	pinMode(10, OUTPUT);
	digitalWrite(10, LOW);
	delay(100);
	digitalWrite(10, HIGH);
	//pinMode(LED_PIN, OUTPUT);
	Radio_Init();

	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);

	// load up the packet contents
	packet.type = MESSAGE;
	memcpy(packet.payload.message.address, my_addr, RADIO_ADDRESS_LENGTH);
	packet.payload.message.messageid = 55;
	snprintf((char*)packet.payload.message.messagecontent, sizeof(packet.payload.message.messagecontent), "Test message.");

	Radio_Set_Tx_Addr(station_addr);
	//Serial.println("Yay");
	//digitalWrite(LED_PIN, HIGH);
	//Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
	delay(100);
}

void loop()
{
	delay(1000);
	//digitalWrite(LED_PIN, HIGH);
	// load up the packet contents again because the packet was used as an ACK packet before.
	packet.type = MESSAGE;
	memcpy(packet.payload.message.address, my_addr, RADIO_ADDRESS_LENGTH);
	packet.payload.message.messageid = 55;
	snprintf((char*)packet.payload.message.messagecontent, sizeof(packet.payload.message.messagecontent), "Test message.");

	if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT) // Transmitt packet.
	{
		Serial.println("Data not trasmitted. Max retry.");
	}
	else // Transmitted succesfully.
	{
		Serial.println("Data trasmitted.");
	}
	//Serial.println("Woo");
	
	// The rxflag is set by radio_rxhandler function below indicating that a
	// new packet is ready to be read.
	if (rxflag)
	{
		if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS) // Receive packet.
		{
			// if there are no more packets on the radio, clear the receive flag;
			// otherwise, we want to handle the next packet on the next loop iteration.
			rxflag = 0;
		}
		if (packet.type == ACK)
		{
			//digitalWrite(LED_PIN, LOW); // Turn off the led.
		}
    }
}

// The radio_rxhandler is called by the radio IRQ pin interrupt routine when RX_DR is read in STATUS register.
void radio_rxhandler(uint8_t pipe_number)
{
	rxflag = 1;
}

