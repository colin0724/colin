#ifndef CAMERA_H
#define CAMERA_H
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include "mats.h"

namespace asio = boost::asio;
using boost::asio::ip::tcp;

class Camera {
public:
	static Camera* Instance() {
		static Camera c;
		return &c;
	}

	void AddClient(std::string& camera_ip, tcp::socket* socket);
	void RemoveClient(std::string& camera_ip, tcp::socket* socket);

private:
	Camera();
	~Camera();

	typedef std::map<std::string, Mats*> CameraMap;
	typedef CameraMap::iterator CameraMapIter;
	typedef std::pair<CameraMapIter, bool> CameraMapRet;

	typedef std::multimap<std::string, tcp::socket*> ClientMap;
	typedef ClientMap::iterator ClientMapIter;
	typedef std::pair<ClientMapIter, bool> ClientMapRet;

	boost::mutex m_mutex;
	CameraMap m_cameras;

	boost::mutex m_cmutex;
	ClientMap m_clients;

	static void ChangeMats(std::string& ip, int mats, void* param);
	void ChangeMatsEx(std::string& ip, int mats);
	void Publish(tcp::socket* socket, int mats);
	void AddCamera(std::string& ip);
};

#endif // !CAMERA_H
