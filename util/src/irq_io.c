#include <irq_io.h>

void irq_format(int channel, char *fmt, va_list va){
	char bf[12];
	char ch, lz;
	int w;
    char buffer[80]; // can send maximum 80 chars at a time. 
    int idx = 0; 
	
	while ((ch = *(fmt++))) {
		if (ch != '%')
            buffer[idx++] = ch;
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch (ch) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i(ch, &fmt, 10, &w);
				break;
			}
			switch(ch) {
			case 0: return;
			case 'c':
                buffer[idx++] = va_arg(va, char);
				break;
			case 's':
				idx = irq_putw(w, 0, va_arg(va, char*), buffer, idx);
				break;
			case 'u':
				ui2a(va_arg(va, unsigned int), 10, bf);
				idx = irq_putw(w, lz, bf, buffer, idx);
				break;
			case 'd':
				i2a(va_arg(va, int), bf);
				idx = irq_putw(w, lz, bf, buffer, idx);
				break;
			case 'x':
				ui2a(va_arg(va, unsigned int), 16, bf);
				idx = irq_putw(w, lz, bf, buffer, idx);
				break;
			case '%':
                buffer[idx++] = ch;
				break;
			}
		}
	}
    buffer[idx] = '\0';

    vint io_server_id;
	switch (channel) {
        case COM1:
            io_server_id = WhoIs("IO_SERVER_UART1_TRANSMIT");
            break;
        case COM2:
            io_server_id = WhoIs("IO_SERVER_UART2_TRANSMIT");
            break;
	}

    Delivery request;
    request.type = PRINTF;
    request.data_arr = buffer;
    Delivery reply_msg;
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
}

int irq_putw(int n, char fc, char *bf, char *val, int idx)
{
	char ch;
	char *p = bf;

	while (*p++ && n > 0) n--;
	while (n-- > 0) val[idx++] = fc;
	while ((ch = *bf++)) val[idx++] = ch;

    return idx;
}

void irq_printf(int channel, char *fmt, ...){
    va_list va;

    va_start(va,fmt);
    irq_format(channel, fmt, va);
    va_end(va);
}
