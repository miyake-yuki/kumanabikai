#ifndef HACKINGNET_H
#define HACKINGNET_H

int send_string(int sockfd, char *buffer);
int recv_line(int sockfd, char *dest_buffer);

#endif