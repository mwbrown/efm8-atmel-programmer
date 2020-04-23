/*******************************************************************************
 * @file callback.c
 * @brief USB Callbacks.
 *******************************************************************************/

//=============================================================================
// src/callback.c: generated by Hardware Configurator
//
// This file is only generated if it does not exist. Modifications in this file
// will persist even if Configurator generates code. To refresh this file,
// you must first delete it and then regenerate code.
//=============================================================================
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_EFM8UB1_Register_Enums.h>
#include <efm8_usb.h>
#include "descriptors.h"

#include "atusbprog_proto.h"
#include "debug_uart.h"

#include "spi_0.h"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define USB_BUF_SEG SI_SEG_IDATA

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

typedef void (*usbRxHandler_t)(uint16_t len);

typedef union {
    uint8_t buf[AUP_MAX_PACKET_SIZE];
    aup_in_msg_t msg;
} usb_tx_t;

typedef union {
    uint8_t buf[AUP_MAX_PACKET_SIZE];
    aup_out_msg_t msg;
} usb_rx_t;

static void atusbprog_setup_tx(uint8_t len_msg);
static void atusbprog_setup_rx(void);

// Command Handlers
static void usbRx_VersionReq(uint16_t len);
static void usbRx_LedReq(uint16_t len);
static void usbRx_InitPgmModeReq(uint16_t len);
static void usbRx_DataWrite(uint16_t len);
static void usbRx_RstReq(uint16_t len);

/* This must be kept up-to-date with aup_msg_type_t. */
SI_SEGMENT_VARIABLE(usbRxHandlers[], const usbRxHandler_t, SI_SEG_CODE) = {
    usbRx_VersionReq,
    usbRx_LedReq,
    usbRx_InitPgmModeReq,
    usbRx_DataWrite,
    usbRx_RstReq,
};

SI_SEGMENT_VARIABLE(numUsbRxHandlers, const size_t, SI_SEG_CODE) =
        sizeof(usbRxHandlers) / sizeof(usbRxHandlers[0]);

SI_SEGMENT_VARIABLE(usb_tx, usb_tx_t, USB_BUF_SEG);
SI_SEGMENT_VARIABLE(usb_rx, usb_rx_t, USB_BUF_SEG);

void USBD_EnterHandler(void) {

}

void USBD_ExitHandler(void) {

}

void USBD_ResetCb(void) {

}

// TODO: move this
static void atusbprog_setup_rx(void) {

    int8_t status;

    if (USBD_EpIsBusy(EP1OUT)) {
        return;
    }

    debug_uart_write_char('r');
    status = USBD_Read(EP1OUT, (uint8_t *)usb_rx.buf, AUP_MAX_PACKET_SIZE, true);

    if (status != 0) {
    	debug_uart_write_char('!');
    }
}

static void atusbprog_setup_tx(uint8_t len_msg) {

    int8_t status;

    if (USBD_EpIsBusy(EP1IN)) {
        return;
    }

    debug_uart_write_char('t');
    status = USBD_Write(EP1IN, (uint8_t *)usb_tx.buf, len_msg + AUP_IN_MSG_HDR_LEN, false);

    if (status != 0) {
    	debug_uart_write_char('!');
    }
}

void USBD_SofCb(uint16_t sofNr) {
    UNREFERENCED_ARGUMENT(sofNr);
    //atusbprog_setup_rx();
}

USB_Status_TypeDef USBD_SetupCmdCb(
        SI_VARIABLE_SEGMENT_POINTER(setup, USB_Setup_TypeDef, MEM_MODEL_SEG)) {

    USB_Status_TypeDef retVal = USB_STATUS_REQ_UNHANDLED;

    UNREFERENCED_ARGUMENT(setup);

    return retVal;
}

static void usbRx_VersionReq(uint16_t len) {

    UNREFERENCED_ARGUMENT(len);

    usb_tx.msg.msg_type = AUP_MSG_VERSION_REQ_RSP;
    usb_tx.msg.msg_data.version_rsp.maj = AUP_PROTO_VERSION_MAJ;
    usb_tx.msg.msg_data.version_rsp.min = AUP_PROTO_VERSION_MIN;
    usb_tx.msg.msg_data.version_rsp.rev = AUP_PROTO_VERSION_REV;
    atusbprog_setup_tx(sizeof(aup_in_msg_version_rsp_t));
}

static void usbRx_LedReq(uint16_t len) {

    UNREFERENCED_ARGUMENT(len);

    // todo cleanup
    // need note on inverted LED (i.e. bsp define, etc)

    debug_uart_write_char('u');

    if (usb_rx.msg.msg_data.led_req.led_mask & LED_REQ_MASK_RED) {
        PIN_LED_RED = usb_rx.msg.msg_data.led_req.led_val & LED_REQ_MASK_RED ? 0 : 1;
    }

    if (usb_rx.msg.msg_data.led_req.led_mask & LED_REQ_MASK_GRN) {
        PIN_LED_GRN = usb_rx.msg.msg_data.led_req.led_val & LED_REQ_MASK_GRN ? 0 : 1;
    }

    if (usb_rx.msg.msg_data.led_req.led_mask & LED_REQ_MASK_BLU) {
        PIN_LED_BLU = usb_rx.msg.msg_data.led_req.led_val & LED_REQ_MASK_BLU ? 0 : 1;
    }
}

static void usbRx_InitPgmModeReq(uint16_t len) {

    UNREFERENCED_ARGUMENT(len);

    debug_uart_write_char('p');

    if (usb_rx.msg.msg_data.pgm_mode_req.chip_id == PGM_MODE_CHIP_ID_RAW_SPI) {

    } else {

    }
}

static void usbRx_DataWrite(uint16_t len) {

    // TODO: just do a spi write
    // TODO: this should be moved out of the USB interrupt

	uint8_t oldLen;

    SI_SEGMENT_VARIABLE(dptr_tx, uint8_t *, USB_BUF_SEG);
    SI_SEGMENT_VARIABLE(dptr_rx, uint8_t *, USB_BUF_SEG);

    if (len <= AUP_IN_MSG_HDR_LEN) {
        /* No data to send. */
        return;
    }

    len -= AUP_IN_MSG_HDR_LEN;
    oldLen = len; // Save for the tx later

    dptr_tx = usb_rx.msg.msg_data.data_write.payload;

    // Set up the response.
    usb_tx.msg.msg_type = AUP_MSG_DATA_WRITE;
    dptr_rx = usb_tx.msg.msg_data.data_write.payload;

    // Fill the fifo
    while (len) {
    	// TODO: switch to using better xfr api
        SPI0_pollWriteByte(*dptr_tx);
        *dptr_rx = SPI0_pollReadByte();
        dptr_tx++;
        dptr_rx++;
        len--;
    }

#if 0
    // Wait for the byte to finish sending
    while (SPI0_isBusy())
    {
    }
#endif

    // Transmit the response.
    atusbprog_setup_tx(oldLen);

}

static void usbRx_RstReq(uint16_t len)
{
	UNREFERENCED_ARGUMENT(len);

	debug_uart_write_char('*');

	/* Update the RST pin before OE. */
	PIN_AT_RST = (usb_rx.msg.msg_data.rst_req.flags & RST_REQ_FLAG_RST) ? 1 : 0;

	/* OE_N is inverted (flag high = low output). */
	PIN_AT_OE_N = (usb_rx.msg.msg_data.rst_req.flags & RST_REQ_FLAG_OE) ? 0 : 1;
}

uint16_t USBD_XferCompleteCb(uint8_t epAddr, USB_Status_TypeDef status,
        uint16_t xferred, uint16_t remaining) {

    usbRxHandler_t handler = NULL;
    uint8_t command_id;

    //UNREFERENCED_ARGUMENT(epAddr);
    UNREFERENCED_ARGUMENT(status);
    UNREFERENCED_ARGUMENT(xferred);
    UNREFERENCED_ARGUMENT(remaining);

    /* If this is not an OUT transaction (Host -> UB1), we don't care. */
    if (epAddr != EP1OUT) {
        return 0;
    }

    if  (status != USB_STATUS_OK) {
    	debug_uart_write_char('S');
        return 0;
    }

    command_id = usb_rx.msg.msg_type;
    if (command_id < numUsbRxHandlers) {
        handler = usbRxHandlers[command_id];
    }

    // Did we find a handler?
    if (handler != NULL) {
        handler(xferred);
    } else {
    	debug_uart_write_char('?');
        // Send a generic command response back indicating an error.
        // TODO
    }

    // Setup the next read.
    atusbprog_setup_rx();

    return 0;
}

void USBD_DeviceStateChangeCb(USBD_State_TypeDef oldState, USBD_State_TypeDef newState)
{
	UNREFERENCED_ARGUMENT(oldState);

    if (newState == USBD_STATE_CONFIGURED)
    {
        atusbprog_setup_rx();
    }
}
