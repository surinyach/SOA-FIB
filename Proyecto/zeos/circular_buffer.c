#include <circular_buffer.h>

void INIT_CIRCULAR_BUFFER(struct circular_buffer *cb) {
	cb->cont = 0;
	cb->read = 0;
	cb->write = 0;
}

int cb_write(struct circular_buffer *cb, char c) {
	if(cb->cont < 128) {
		cb->buffer[cb->write] = c;
		++cb->cont;
		if(cb->write == 127) cb->write = 0;
		else ++cb->write;
		return 0;  
	}
	else return -1; 
}

char cb_read(struct circular_buffer *cb) {
	if(cb->cont != 0) {
		char c = cb->buffer[cb->read];
		--cb->cont;
		if(cb->read == 127) cb->read = 0;
		else ++cb->read;
		return c;
	}
	else return '\0';
}