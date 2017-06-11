#ifndef __UART_IRQ_H__
#define __UART_IRQ_H__

#include <ts7200.h>
#include <kernel.h>

typedef enum UART_IRQ_TYPE {
	XMIT,
	RCV
} UART_IRQ_TYPE;

uint32 uart_irq_mask();

#define	UART2_GENERAL_INTERRUPT	55
#define UART2_CTRL  (UART2_BASE + UART_CTLR_OFFSET) 
#define UART2_INTR	(UART2_BASE + UART_INTR_OFFSET) // Interrupt identification and clear registers
#define UART2_DATA	(UART2_BASE + UART_DATA_OFFSET)

#define	UART1_GENERAL_INTERRUPT	53
#define UART1_CTRL  (UART1_BASE + UART_CTLR_OFFSET) 
#define UART1_INTR	(UART1_BASE + UART_INTR_OFFSET) // Interrupt identification and clear registers
#define UART1_DATA	(UART1_BASE + UART_DATA_OFFSET)

#define UARTEN_MASK	0x1
#define MSIEN_MASK	0x8		// modem status int
#define RIEN_MASK	0x10	// receive int
#define TIEN_MASK	0x20	// transmit int
#define RTIEN_MASK	0x40	// receive timeout int
#define LBEN_MASK	0x80	// loopback 

uint32 uart_irq_mask(int channel);
void uart_irq_soft(int channel);
void uart_irq_soft_clear();
void mode_irq();
void uart_vic_enable(int channel);
void uart_irq_enable(int channel);
void uart_irq_disable(int channel);
void uart_device_enable(int channel, UART_IRQ_TYPE type);
void uart_device_disable(int channel, UART_IRQ_TYPE type);
void uart_irq_handle(int channel, Kernel_state *ks);

#endif // __UART_IRQ_H__

