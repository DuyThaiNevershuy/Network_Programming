#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_CLIENT 10
#define MAX_MSG_LEN 1024
