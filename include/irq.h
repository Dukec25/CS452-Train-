#ifndef __IRQ_H__
#define __IRQ_H__

#include <define.h>
#include <ts7200.h>
#include <kernel.h>

/* irq */
#define HWI_MASK			0x8000000
#define VIC2_BASE			0x800C0000
#define VIC2_INT_SEL		0x800C000C
#define VIC2_INT_ENBL		0x800C0010
#define	VIC2_VEC_ADDR		0x800C0030
#define	VIC2_INT_ENBL_CLR	0x800C0014
#define VIC2_SOFT_INT		0x800C0018
#define VIC2_SOFT_INT_CLR	0x800C001C

/* timer */
#define	TIMER_UNDER_FLOW_INTERRUPT	52
#define TIMER_LDR 					(TIMER3_BASE + LDR_OFFSET)
#define TIMER_VAL 					(TIMER3_BASE + VAL_OFFSET)
#define TIMER_CTRL					(TIMER3_BASE + CRTL_OFFSET)
#define TIMER_CLR					(TIMER3_BASE + CLR_OFFSET)
#define	TIMER_REQUENCY				50800
#define	SEC							1000
#define	TICK						10

#define SOFT

void irq_enable();
void irq_disable();
void irq_handle(Kernel_state *ks);
void timer_enable();
void timer_irq_enable();
void timer_irq_soft_enable();
void timer_irq_disable();
void timer_irq_handle(Kernel_state *ks);

#endif //__IRQ_H__
