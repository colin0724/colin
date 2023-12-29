#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>

class Server 
{
public:
	Server(boost::asio::io_service& io_service, unsigned port);
    ~Server();

private:
    struct ServerImpl;
	boost::asio::io_service& io_;
	boost::asio::signal_set signals_;
    boost::scoped_ptr<ServerImpl> d;

	void stop();
};

#endif /* SERVER_H */

