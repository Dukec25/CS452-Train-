#ifndef __UART_IRQ_H__
#define __UART_IRQ_H__

#include <ts7200.h>
#include <kernel.h>
#include <irq.h>
#include <define.h>
#include <io_server.h>
#include <clock_server.h>

#define	UART2_GENERAL_INTERRUPT	55
#define UART2_CTRL  (UART2_BASE + UART_CTLR_OFFSET) 
	#define UARTEN_MASK	0x1
	#define MSIEN_MASK	0x8	// modem status int
	#define RIEN_MASK	0x10	// receive int
	#define TIEN_MASK	0x20	// transmit int
	#define RTIEN_MASK	0x40	// receive timeout int
	#define LBEN_MASK	0x80	// loopback 
#define UART2_INTR	(UART2_BASE + UART_INTR_OFFSET) // Interrupt identification and clear registers


#endif // __UART_IRQ_H__

