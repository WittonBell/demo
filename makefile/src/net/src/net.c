#include "../net.h"
#include <event2/event.h>

ENetCode startNet(netOption opt) {
	struct event_config* cfg = event_config_new();
	event_config_free(cfg);

	return ENetCodeOK;
}
