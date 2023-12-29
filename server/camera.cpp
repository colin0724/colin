#include "camera.h"
#include "mats.pb.h"
#include "packedmsg.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;

Camera::Camera()
{
}

Camera::~Camera()
{
	m_clients.clear();

	CameraMapIter iter = m_cameras.begin();
	while (iter != m_cameras.end())
	{
		if (!iter->second) {
			Mats* m = (Mats*)iter->second;
			delete m;
			m = 0;
		}
	}

	m_cameras.clear();
}

void Camera::AddClient(string& camera_ip, tcp::socket* socket)
{
	boost::mutex::scoped_lock lock(m_cmutex);
	m_clients.insert(make_pair(camera_ip, socket));

	AddCamera(camera_ip);
}

void Camera::AddCamera(string& ip) {
	boost::mutex::scoped_lock lock(m_mutex);
	CameraMapIter iter = m_cameras.find(ip);
	if (iter != m_cameras.end()) {
		return;
	}

	Mats* m = new Mats(ip, ChangeMats, (void*)this);
	m_cameras.insert(make_pair(ip, m));
}

void Camera::RemoveClient(string& camera_ip, tcp::socket* socket)
{
	boost::mutex::scoped_lock lock(m_cmutex);
	ClientMapIter pos = m_clients.lower_bound(camera_ip);
	for (; pos != m_clients.upper_bound(camera_ip); ++pos) {
		if (pos->second == socket) {
			m_clients.erase(pos);
			return;
		}
	}
}

void Camera::ChangeMats(string& ip, int mats, void* param)
{
	if (!param) return;

	Camera* c = (Camera*)param;
	c->ChangeMatsEx(ip, mats);
}

void Camera::ChangeMatsEx(string& ip, int mats)
{
	boost::mutex::scoped_lock lock(m_cmutex);
	ClientMapIter pos = m_clients.lower_bound(ip);
	for (; pos != m_clients.upper_bound(ip); ++pos) {
		Publish(pos->second, mats);
	}
}

void Camera::Publish(tcp::socket* socket, int mats) {
	if (!socket) {
		return;
	}

	boost::shared_ptr<mats::CheckMats> resp(new mats::CheckMats);
	resp->set_mat(mats);

	vector<uint8_t> writebuf;
	PackedMessage<mats::CheckMats> resp_msg(resp);

	resp_msg.pack(writebuf);
	asio::write(*socket, asio::buffer(writebuf));
}