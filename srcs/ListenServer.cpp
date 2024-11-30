#include <ListenServer.hpp>
#include <Host.hpp>
#include <Client.hpp>
#include <Request.hpp>
#include <ILogger.hpp>
#include <UniqueValuesMapIterator.tpp>

//TODO : Not so sure how to handle socket creation and listening errors ?
std::list<ListenServer>	ListenServer::_serverList;

ListenServer::ListenServer() : _sockFd(0), _maxClientNbr(MAX_CLIENT_NBR), _nbrHost(0) {}

ListenServer::ListenServer(const ListenServer& copy) : _orphanClients(copy._orphanClients), \
	_connectedClients(copy._connectedClients), _sockFd(copy._sockFd), _hostMap(copy._hostMap), \
	_ip(copy._ip), _port(copy._port), _maxClientNbr(copy._maxClientNbr), _nbrHost(copy._nbrHost) {}

ListenServer::ListenServer(std::string const &hostAddr, std::string const &hostPort):  _sockFd(0), _ip(hostAddr), _port(hostPort), _maxClientNbr(MAX_CLIENT_NBR), _nbrHost(0) {}

ListenServer::~ListenServer() {}

/// @brief Add host server_names as keys in server's ```_hostMap``` if they are
/// not already present.
/// @param host
void	ListenServer::assignHost(Host *host)
{
	for (std::vector<std::string>::const_iterator it = host->getServerNames().begin();\
			it != host->getServerNames().end(); it++)
	{
		if (_hostMap.find(*it) == _hostMap.end())
			_hostMap[*it] = host;
	}
	this->_nbrHost++;
}

/// @brief for each listen port of the host, either assign the host
/// to existing ListenServer(s) or create a new server with a socket
/// @return ```1``` if at least one server failed to launch
int	ListenServer::registerHost(Host *host)
{
	int res = 0;

	for (std::set<std::string>::const_iterator it = host->getPorts().begin();
			it != host->getPorts().end(); it++) {
		if (registerHost(host, *it))
			res = 1;
	}
	return (res);
}

/// @brief Either assign the host to a listenServer if one is already linked
/// to ```port```, or create a new listenServer and add the host to it.
/// @param host 
/// @param port 
/// @return 
int	ListenServer::registerHost(Host* host, const std::string& port) {
	std::list<ListenServer>::iterator	it = findServer(host->getAddr(), port);
	if (it != _serverList.end()) {
		it->assignHost(host);
		host->addListenServer(&*it);
	}
	else
	{
		ListenServer	*new_server = addServer(host->getAddr(), port);
		if (new_server == NULL)
			return (1);
		new_server->assignHost(host);
		host->addListenServer(new_server);
	}
	return (0);
}

bool	ListenServer::serverExist(const std::string& hostAddr, const std::string& hostPort)
{
	if (findServer(hostAddr, hostPort) != _serverList.end())
		return (true);
	return (false);
}

std::list<ListenServer>::iterator	ListenServer::findServer(const std::string& hostAddr, const std::string& hostPort)
{
	std::list<ListenServer>::iterator	it;

	for (it = _serverList.begin(); it != _serverList.end(); it++)
	{
		if ((it->_ip == hostAddr) && (it->_port == hostPort))
			break;
	}
	return (it);
}

//Create a new ListenServer and a new socket listening to the given port, add the server to the static list
ListenServer*	ListenServer::addServer(const std::string& hostAddr, const std::string& hostPort)
{
	int				new_socket = 0, val = 1;
	struct	addrinfo *res = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(NULL, hostPort.c_str(), &hints, &res))
		return (NULL);
	new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (new_socket < 0) {
		freeaddrinfo(res);
		return (NULL);
	}
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (bind(new_socket, res->ai_addr, res->ai_addrlen))
	{
		LOGE("Could not create a socket for host: %ss and port %ss\n", &hostAddr, &hostPort);
		close(new_socket);
		freeaddrinfo(res);
		return (NULL);
	}
	ListenServer	serverTmp(hostAddr, hostPort);
	serverTmp._sockFd = new_socket;
	ListenServer*	newServer = &*_serverList.insert(_serverList.end(), serverTmp);
	newServer->_sockFd = new_socket;
	freeaddrinfo(res);
	return (newServer);
}

void	ListenServer::removeServer(const std::string& hostAddr, const std::string& hostPort)
{
	std::list<ListenServer>::iterator	it = findServer(hostAddr, hostPort);
	if (it != _serverList.end())
		removeServer(it);
}

void	ListenServer::removeServer(std::list<ListenServer>::iterator it)
{
	if (it == _serverList.end())
		return;
	if (it->_sockFd)
		it->shutdown();
	for (std::list<Client*>::iterator clientIt = it->_orphanClients.begin(); clientIt != it->_orphanClients.end();)
	{
		(*clientIt++)->terminate();
		// it->_orphanClients.remove(*clientIt++);
	}
	for (std::list<Client*>::iterator clientIt = it->_connectedClients.begin(); clientIt != it->_connectedClients.end();)
	{
		(*clientIt++)->terminate();
		// it->_connectedClients.remove(*clientIt++);
	}
	for (UniqueValuesMapIterator<std::string, Host*> hostIt = it->_hostMap.begin(); hostIt != it->_hostMap.end();)
	{
		it->unassignHost((hostIt.safePostIncrement(it->_hostMap.end()))->second);
	}
	_serverList.erase(it);
}

/// @brief Remove an host with given name in the server.
/// @param serverName 
/// @return ```0``` if the host was deleted, ```1``` if it wasn't found.
int	ListenServer::removeHost(const std::string& serverName) {
	std::map<std::string, Host*>::iterator	it = _hostMap.find(serverName);
	if (_hostMap.end() == it)
		return (1);
	unregisterHost(it->second, this->_port);
	return (0);
}

Host*	ListenServer::findHost(const std::string& serverName) {
	std::map<std::string, Host*>::iterator	it = _hostMap.find(serverName);
	if (_hostMap.end() == it)
		return (NULL);
	return (it->second);
}

/// @brief Remove the host from the listen server(s) it belongs to if any.
/// After the deletion, if a listen server doesn't refer to any host anymore,
/// it is terminated and deleted.
/// @param host
void	ListenServer::unregisterHost(Host* host) {
	unregisterHost(host, host->getListenServers());
}

/// Given a list of valid ListenServer pointer, unregister to host from all the server
/// of this list, resulting empty servers are removed.
void	ListenServer::unregisterHost(Host* host, const std::set<ListenServer*> serverSet) {
	for (std::set<ListenServer*>::const_iterator it = serverSet.begin(); it != serverSet.end();) {
		unregisterHost(host, (*it++)->_port);
	}
}

/// @brief Given a port and host address, remove the host from the listenServer
/// associated with this port.
/// @param host 
/// @param port 
void	ListenServer::unregisterHost(Host* host, const std::string& port) {
	std::list<ListenServer>::iterator it = findServer(host->getAddr(), port);
	unregisterHost(host, it);
}

/// @brief Given a port and host address, remove the host from the listenServer
/// associated with this port.
/// @param host 
/// @param port 
void	ListenServer::unregisterHost(Host* host, std::list<ListenServer>::iterator serverPos) {
	if (serverPos == _serverList.end())
		return;
	serverPos->unassignHost(host);
	if (serverPos->_hostMap.size() == 0)
		removeServer(serverPos);
	if (host->getListenServers().size() == 0)
		Host::removeHost(host);
}

/// @brief Check if the servernames are already present in the listen server
/// associated with given adress and listening ports
/// @param host 
/// @return ```false``` if there is a conflict
bool	ListenServer::checkServerNames(const std::string& hostAddr, const std::set<std::string>& ports, const std::vector<std::string>& serverNames) {
	for (std::set<std::string>::const_iterator itPort = ports.begin(); itPort != ports.end(); itPort++) {
		std::list<ListenServer>::iterator serverPos = findServer(hostAddr, *itPort);
		if (serverPos == _serverList.end())
			continue;
		for (std::vector<std::string>::const_iterator itName = serverNames.begin(); itName != serverNames.end(); itName++) {
			if (serverPos->findHost(*itName) != NULL)
				return (false);
		}
	}
	return (true);
}

/// @brief Remove a client pointer from the list of orphan and/or connected clients.
/// @param client 
void	ListenServer::removeClient(Client* client) {
	_orphanClients.remove(client);
	_connectedClients.remove(client);
}

/// @brief Unregister the host names from ```_hostMap```.
/// @param host
void	ListenServer::unassignHost(Host* host) {
	std::map<std::string, Host*>::iterator	mapIt;
	mapIt = _hostMap.find(host->getServerNames().at(0));
	if (mapIt == _hostMap.end())
		return;
	for (std::list<Client*>::const_iterator clientIt = mapIt->second->getClientListBegin();\
			clientIt != mapIt->second->getClientListEnd(); clientIt++)
		_connectedClients.remove(*clientIt);
	for (std::vector<std::string>::const_iterator nameIt = host->getServerNames().begin();\
			nameIt != host->getServerNames().end(); nameIt++)
	{
		mapIt = _hostMap.find(*nameIt);
		if (mapIt != _hostMap.end())
			_hostMap.erase(mapIt);
	}
	this->_nbrHost--;
	host->removeListenServer(this);
	LOGI("Host (%ss) was unassigned from %ss:%ss", &host->getServerNames().at(0), &this->_ip, &this->_port);
}

// add the socket to the epoll interest list with a pointer to ListenServer in event.data. Then tells the socket to listen.
int	ListenServer::registerToEpoll(int epollfd)
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	event.data.ptr = this;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->_sockFd, &event))
	{
		close(this->_sockFd);
		return (-1);
	}
	if (listen(this->_sockFd, 4096))
	{
		close(this->_sockFd);
		return (-1);
	}
	return (0);
}

int	ListenServer::getNbrConnectedClients(void) const {
	return (this->_connectedClients.size());
}

void	ListenServer::shutdown()
{
	close(this->_sockFd);
	this->_sockFd = 0;
}

// Start all the servers using the start function.
int	ListenServer::startServers(int epollfd)
{
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end(); it++)
	{
		if (it->registerToEpoll(epollfd))
		{
			_serverList.erase(it);
			LOGE("Could not listen on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
		}
		else {
			LOGI("Listening on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
			LOGI("%Ls", &*it);
		}
	}
	return (0);
}

//Not sure it is needed since the destructor should close the socketfd anyway
void	ListenServer::closeServers()
{
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end(); it++){
		it->shutdown();
	}
}

void	ListenServer::removeServers()
{
	LOGI("Stopping servers...");
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end();)
	{
		removeServer(it++);
	}
}

int	ListenServer::getNbrServer(void) {
	return (_serverList.size());
}

int ListenServer::getNbrHost(void) const {
	return (this->_nbrHost);
}

std::list<ListenServer>::const_iterator	ListenServer::getServerListBegin(void) {
	return (_serverList.begin());
}

std::list<ListenServer>::const_iterator	ListenServer::getServerListEnd(void) {
	return (_serverList.end());
}

/// @brief Accept an incoming connection, create a client and add it to the
/// server's orphan list. MUST follows an epoll event insuring that the call
/// to ```accept()``` will not block. Client will then have to be assign externally
/// to its correct host when header parsing is done.
/// @param
/// @return ```NULL``` if no client could be initiated or if connection
/// socket could not be created.
Client*	ListenServer::acceptConnection(void) {
	if (getNbrConnectedClients() >= _maxClientNbr) {
		close(accept(_sockFd, NULL, NULL));
		LOGE("A connection was refused by server (%ss:%ss) because the maximum" \
			" number of allowed clients was reached.", &_ip, &_port);
		return (NULL);
	}
	ClientSocket	socket;
	socket.addrSize = sizeof(socket.addr);
	socket.fd = accept(_sockFd, &socket.addr, &socket.addrSize);
	if (socket.fd < 0) {
		LOGE("Server %ss:%ss failed to accept a new client.", &_ip, &_port);
		return (NULL);
	}
	Client*	newClient = Client::newClient(socket, *this);
	if (newClient == NULL) {
		LOGE("Unexpected error when creating new client");
		close(socket.fd);
		return (NULL);
	}
	_connectedClients.push_back(newClient);
	_orphanClients.push_back(newClient);
	LOGI("A new client (%ss:%d, fd = %d) connected to server %ss:%ss", &newClient->getStrAddr(), newClient->getAddrPort(), newClient->getfd(), &_ip, &_port);
	return (newClient);
}

/// @brief Try to bind an orphan client to an host using parsed host header.
/// If the host is not referred by the listen server, it will try to bind the client
/// to the first ```server_name``` of the list. hostName should not be empty as
/// the empty ```host``` header situation should be handled by ```IControl```.
/// Client host is not set.
/// @param hostName
/// @return ```NULL``` if given hostName was not found in the host list of the listen server.
Host*	ListenServer::bindClient(Client& client, const std::string& hostName)
{
	std::map<std::string, Host*>::iterator	it;

	it = _hostMap.find(hostName);
	if (it == _hostMap.end())
		it = _hostMap.begin();
	it->second->addClient(&client);
	_orphanClients.remove(&client);
	return (it->second);
}

std::ostream&	ListenServer::printShort(std::ostream& os) const {
	os << "	->  ip/port: " << this->_ip << '/' << this->_port << std::endl;
	os << "	sock_fd: " << this->_sockFd << std::endl;
	os << "	number of connected clients (orphan): " << this->_connectedClients.size() \
		<< " (" << this->_orphanClients.size() << ")\n";
	os << "	host list (" << this->getNbrHost() << "):\n";
	for (UniqueValuesMapIterator_const<std::string, Host*> it(this->_hostMap.begin()); it != this->_hostMap.end(); it.safePostIncrement(this->_hostMap.end()))
		it->second->printShort(os);
	return (os);
}

std::ostream&	ListenServer::printFull(std::ostream& os) const {
	os << "	->  ip/port: " << this->_ip << '/' << this->_port << std::endl;
	os << "	sock_fd: " << this->_sockFd << std::endl;
	os << "	number of connected clients (orphan): " << this->_connectedClients.size() \
		<< " (" << this->_orphanClients.size() << ")\n";
	os << "	host list (" << this->getNbrHost() << "):\n";
	for (UniqueValuesMapIterator_const<std::string, Host*> it(this->_hostMap.begin()); it != this->_hostMap.end(); it++)
		it->second->printFull(os);
	return (os);
}

std::ostream&	operator<<(std::ostream& os, const ListenServer& ls) {
	ls.printShort(os);
	return (os);
}
