#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "server.h"

namespace asio = boost::asio;

int main(int argc, const char* argv[])
{
	try {
		asio::io_service io_service;
		Server server(io_service, 4050);
		io_service.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}