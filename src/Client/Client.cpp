#include <Client.h>

int Client::count_id = 0;

Client::Client(uint32_t ip_host, uint16_t port)
{
	#ifdef _WIN32 // Windows
	WSAData wsa_data;
	WORD DLL_version = MAKEWORD(2, 2);
	if (FAILED(WSAStartup(DLL_version, &wsa_data)))
	{
		last_error = WSAGetLastError();
		current_status = err_install;
		return;
	}

	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (client_socket == INVALID_SOCKET)
	{
		last_error = WSAGetLastError();
		current_status = err_socket_init;
		return;
	}

    #else // *nix
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket  < 0)
	{
		last_error = errno;
		current_status = err_socket_init;
		return;
	}
	#endif
	
	
	destination_address.sin_port = htons(port);
	destination_address.sin_family = AF_INET;
    
	#ifdef _WIN32 // Windows
	destination_address.sin_addr.S_un.S_addr = ip_host;

	if (connect(client_socket, (sockaddr *)&destination_address, sizeof(destination_address)) == SOCKET_ERROR) 
	{
		closesocket(client_socket);
		last_error = WSAGetLastError();
		current_status = err_socket_connect;
		return;
	}
    #else // *nix
	destination_address.sin_addr.s_addr = ip_host;

    if ((connect(client_socket, (struct sockaddr *)&destination_address, (sizeof(destination_address)))) < 0)
	{
        close(client_socket);
	    last_error = errno;
	    current_status = err_socket_connect;
	    return;
	}
	#endif
	
	current_status = connected;
	id = count_id++;
}


std::vector<uint8_t> Client::get_BER_size(const size_t var_size)
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


int Client::receive_data(std::mutex &printer_mtx)
{
	int size = recv(client_socket, buffer, MAX_BUFFER_SIZE, NULL);
	if (size <= 0)
	{
		printer_mtx.lock();
		std::cout << std::string() + "Client disconnected id = " + std::to_string(id) << std::endl;
		printer_mtx.unlock();
		current_status = disconnected;
		return size;
	}
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
		for (int i = count; i > 0; --i)
		{
			size_msg |= (buffer[i] & t) << (8 * j++);
		}
		received_data.resize(0);
		received_data.insert(received_data.begin(), (buffer + 1 + count), (buffer + 1 + count + size_msg));
	}
	printer_mtx.lock();
	std::cout << std::string() + "id:" + std::to_string(id) + " "  << std::string_view(received_data.data(), received_data.size()) << std::endl;
	printer_mtx.unlock();
	return size;
}


int Client::send_data(const char* buffer, const int length)
{
	std::vector<uint8_t> msg = get_BER_size((size_t)length);
	std::vector<uint8_t> data(buffer, buffer + length);
	msg.insert(msg.end(), data.begin(), data.end());

	auto res = send(client_socket, (char*)msg.data(), msg.size(), NULL);
	return res;
}


Client::~Client()
{
	current_status = disconnected;
	#ifdef _WIN32 // Windows
	closesocket(client_socket);
	WSACleanup();
	#else // *nix
	close(client_socket);
	#endif
}

