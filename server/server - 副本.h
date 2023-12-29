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
	//Server();
    void start_accept();

    struct ServerImpl;
    boost::scoped_ptr<ServerImpl> d;
};

#endif /* SERVER_H */

