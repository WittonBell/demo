#include <stdint.h>

typedef enum {
	ENetCodeOK = 0,
}ENetCode;

typedef struct {
	int port;
}netOption;

ENetCode startNet(netOption opt);
