#include "server.h"
#include "packedmessage.h"
#include "mats.pb.h"
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

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

	int32_t m_mats;
	string m_camera_ip;

    vector<uint8_t> m_readbuf;
    PackedMessage<mats::Request> m_packed_request;

    Connection(asio::io_service& io_service)
        : m_socket(io_service), m_camera_ip("localhost"), m_mats(-1),
        m_packed_request(boost::shared_ptr<mats::Request>(new mats::Request()))
    {
    }
    
    void handle_read_header(const boost::system::error_code& error)
    {
        DEBUG && (cerr << "handle read " << error.message() << '\n');
        if (!error) {
            DEBUG && (cerr << "Got header!\n");
            DEBUG && (cerr << show_hex(m_readbuf) << endl);
            unsigned msg_len = m_packed_request.decode_header(m_readbuf);
            DEBUG && (cerr << msg_len << " bytes\n");
            start_read_body(msg_len);
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
        }
    }

    // Called when enough data was read into m_readbuf for a complete request
    // message. 
    // Parse the request, execute it and send back a response.
    //
    void handle_request()
    {
        if (m_packed_request.unpack(m_readbuf)) {
            RequestPointer req = m_packed_request.get_msg();
            ResponsePointer resp = prepare_response(req);

            
            vector<uint8_t> writebuf;
            PackedMessage<mats::Response> resp_msg(resp);
            resp_msg.pack(writebuf);
            asio::write(m_socket, asio::buffer(writebuf));
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
        //
        m_readbuf.resize(HEADER_SIZE + msg_len);
        asio::mutable_buffers_1 buf = asio::buffer(&m_readbuf[HEADER_SIZE], msg_len);
        asio::async_read(m_socket, buf,
                boost::bind(&Connection::handle_read_body, shared_from_this(),
                    asio::placeholders::error));
    }

    ResponsePointer prepare_response(RequestPointer req)
    {
		int32_t ret = 0;
        switch (req->type())
        {
            case mats::Request::INIT_CAMERA:
            {
				m_camera_ip = req->init_param().ip();
				if (m_camera_ip.empty() < 1) {
					ret = -1;
				} else {
					start_check_mats();
				}

                break;
            }
            default:
                assert(0 && "Whoops, bad request!");
                break;
        }

        ResponsePointer resp(new mats::Response);
		resp->set_result(ret);
        return resp;
    }

	void start_check_mats()
	{
		boost::thread thread(boost::bind(&Connection::check_mats_thread, this));
	}

	void check_mats_fun()
	{
		
	}

	void check_mats_thread()
	{
		int32_t mats = -1;
		while (1) {
			if (mats == m_mats) {
				continue;
			}

			send_mats();
		}
	}

	void send_mats()
	{
		vector<uint8_t> writebuf;

		CheckMatsPointer resp(new mats::CheckMats);
		resp->set_mat(m_mats);

		PackedMessage<mats::CheckMats> resp_msg(resp);
		resp_msg.pack(writebuf);

		asio::write(m_socket, asio::buffer(writebuf));
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
        //
        Connection::Pointer new_connection = Connection::create(acceptor.get_io_service());

        // Asynchronously wait to accept a new client
        //
        acceptor.async_accept(new_connection->get_socket(),
            boost::bind(&ServerImpl::handle_accept, this, new_connection,
                asio::placeholders::error));
    }

    void handle_accept(Connection::Pointer connection,
            const boost::system::error_code& error)
    {
        // A new client has connected
        //
        if (!error) {
            // Start the connection
            //
            connection->start();

            // Accept another client
            //
            start_accept();
        }
    }
};

Server::Server(asio::io_service& io_service, unsigned port)
	: d(new ServerImpl(io_service, port))
{
}

Server::~Server()
{
}


