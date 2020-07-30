#include <Server.h>


bool Server::start(const uint16_t port)
{
	#ifdef _WIN32 // Windows
	WSAData wsa_data;
	WORD DLL_version = MAKEWORD(2, 2);
	if (FAILED(WSAStartup(DLL_version, &wsa_data)))
	{
		last_error = WSAGetLastError();
		current_status = err_install;
		return false;
	}
	#endif

	server_address.sin_port = htons(port);
	server_address.sin_family = AF_INET;

    #ifdef _WIN32 // Windows
	server_address.sin_addr.S_un.S_addr = INADDR_ANY;

	server_socket = socket(AF_INET, SOCK_STREAM, NULL);
	if (server_socket == SOCKET_ERROR)
	{
		last_error = WSAGetLastError();
		current_status = err_socket_init;
		return false;
	}

	if (::bind(server_socket, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
	{
		last_error = WSAGetLastError();
		current_status = err_socket_bind;
		return false;
	}

	if (::listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		last_error = WSAGetLastError();
		current_status = err_socket_listening;
		return false;
	}

	#else // *nix
	server_address.sin_addr.s_addr = INADDR_ANY;

	server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0)
	{
		last_error = errno;
		current_status = err_socket_init;
		return false;
	}

	if(::bind(server_socket,(struct sockaddr *)&server_address , sizeof(server_address)) < 0) 
	{
		last_error = errno;
		current_status = err_socket_bind;
		return false;
	}

	if(::listen(server_socket, 3) < 0)
	{
		last_error = errno;
		current_status = err_socket_listening;
		return false;
	}
    #endif

	current_status = work;
	count_clients = 0;
	
	handle_thread_master = std::thread([this] {this->handle_loop_master(); });
	disconnect_thread_master = std::thread([this] {this->disconnect_loop_master(); });

	return true;
}

Server::Server(const uint16_t port) { start(port); }

Server::Server() {}



void Server::handle_loop_master()
{
	while (current_status == work)
	{
		sockaddr_in client_addr; 
		int addrlen = sizeof(client_addr);
		#ifdef _WIN32 // Windows
		SOCKET client_socket; 
		#else // *nix
		int client_socket; 
		#endif

		#ifdef _WIN32 // Windows
		if ((client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &addrlen)) != 0)
		#else // *nix
		if((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)) >= 0) 
		#endif
		{
			std::unique_ptr<Client> current_client = std::make_unique<Client>(client_socket, client_addr);
			
			printer_mtx.lock();
			std::cout << "New client connected " + current_client->get_clinen_info_str() + " | id = " + std::to_string(current_client->get_id()) << std::endl;
			printer_mtx.unlock();

			char msg[] = "Welcome to server";
			send_to(client_socket, msg, sizeof(msg));
			current_client->receive_data(*this);

			mtx.lock();
			pool_clients.push_back(std::move(current_client));
			++count_clients;
			mtx.unlock();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
	}
}


void Server::disconnect_loop_master()
{
	while (current_status == work)
	{
		while (!que_delet_clients.empty())
		{
			mtx.lock();
			auto front_id = que_delet_clients.front();
			pool_clients.remove_if([front_id](auto &cl) {return (front_id == cl->get_id()); });
			que_delet_clients.pop();
			--count_clients;
			mtx.unlock();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
	}
}

std::vector<uint8_t> Server::get_BER_size(const size_t var_size)
{
	std::vector<uint8_t> size;
	if (var_size < 128)
	{
		size.resize(1);
		size[0] = static_cast<uint8_t>(var_size);
	}
	else
	{
		size_t t = 255;
		uint8_t current_byte;
		uint8_t count = 0;
		std::vector<uint8_t> buff(8);
		do {
			current_byte = (t & var_size) >> (count * 8);
			t = t << 8;
			buff[count++] = current_byte;

		} while (current_byte > 0);

		size.resize(count);
		--count;
		size[0] = 0b10000000 + count;
		int j = 1;
		for (int i = buff.size() - 1; i >= 0; --i)
			if (buff[i] > 0)
				size[j++] = buff[i];
	}

	return size;
}


#ifdef _WIN32 // Windows
int Server::send_to(const SOCKET client_socket, const char * buf, const int length)
#else 
// *nix
int Server::send_to(const int client_socket, const char * buf, const int length)
#endif
{
	std::vector<uint8_t> msg = get_BER_size((size_t)length);
	std::vector<uint8_t> data(buf, buf + length);
	msg.insert(msg.end(), data.begin(), data.end());

	int res = send(client_socket, (char*)msg.data(), msg.size(), NULL);

	return res;
}


void Server::stop()
{
	current_status = close;
	#ifdef _WIN32 // Windows
	closesocket(server_socket);
	#else // *nix
    ::close(server_socket);
    #endif

    pool_clients.clear();
	while (!que_delet_clients.empty()) que_delet_clients.pop();
	count_clients = 0;

	#ifdef _WIN32 // Windows
	WSACleanup();
	#endif
}


Server::~Server()
{
	stop();
}


bool Server::send_to_client(const uint16_t id, const char *buf, int length)
{
	mtx.lock();
	for (auto it = pool_clients.begin(); it != pool_clients.end(); ++it)
	{
		if ((*(it))->get_id() == id)
		{
			send_to((*(it))->get_socket(), buf, length);
			mtx.unlock();
			return true;
		}
	}
	mtx.unlock();
	return false;
}


uint16_t Server::get_clinet_count()
{
	return count_clients;
}

Server::status Server::get_current_status()
{
	return current_status;
}

void Server::server_join()
{
	handle_thread_master.join();
	disconnect_thread_master.join();
}




uint16_t Client::count_id = 0;

std::string Client::get_clinen_info_str()
{
	#ifdef _WIN32 // Windows
	return std::string()
		+ std::to_string(address.sin_addr.S_un.S_un_b.s_b1) + "."
		+ std::to_string(address.sin_addr.S_un.S_un_b.s_b2) + "."
		+ std::to_string(address.sin_addr.S_un.S_un_b.s_b3) + "."
		+ std::to_string(address.sin_addr.S_un.S_un_b.s_b4) + 
		+ ":" + std::to_string((ntohs(address.sin_port)));
	#else // *nix
	char host[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &address.sin_addr, host, INET_ADDRSTRLEN);
	return std::string(host) + ":" + std::to_string(ntohs(address.sin_port));
	#endif
}

void Client::client_join()
{
	rx_client_thred.join();
}




int Client::recv_from()
{
	int size = recv(socket, buffer, CLIENT_BUFFER_SIZE, NULL);
	if (size <= 0) return size;
	if (static_cast<uint8_t>(buffer[0]) < 128)
	{
		received_data.resize(0); 
		received_data.insert(received_data.begin(), buffer + 1, buffer + buffer[0] + 1);
	}
	else
	{
		uint8_t count = buffer[0] & 0b01111111;
		size_t t = 255;
		size_t size_msg = 0;
		uint8_t j = 0;
		for (int i = count; i > 0 ; --i)
		{
			size_msg |= (buffer[i] & t) << (8*j++);
		}
		received_data.resize(0);
		received_data.insert(received_data.begin(), (buffer + 1 + count), (buffer + 1 + count + size_msg));
	}
	return size;
}

#ifdef _WIN32 // Windows
Client::Client(const SOCKET socket, const sockaddr_in address)
#else // *nix
Client::Client(const int socket, const sockaddr_in address)
#endif
{
	this->socket = socket;
	this->address = address;
	id = count_id++;
}

Client::~Client()
{
	if (rx_client_thred.joinable()) 
		rx_client_thred.join();

	#ifdef _WIN32 // Windows
	shutdown(socket, 0); 
	closesocket(socket);
	#else // *nix
	shutdown(socket, 0);
    close(socket);
    #endif

}

void Client::receive_data(Server & server)
{
	rx_client_thred = std::thread([this, &server]()
	{
		while (server.current_status == Server::status::work)
		{
			int size = recv_from();
			if (size > 0)
			{
				server.printer_mtx.lock();
				std::cout << "New message from " + get_clinen_info_str() + " | id = " + std::to_string(id) + " : " << std::endl;
				std::cout << std::string_view(received_data.data(), received_data.size()) << std::endl;
				server.printer_mtx.unlock();
			}
			else
			{
				server.printer_mtx.lock();
				std::cout << "Client disconnected " + get_clinen_info_str() + " | id = " + std::to_string(id) << std::endl;
				server.printer_mtx.unlock();
				server.mtx.lock();
				server.que_delet_clients.push(get_id());
				server.mtx.unlock();
				break;
			}
		}
	}
	);



}

uint32_t Client::get_ip() const
{
	return uint32_t();
}

uint16_t Client::get_port() const
{
	return uint16_t();
}

sockaddr_in Client::get_address() const
{
	return address;
}

#ifdef _WIN32 // Windows
SOCKET Client::get_socket() const
#else // *nix
int Client::get_socket() const
#endif
{
	return socket;
}

uint16_t Client::get_id() const
{
	return id;
}
