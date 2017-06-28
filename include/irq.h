#ifndef __IRQ_H__
#define __IRQ_H__

#include <define.h>
#include <ts7200.h>
#include <kernel.h>

/* irq */
#define HWI_MASK			0x80000000
#define VIC2_BASE			0x800C0000
#define	VIC2_IRQ_STATUS		0x800C0000
#define VIC2_INT_SEL		0x800C000C
#define VIC2_INT_ENBL		0x800C0010
#define	VIC2_VEC_ADDR		0x800C0030
#define	VIC2_INT_ENBL_CLR	0x800C0014
#define VIC2_SOFT_INT		0x800C0018
#define VIC2_SOFT_INT_CLR	0x800C001C

/* timer 3 */
#define	TIMER3_UNDER_FLOW_INTERRUPT	52
#define TIMER3_LDR 					(TIMER3_BASE + LDR_OFFSET)
#define TIMER3_VAL 					(TIMER3_BASE + VAL_OFFSET)
#define TIMER3_CTRL					(TIMER3_BASE + CRTL_OFFSET)
#define TIMER3_CLR					(TIMER3_BASE + CLR_OFFSET)
#define	TIMER3_REQUENCY				508000
#define	SEC							1000
#define	TICK						10

void irq_enable();
void irq_disable();
void irq_handle(Kernel_state *ks, int *cts_send);

void timer3_enable();
void timer3_clear();
void timer3_irq_enable();
void timer3_irq_soft();
void timer3_irq_disable();
void timer3_irq_soft_clear();
void timer3_irq_handle(Kernel_state *ks);

#endif //__IRQ_H__
