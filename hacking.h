#ifndef HACKING_H
#define HACKING_H

void fatal(char *message);

void *ec_malloc(unsigned int size);

void dump(const unsigned char *data_buffer, const unsigned int length);

#endif