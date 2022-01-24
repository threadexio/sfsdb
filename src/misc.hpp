#pragma once

#include <chrono>
#include <thread>

#define HANDLER_SUCCESS		0
#define HANDLER_ERROR		1
#define HANDLER_NO_SEND_RES 2

#define MAX_NET_MSG_LEN 255

#define MSG_TIMEOUT_MS 20

#define DO_TIMEOUT                                                             \
	std::this_thread::sleep_for(std::chrono::milliseconds(MSG_TIMEOUT_MS))
