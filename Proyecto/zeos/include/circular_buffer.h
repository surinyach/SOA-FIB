#ifndef  CIRCULAR_BUFFER_H
#define  CIRCULAR_BUFFER_H

struct circular_buffer {
	char buffer[128];
	int cont;		
	int read;		
	int write;	
};

void INIT_CIRCULAR_BUFFER(struct circular_buffer *cb);

int cb_write(struct circular_buffer *cb, char c);

char cb_read(struct circular_buffer *cb);

#endif