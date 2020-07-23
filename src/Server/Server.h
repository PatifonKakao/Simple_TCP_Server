#pragma once

#ifdef _WIN32 // Windows 

#pragma comment( lib, "wsock32.lib" )
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

#include <thread>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <mutex>
#include <queue>
#include <iostream>



class Client;


class Server 
{
public:
	enum status
	{
		work = 0,
		err_install = 1,
		err_socket_init = 2,
		err_socket_bind = 3,
		err_socket_listening = 4,
		close = 5

	};

	Server();
	Server(const uint16_t port);
	~Server();

	bool send_to_client(const uint16_t id, const char *buf, int length);
	uint16_t get_clinet_count();

	status get_current_status();

	bool start(const uint16_t port);
	void stop();
	
	void server_join();
private:
	friend class Client;

	std::mutex mtx;
	std::mutex printer_mtx;

	static const uint32_t SERVER_BUFFER_SIZE = 1024;
	char buffer[SERVER_BUFFER_SIZE];

	const uint32_t DELAY = 50;

	status current_status = close;
	int last_error;

	uint16_t port;

	std::vector<uint8_t> get_BER_size(const char* var, const size_t var_size);

	#ifdef _WIN32 // Windows
	SOCKET server_socket;
	int send_to(const SOCKET client_socket, const char *buf, const int length);
	#else // *nix
	int server_socket;
	int send_to(const int client_socket, const char *buf, const int length);
	#endif
	sockaddr_in server_address;

	std::thread handle_thread_master;
	std::thread disconnect_thread_master;

	void handle_loop_master();
	void disconnect_loop_master();
	
	uint16_t count_clients;

    std::list<std::unique_ptr<Client>> pool_clients;

	std::queue<uint16_t> que_delet_clients;
	
	

};


class Client
{
public:
    #ifdef _WIN32 // Windows
	Client(const SOCKET socket, const sockaddr_in address);
	SOCKET get_socket() const;
	#else // *nix
    Client(const int socket, const sockaddr_in address);
	int get_socket() const;
	#endif
	~Client();

	sockaddr_in get_address() const;

	void receive_data(Server & server);

	uint32_t get_ip() const;
	uint16_t get_port() const;

	uint16_t get_id() const;
	std::string get_clinen_info_str();

	void client_join();

	std::vector<char> received_data;
private:

	static const uint16_t CLIENT_BUFFER_SIZE = 1024;
	char buffer[CLIENT_BUFFER_SIZE];

	#ifdef _WIN32 // Windows
	SOCKET socket;
	#else // *nix
	int socket;
	#endif

	sockaddr_in address;

	std::thread rx_client_thred;

	int recv_from();
	static uint16_t count_id;
	int id;
};

