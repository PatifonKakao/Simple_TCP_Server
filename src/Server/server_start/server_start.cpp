
#include <Server.h>

#include <iostream>
#include <random>


int main(int argc, char* argv[])
{
	std::default_random_engine rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 20);
	auto t = std::thread([&]() {
		Server server(8080);
		if (Server::status::work == server.get_current_status())
			std::cout << "waiting clients ..." << std::endl;
		int n = 0;
		while (Server::status::work == server.get_current_status())
		{
			int cl = dist(rng);
			std::string str = "msg " + std::to_string(n++);
			server.send_to_client(cl, str.data(), str.length());
			std::this_thread::sleep_for(std::chrono::milliseconds(100 + cl*100));
		}
		
		server.server_join();
	});

	t.join();
    return 0;
}