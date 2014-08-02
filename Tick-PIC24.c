/*	Library for time management on various microcontrollers
	Copyright (C) 2014 Jesus Ruben Santa Anna Zamudio.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Tick.h"

/* Internal counter for timer interrupts */
static volatile unsigned long tickcnt = 0;
/* Total counter 6 bytes including timer register value */
static unsigned char tickbuffer[6];

static void tick_read_internal();

void tick_init()
{
	// Prescaler bits
	T1CONbits.TCKPS = 3;
	// Initialize timer registers
	PR1 = 0xFFFF;
	TMR1 = 0;
	// Initialize timer interrupts
	IPC0bits.T1IP = 2;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	// Start timer
	T1CONbits.TON = 1;
}

DWORD tick_get()
{
	tick_read_internal();
	return *((unsigned long *) &tickbuffer[0]);
}

static void tick_read_internal()
{
	do {
		DWORD xTempTicks;

		IEC0bits.T1IE = 1; // Enable interrupt
		Nop();
		IEC0bits.T1IE = 0; // Disable interrupt

		// Get low 2 bytes
		((WORD*) tickbuffer)[0] = TMR1;

		// Correct corner case where interrupt increments byte[4+] but 
		// TMR1 hasn't rolled over to 0x0000 yet
		xTempTicks = tickcnt;
		if (((WORD*) tickbuffer)[0] == 0xFFFFu)
			xTempTicks--;

		// Get high 4 bytes
		tickbuffer[2] = ((BYTE *) & xTempTicks)[0];
		tickbuffer[3] = ((BYTE *) & xTempTicks)[1];
		tickbuffer[4] = ((BYTE *) & xTempTicks)[2];
		tickbuffer[5] = ((BYTE *) & xTempTicks)[3];
	} while (IFS0bits.T1IF);
	IEC0bits.T1IE = 1; // Enable interrupt
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
	// Increment internal high tick counter
	tickcnt++;
	// Reset interrupt flag
	IFS0bits.T1IF = 0;
}
