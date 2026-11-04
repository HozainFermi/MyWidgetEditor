#pragma once
// httplib тянет winsock2.h, поэтому подключаем его только в .cpp, а не в заголовках виджетов
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
