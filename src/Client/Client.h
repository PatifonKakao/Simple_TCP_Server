
#pragma once

#ifdef _WIN32 // Windows 
#pragma comment( lib, "wsock32.lib" )
#pragma warning(disable: 4996)
#include <winsock2.h>
#else // *nix

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#include <stdint.h>
#include <vector>
#include<iostream>
#include <string>
#include <string_view>
#include <mutex>


class Client
{
public:

	enum status
	{
		connected = 0,
		err_install = 1,
		err_socket_init = 2,
		err_socket_connect = 3,
		err_socket_listening = 4,
		disconnected = 5

	};

	Client(uint32_t ip_host, uint16_t port);
	~Client();

	int receive_data(std::mutex &printer_mtx);
	const char* get_receive_data() const { return received_data.data(); }
	size_t get_size_data() const { return received_data.size(); }

	int send_data(const char* buffer, const int length);

	int get_current_status() const { return current_status; }
	int get_last_error() const { return last_error; }

	int get_id() const { return id; }

private:
	static const uint32_t MAX_BUFFER_SIZE = 1024;
	char buffer[MAX_BUFFER_SIZE];

	status current_status = disconnected;
	int last_error;

	#ifdef _WIN32 // Windows
	SOCKET client_socket;
	#else // *nix
	int client_socket;
	#endif

    struct sockaddr_in destination_address;

	std::vector<uint8_t> get_BER_size(const size_t var_size);
	std::vector<char> received_data;

	static int count_id;
	int id;
	
};

