//=========================================================
// src/Interrupts.c: generated by Hardware Configurator
//
// This file will be regenerated when saving a document.
// leave the sections inside the "$[...]" comment tags alone
// or they will be overwritten!
//=========================================================

// USER INCLUDES
#include <SI_EFM8UB1_Register_Enums.h>

static unsigned char blink;

//-----------------------------------------------------------------------------
// TIMER0_ISR
//-----------------------------------------------------------------------------
//
// TIMER0 ISR Content goes here. Remember to clear flag bits:
// TCON::TF0 (Timer 0 Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER0_ISR, TIMER0_IRQn)
{
	//P1_B6 = blink;
	blink = !blink;

	TCON_TF0 = 0;
}

//-----------------------------------------------------------------------------
// UART1_ISR
//-----------------------------------------------------------------------------
//
// UART1 ISR Content goes here. Remember to clear flag bits:
// SCON1::RI (Receive Interrupt Flag)
// SCON1::TI (Transmit Interrupt Flag)
// UART1FCN1::RFRQ (Receive FIFO Request)
// UART1FCN1::TFRQ (Transmit FIFO Request)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (UART1_ISR, UART1_IRQn)
{
	unsigned char c;
	unsigned char sfr_save = SFRPAGE;
	SFRPAGE = 0x20;

	// Clear the interrupt flag and echo the output.
	SCON1_RI = 0;
	c = SBUF1;
	SBUF1 = c;

	SFRPAGE = sfr_save;
}
