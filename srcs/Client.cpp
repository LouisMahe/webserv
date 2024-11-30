#include <Client.hpp>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <ctime>
#include <arpa/inet.h>
#include <ILogger.hpp>
#include <CGIProcess.hpp>
#include <IControl.hpp>

std::list<Client>	Client::_clientList;

Client::~Client(void) {
	if (_request)
		delete(_request);
	if (_response)
		delete(_response);
	if (_bodyStream) {
		_bodyStream->close();
		delete _bodyStream;
	}

	// !!!! bodystream allocated
}

Client::Client(const ClientSocket& socket, ListenServer& listenServer) \
	: _socket(socket), _host(NULL), _listenServer(listenServer), _headerStatus(HEADER_STATUS_ONGOING), \
	_bodyStatus(BODY_STATUS_NONE), _mode(CLIENT_MODE_READ)
{
	_port = 0;
	gettimeofday(&_lastInteraction, NULL);
	_request = NULL;
	_response = NULL;
	_bodyStream = NULL;
	cgiProcess = NULL;
}

//!!! bodystream copy not allocated is it ok ?
Client::Client(const Client& copy) : _socket(copy._socket), _addressStr(copy._addressStr), \
	_port(copy._port), _host(copy._host), _listenServer(copy._listenServer), _request(copy._request), \
	_response(copy._response), _lastInteraction(copy._lastInteraction), \
	_headerStatus(copy._headerStatus), _bodyStatus(copy._bodyStatus), _mode(copy._mode), _buffer(copy._buffer), \
	_bodyFileName(copy._bodyFileName), _bodyStream(copy._bodyStream), cgiProcess(copy.cgiProcess)
{}

int	Client::getTotalNbrClient(void) {
	return (_clientList.size());
}

void	Client::clearBuffers(void) {
	while (!outBuffers.empty()) {
		outBuffers.pop();
	}
}

std::list<Client>::iterator	Client::findClient(Client* client)
{
	std::list<Client>::iterator pos;
	for (pos = _clientList.begin(); pos != _clientList.end(); pos++)
	{
		if (client == &*pos)
			return (pos);
	}
	return (pos);
}

/// @brief Initialize a new client based on given socket.
/// Given socket must refer to a valid socket that was previously
/// returned via a call to ```accept()```.
/// @param socket
/// @return Pointer toward the client object that was created.
/// ```NULL``` should means an unexpected (and perhaps fatal) error
/// occured.
Client*	Client::newClient(const ClientSocket& socket, ListenServer& listenServer)
{
	char	strIp[64];
	Client	clientTmp(socket, listenServer);

	Client& client = *_clientList.insert(_clientList.end(), clientTmp);
	inet_ntop(AF_INET, &(((struct sockaddr_in*)&client._socket.addr)->sin_addr),
					strIp, 64);
	client._port = htons((((struct sockaddr_in *)&client._socket.addr.sa_data)->sin_port));
	client._addressStr += strIp;
	return (&client);
}

/// @brief Permanently delete a client.
/// @warning Will invalidate every reference to this client.
/// This operation should be done by the client's host
/// or the listenServer for orphan client and simultaneously with deleting
/// every reference to the client.
/// @param
void	Client::deleteClient(Client* client)
{
	if (client == NULL)
		return;
	std::list<Client>::iterator pos = findClient(client);
	if (pos != _clientList.end())
		_clientList.erase(pos);
}

int	Client::getfd(void) const {
	return (this->_socket.fd);
}

const Host*	Client::getHost(void) const {
	return (this->_host);
}

const std::string&	Client::getStrAddr(void) const {
	return (this->_addressStr);
}

int	Client::getAddrPort(void) const {
	return (this->_port);
}

/// @brief Return the status of the request at the front of the queue.
/// @return ```-1``` if no request in the queue.
int	Client::getRequestStatus() const {
	if (this->_request != NULL)
		return (this->_request->getStatus());
	return (-1);
}

int	Client::getResponseStatus() const {
	// return (_response.getStatus());
	return (0);
}

void	Client::setHost(const std::string& hostname) {
	if (_host) {
		if (_host->checkServerName(hostname))
			return;
		_host->removeClient(this);
	}
	this->_host = this->_listenServer.bindClient(*this, hostname);
}

/// @brief
/// @param
void	Client::shutdownConnection(void) {
	close(_socket.fd);
}


Request*	Client::getRequest()
{
	if (_request == NULL)
	{
		updateLastInteraction();
		_request = new Request();
	}
	return (_request);
}



void	Client::stashBuffer(std::string &str)
{
	this->_buffer += str;
}

void	Client::retrieveBuffer(std::string &str)
{
	str += this->_buffer;
	this->_buffer.clear();
}

void	Client::clearBuffer()
{
	this->_buffer.clear();
}

int	Client::parseRequest(const char* bufferIn, int n_read) {
	_buffer += std::string(bufferIn, n_read);
	Request*	request = getRequest();
	int			res = 0;

	LOGI("fullBuffer: %ss", &_buffer);
	if (_headerStatus < HEADER_STATUS_READY)
	{
		res = request->parseHeaders(_buffer);
		if (res < 0)
			_headerStatus = HEADER_STATUS_READY;
		return (res);
	}
	else if (_headerStatus == HEADER_STATUS_DONE && _bodyStatus == ONGOING)
	{
		res = request->parseInput(_buffer, _bodyStream);
		if (res < 0)
			_bodyStatus = BODY_STATUS_DONE;
		return(res);
	}
	else {
		LOGI("header status = %d, body_status = %d", _headerStatus, _bodyStatus);
		return (0);
	}




	// while (!fullBuffer.empty() && request->getStatus() < 4)
	// {
	// 	res = request->parseInput(fullBuffer);
	// 	// if (res < 0 && (AssignHost(client) || req_ptr->getLenInfo()))
	// 	// {
	// 	// 	req_ptr->_fillError(400, "Host header missing or invalid");
	// 	// 	req_ptr->setStatus(COMPLETE);
	// 	// 	client->setStatus(CLIENT_MODE_ERROR);
	// 	// 	fullBuffer.clear();
	// 	// 	break ;
	// 	// }
	// 	if (res > 0)
	// 	{

	// 		fullBuffer.clear();
	// 		break ;
	// 	}
	// }
	// if (request->getStatus() == COMPLETE && _status != CLIENT_MODE_ERROR)
	// {
	// 	stashBuffer(fullBuffer);
	// 	req_ptr = client->getRequest();
	// 	req_ptr->printHeaders();
	// 	LOGE("Request status: %d | Client status: %d", client->getRequestStatus(), client->getStatus());
	// }
}

static long long	getDuration(struct timeval time)
{
	struct timeval	now;
	gettimeofday(&now, NULL);

	return (now.tv_sec * 1000 + now.tv_usec / 1000 - time.tv_sec * 1000 - time.tv_usec / 1000);
}

std::ostream&	operator<<(std::ostream& os, const Client& client) {
	os << "		->  fd: " << client._socket.fd << std::endl;
	os << "		ip/port: " << client._addressStr << ':' << client._port << std::endl;
	os << "		host: " << client._host;
	if (client._host)
		os << " (" << client._host->getServerNames().at(0) << ')';
	os << std::endl;
	if (client._request)
		os << " | status = " << client._request->getStatus() << " | error = " \
			<< client._request->getError();
	os << std::endl;
	os << "		last interaction: " << getDuration(client._lastInteraction) << " ms" << std::endl;
	os << "		nbr of out buffers: " << client.outBuffers.size() << std::endl;
	return (os);
}


int	Client::getHeaderStatus() const
{
	return (this->_headerStatus);
}

void	Client::setHeaderStatus(int status) {
	this->_headerStatus = status;
}

int	Client::getBodyStatus() const
{
	return (this->_bodyStatus);
}

void	Client::setBodyStatus(int status) {
	this->_bodyStatus = status;
}


int	Client::getMode() const
{
	return (this->_mode);
}

void	Client::setMode(int mode) {
	this->_mode = mode;	
}

const std::string&	Client::getBodyFile() const
{
	return (this->_bodyFileName);
}

void	Client::setBodyFile(const std::string& path)
{
	if (path != "")
	{
		_bodyStream = new std::ofstream(path.c_str(), std::ios_base::trunc);
		_bodyFileName = path;
		this->_request->resHints.bodyFileName = path;
	}
	else
	{
		_bodyStream = NULL;
		_bodyFileName = "";
		this->_request->resHints.bodyFileName = path;
	}
}

AResponse*	Client::getResponse() {
	return (this->_response);
}

void	Client::setResponse(AResponse* response) {
	this->_response = response;
}

/// @brief Delete body stream and associated file 
void	Client::deleteBodyStream()
{
	if (_bodyStream) {
		delete _bodyStream;
		_bodyStream = NULL;
		if (_request && (_request->resHints.unlink == true || _request->resHints.status >= 300))
			unlink(_bodyFileName.c_str());
	}
}

void	Client::checkTO()
{
	for (std::list<Client>::iterator it = _clientList.begin(); it != _clientList.end();)
	{
		if (it->getMode() == CLIENT_MODE_READ && getDuration(it->_lastInteraction) > CLIENT_TIME_OUT)
		{
			if (it->_request == NULL) {
				it->terminate();
				return;
			}
			it->setMode(CLIENT_MODE_WRITE);
			it->_request->setStatus(COMPLETE);
			it->_request->_fillError(408, "");
			IControl::generateResponse(*it++, 408);
		}
		else
			it++;
	}
}

void	Client::updateLastInteraction()
{
	gettimeofday(&_lastInteraction, NULL);
}

void	Client::deleteCGIProcess() {
	if (cgiProcess) {
		delete cgiProcess;
		cgiProcess = NULL;
	}
}

void	Client::clearResponse(void) {
	if (_response) {
		delete _response;
		_response = NULL;
	}
}

void	Client::clear() {
	deleteBodyStream();
	deleteCGIProcess();
	if (_request) {
		delete _request;
		_request = NULL;
	}
	clearResponse();
	_bodyFileName = "";
	_bodyStatus = BODY_STATUS_NONE;
	_headerStatus = HEADER_STATUS_ONGOING;
}

void	Client::terminate(void) {
	LOGD("terminating client (%d)", this->getfd());
	if (cgiProcess && cgiProcess->getPID() > 0) {
		kill(cgiProcess->getPID(), SIGKILL);
	}
	IControl::removeFromEpoll(_socket.fd);
	clear();
	if (_host) {
		_host->removeClient(this);
	}
	_listenServer.removeClient(this);
	close(_socket.fd);
	Client::deleteClient(this);
}
