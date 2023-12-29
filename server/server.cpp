#include "server.h"
#include "mats.pb.h"
#include "packedmsg.h"
#include "camera.h"

#include <iostream>
#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;
namespace asio = boost::asio;
using asio::ip::tcp;
using boost::uint8_t;

#define DEBUG true

// Database connection - handles a connection with a single client.
// Create only through the Connection::create factory.
class Connection : public boost::enable_shared_from_this<Connection>
{
public:
	typedef boost::shared_ptr<Connection> Pointer;
	typedef boost::shared_ptr<mats::Request> RequestPointer;
	typedef boost::shared_ptr<mats::Response> ResponsePointer;
	typedef boost::shared_ptr<mats::CheckMats> CheckMatsPointer;

	static Pointer create(asio::io_service& io_service)
	{
		return Pointer(new Connection(io_service));
	}

	tcp::socket& get_socket()
	{
		return m_socket;
	}

	void start() 
	{
		start_read_header();
	}

private:
	tcp::socket m_socket;
	std::string m_ip;
	vector<uint8_t> m_readbuf;
	PackedMessage<mats::Request> m_request;

	Connection(asio::io_service& io_service)
		: m_socket(io_service), m_ip(""),
		m_request(boost::shared_ptr<mats::Request>(new mats::Request()))
	{
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		DEBUG && (cerr << "handle read " << error.message() << '\n');
		if (!error) {
			DEBUG && (cerr << "Got header!\n");
			DEBUG && (cerr << show_hex(m_readbuf) << endl);
			unsigned msg_len = m_request.decode_header(m_readbuf);
			DEBUG && (cerr << msg_len << " bytes\n");
			start_read_body(msg_len);
		} else {
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, const_cast<boost::system::error_code&>(error));
			m_socket.close(const_cast<boost::system::error_code&>(error));
			Camera::Instance()->RemoveClient(m_ip, &m_socket);
			return;
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		DEBUG && (cerr << "handle body " << error << '\n');
		if (!error) {
			DEBUG && (cerr << "Got body!\n");
			DEBUG && (cerr << show_hex(m_readbuf) << endl);
			handle_request();
			start_read_header();
		} else {
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, const_cast<boost::system::error_code&>(error));
			m_socket.close(const_cast<boost::system::error_code&>(error));
			Camera::Instance()->RemoveClient(m_ip, &m_socket);
			return;
		}
	}

	// Called when enough data was read into m_readbuf for a complete request
	// message. 
	// Parse the request, execute it and send back a response.
	void handle_request()
	{
		if (m_request.unpack(m_readbuf)) {
			RequestPointer req = m_request.get_msg();
			ResponsePointer resp = prepare_response(req);

			/*vector<uint8_t> writebuf;
			PackedMessage<mats::Response> resp_msg(resp);

			resp_msg.pack(writebuf);
			asio::write(m_socket, asio::buffer(writebuf));*/
		}
	}

	void start_read_header()
	{
		m_readbuf.resize(HEADER_SIZE);
		asio::async_read(m_socket, asio::buffer(m_readbuf),
			boost::bind(&Connection::handle_read_header, shared_from_this(),
				asio::placeholders::error));
	}

	void start_read_body(unsigned msg_len)
	{
		// m_readbuf already contains the header in its first HEADER_SIZE
		// bytes. Expand it to fit in the body as well, and start async
		// read into the body.
		m_readbuf.resize(HEADER_SIZE + msg_len);
		asio::mutable_buffers_1 buf = asio::buffer(&m_readbuf[HEADER_SIZE], msg_len);
		asio::async_read(m_socket, buf,
			boost::bind(&Connection::handle_read_body, shared_from_this(),
				asio::placeholders::error));
	}

	ResponsePointer prepare_response(RequestPointer req)
	{
		int32_t ret = 0;
		string ip = req->ip();
		if (ip.empty()) {
			ret = -1;
		}

		start_check_mats(ip);

		ResponsePointer resp(new mats::Response());
		resp->set_result(ret);
		return resp;
	}

	void start_check_mats(string& ip)
	{
		if (m_ip == ip){
			return;
		}

		m_ip = ip;
		Camera::Instance()->AddClient(m_ip, &m_socket);
		std::cout << "camera ip:" << m_ip << std::endl;
	}
};

struct Server::ServerImpl
{
    tcp::acceptor acceptor;

    ServerImpl(asio::io_service& io_service, unsigned port)
        : acceptor(io_service, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

    void start_accept()
    {
        // Create a new connection to handle a client. Passing a reference
        // to db to each connection poses no problem since the server is 
        // single-threaded.
        Connection::Pointer new_connection = Connection::create(acceptor.get_io_service());

        // Asynchronously wait to accept a new client
        acceptor.async_accept(new_connection->get_socket(),
            boost::bind(&ServerImpl::handle_accept, this, new_connection,
                asio::placeholders::error));
    }

    void handle_accept(Connection::Pointer connection,
            const boost::system::error_code& error)
    {
        // A new client has connected
        if (!error) {
            // Start the connection
            connection->start();

            // Accept another client
            start_accept();
        }
    }
};

Server::Server(asio::io_service& io_service, unsigned port)
	: d(new ServerImpl(io_service, port)), io_(io_service), signals_(io_service)
{
	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)  
	signals_.add(SIGQUIT);
#endif  
	signals_.async_wait(boost::bind(&Server::stop, this));
}

Server::~Server()
{
}

void Server::stop() {
	io_.stop();
}



