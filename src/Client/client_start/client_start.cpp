
#include <Client.h>

#include<iostream>


#include <chrono>
#include <thread>

#include <string>
#include <mutex>
#include <vector>
#include <memory>

#include <random>



int main(int argc, char* argv[])
{
	std::mutex print_mtx;
	std::string ip_str = "127.0.0.1";
	uint16_t port = 8080;
	uint32_t ip = inet_addr(ip_str.c_str());

	const int cout_clients = 10;
	std::vector<std::unique_ptr<Client>> clients;
	std::vector<std::thread> rx_cls_threads;
	std::vector<std::thread> tx_cls_threads;

	clients.resize(cout_clients);
	
	std::default_random_engine rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(1, 1000);  

	for (auto i = 0; i < cout_clients; ++i)
	{
		std::unique_ptr<Client> curr = std::make_unique<Client>(ip, port);

		if (Client::status::connected == curr->get_current_status())
		{
			print_mtx.lock();
			std::cout << "Client connected|id:" + std::to_string(curr->get_id()) << std::endl;
			print_mtx.unlock();
		}
		else
		{
			print_mtx.lock();
			std::cout << "Client couldn't connect|id:" + std::to_string(curr->get_id())
			 + "|Status: " + std::to_string(curr->get_current_status()) + " Error code: " + std::to_string(curr->get_last_error()) << std::endl;
			print_mtx.unlock();
		}
		
		clients.at(i) = std::move(curr);
		auto txtr = std::thread([&clients, i, &rng, &dist]()
		{
			int j = 0;
			while (Client::status::connected == clients.at(i)->get_current_status())
			{
				std::string str = "msg " + std::to_string(j++);
				int r = dist(rng);
				std::string add(r, '_');
				add += "| size = ";
				str.insert(str.size(), add);
				str.insert(str.size(), std::to_string(str.size() + (std::to_string(str.size())).size()));
				clients.at(i)->send_data(str.data(), str.size());
				std::this_thread::sleep_for(std::chrono::milliseconds(300+r*10));
			}
		});

		auto rxtr = std::thread([&clients, &print_mtx, i]()
		{
			while (Client::status::connected == clients.at(i)->get_current_status())
			{
				clients.at(i)->receive_data(print_mtx);
			}
		});
		tx_cls_threads.push_back(std::move(txtr));
		rx_cls_threads.push_back(std::move(rxtr));
	}

	for (auto i = 0; i < cout_clients; ++i)
	{
		if (tx_cls_threads[i].joinable()) tx_cls_threads[i].join();
		if (rx_cls_threads[i].joinable()) rx_cls_threads[i].join();
	}

    return 0;
}