#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <vector>

#include "mats.pb.h"
#include "packedmessage.h"


using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::endpoint end_point(boost::asio::ip::address::from_string("127.0.0.1"), 4050);
		tcp::socket socket(io_service);
		socket.connect(end_point);
		std::string data;

		typedef boost::shared_ptr<mats::Request> RequestPointer;
		typedef boost::shared_ptr<mats::Response> ResponsePointer;
		typedef boost::shared_ptr<mats::CheckMats> CheckMatsPointer;


		RequestPointer req(new mats::Request);
		req->set_ip("127.0.0.127");

		std::vector<uint8_t> writebuf;
		PackedMessage<mats::Request> req_msg(req);

		req_msg.pack(writebuf);
		boost::asio::write(socket, boost::asio::buffer(writebuf));

		for (;;)
		{
			boost::array<char, 128> buf;
			boost::system::error_code error;

			size_t len = socket.read_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
			{
				break; // Connection closed cleanly by peer.
			}
			else if (error)
				throw boost::system::system_error(error); // Some other error.

			data.append(buf.begin(), buf.end());
			std::cout << data << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}