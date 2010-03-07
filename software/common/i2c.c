/******************************************************************************/
/* File    :	i2c.c							      */
/* Function:	I2C bus functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "i2c.h"
#include "hardware.h"

#include "serial.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BUS_FREQ	400000	/* 400kHz bus frequency */

#define IDLE		0
#define START		1
#define RESTART		3
#define WRITE		5
#define READ		7
#define STOP		11


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	state	= 0;
static unsigned char	data	= 0;
static bit		ack	= 0;
static bit		error	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void i2c_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Set I2C controller in master mode */
	SSPCON  = 0x38;
	SSPCON2 = 0x00;

	/* Set I2C bus frequency */
	SSPADD = ((_XTAL_FREQ/4) / BUS_FREQ) /*-1*/;

	CKE = 1;	// use I2C levels      worked also with '0'
	SMP = 1;	// disable slew rate control  worked also with '0'
	
	PSPIF=0;	// clear SSPIF interrupt flag
	BCLIF=0;	// clear bus collision flag
}
/* End: i2c_init */


void i2c_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		-       */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Nothing to do here if busy */
	if ((SSPCON2 & 0x1F) | RW)
		return;

	switch (state) {
	case IDLE:
		break;

	case START:
		/* Start condition */
//putch('1');
		SEN = 1;
		state++;
		break;
	case START+1:
		state = IDLE;
		break;

	case RESTART:
//putch('2');
		/* Repeated start condition */
		RSEN = 1;
		state++;
		break;
	case RESTART+1:
		state = IDLE;
		break;

	case WRITE:
//putch('3');
		/* Write data */
		SSPBUF = data;
		state++;
		break;
	case WRITE+1:
		ack = !ACKSTAT;
		state = IDLE;
		break;

	case READ:
//putch('4');
		/* Enable reading */
		RCEN = 1;
		state++;
		break;
	case READ+1:
//putch('5');
		/* Read data */
		data = SSPBUF;
		state++;
		break;
	case READ+2:
//putch('6');
		/* Send (n)ack */
		ACKDT = ack?0:1;
		ACKEN = 1;
		state++;
		break;
	case READ+3:
		state = IDLE;
		break;

	case STOP:
//putch('7');
		/* Stop condition */
		PEN = 1;
		state++;
		break;
	case STOP+1:
		state = IDLE;
		break;
	}
} /* i2c_work */


void i2c_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: i2c_term */

unsigned char i2c_busy(void)
{
	return (state != IDLE);
}

void i2c_start(void)
{
	state = START;
}

void i2c_restart(void)
{
	state = RESTART;
}

void i2c_address(unsigned char byte, unsigned char read)
{
	data = byte << 1;
	if (read)
		data |= 0x01;
	state = WRITE;
}

void i2c_read(unsigned char ack)
{
}

unsigned char i2c_readbyte(void)
{
	return 0;
}

void i2c_write(unsigned char byte)
{
	data = byte;
	state = WRITE;
}

void i2c_stop(void)
{
	state = STOP;
}

unsigned char i2c_acked (void)
{
	return ack;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/