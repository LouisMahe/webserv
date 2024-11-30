#include <Host.hpp>
#include <sstream>
#include <algorithm>
#include <ILogger.hpp>
#include <limits.h>

std::list<Host>			Host::_hostList;
std::list<CGIConfig>	Host::_cgis;
std::list<Location>		Host::_locations;

Host::~Host(void) {}

Host::Host(void): _addr(DFT_VALUE_ADDR), _body_max_size(DFT_VALUE_MAX_BODY_SIZE) {}

Host&	Host::operator=(Host& rhs) {
	(void) rhs;
	LOGE("Undefined assigment operator for host class");
	return (*this);
}

Host::Host(const Host& copy) : _addr(copy._addr), _ports(copy._ports), \
	_body_max_size(copy._body_max_size), _dir_errors(copy._dir_errors), \
	_server_names(copy._server_names), _locationMap(copy._locationMap), \
	_cgiMap(copy._cgiMap), _clients(copy._clients), _listenServers(copy._listenServers) {}

Location::Location(void) : methods(DFT_VALUE_METHOD), dir_listing(false), upload(false), redir(0) {}

Location::Location(const Location& copy) : root(copy.root), methods(copy.methods), dir_listing(copy.dir_listing), \
	default_uri(copy.default_uri), upload(copy.upload), upload_root(copy.upload_root), \
	redir(copy.redir), addr_redir(copy.addr_redir)
{}

CGIConfig::CGIConfig(void) : methods(GET) {}

CGIConfig::CGIConfig(const CGIConfig& copy) : exec(copy.exec), root(copy.root), \
methods(copy.methods), extension(copy.extension)
{}

bool	CGIConfig::operator==(const CGIConfig& rhs) const {
	if (this->exec != rhs.exec
		|| this->root != rhs.root
		|| this->methods != rhs.methods
		|| this->extension != rhs.extension)
		return (false);
	return (true);
}

// 

bool	Location::operator==(const Location& rhs) const {
	if (this->addr_redir.size() != rhs.addr_redir.size()
		|| !std::equal(this->addr_redir.begin(), this->addr_redir.end(), rhs.addr_redir.begin())
		|| this->redir != rhs.redir
		|| this->default_uri != rhs.default_uri
		|| this->dir_listing != rhs.dir_listing
		|| this->methods != rhs.methods
		|| this->upload != rhs.upload
		|| this->upload_root != rhs.upload_root
		|| this->root != rhs.root)
		return (false);
	return (true);
}

std::list<Host>::iterator	Host::findHost(Host* host)
{
	std::list<Host>::iterator pos;
	for (pos = _hostList.begin(); pos != _hostList.end(); pos++)
	{
		if (host == &*pos)
			return (pos);
	}
	return (pos);
}

/// @brief Append a h
/// @param host
/// @brief Append a host to the list of host and call
/// addHost on ListenServer
/// @param host
void	Host::addHost(Host& host) {
	Host& newHost = *_hostList.insert(_hostList.end(), host);
	ListenServer::registerHost(&newHost);
}

void	Host::removeHost(Host* host) {
	if (host == NULL)
		return;
	host->shutdown();
	std::list<Host>::iterator pos = findHost(host);
	if (pos != _hostList.end())
		_hostList.erase(pos);
}

void	Host::addListenServer(ListenServer* server) {
	if (server)
		_listenServers.insert(server);
}

void	Host::removeListenServer(ListenServer* server) {
	if (server)
		_listenServers.erase(server);
}

const std::set<std::string>&	Host::getPorts(void) const {
	return (this->_ports);
}

const std::set<ListenServer*>&	Host::getListenServers(void) const {
	return (this->_listenServers);
}

int	Host::getMaxSize(void) const {
	return (this->_body_max_size);
}

const std::string&	Host::getAddr(void) const {
	return (this->_addr);
}

Client*	Host::getClientByFd(int fd) const {
	(void) fd;
	return (NULL);
}

const Location*	Host::getLocation(const std::string& uri) const {
	return (getMapObjectByKey(_locationMap, uri));
}

const CGIConfig*	Host::getCGIConfig(const std::string& uri) const {
	return (getMapObjectByKey(_cgiMap, uri));
}

const Location*	Host::matchLocation(const std::string& uri) const {
	return (getObjectMatch(_locationMap, uri));
}

const CGIConfig*	Host::matchCGIConfig(const std::string& uri) const {
	return (getObjectMatch(_cgiMap, uri));
}


const std::vector<std::string>&	Host::getServerNames(void) const {
	return (this->_server_names);
}

std::list<Client*>::const_iterator	Host::getClientListBegin(void) const {
	return (_clients.begin());
}

std::list<Client*>::const_iterator	Host::getClientListEnd(void) const {
	return (_clients.end());
}

/// @brief Check if a given serverName match with the host
/// @param name 
/// @return 
bool	Host::checkServerName(const std::string& name) const {
	std::vector<std::string>::const_iterator pos;
	pos = std::find(_server_names.begin(), _server_names.end(), name);
	if (pos == _server_names.end())
		return (false);
	return (true);
}

const std::string&	Host::getDirErrorPages(void) const {
	return (this->_dir_errors);
}

/// @brief Remove the host pointer from client list
/// Memory safe.
/// @param client 
void	Host::removeClient(Client* client) {
	if (client)
		this->_clients.remove(client);
}

void	Host::addClient(Client* newClient) {
	if (newClient)
		this->_clients.push_back(newClient);
}

/// @brief Terminate and close connection with every clients belonging to the host.
/// @param
void	Host::shutdown(void) {
	for (std::list<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
		(*it)->terminate();
}

void	Host::addCGIConfig(const std::deque<std::string>& names, CGIConfig& cgiConfig)
{
	std::list<CGIConfig>::iterator pos;

	pos = std::find(_cgis.begin(), _cgis.end(), cgiConfig);
	if (pos == _cgis.end())
		pos = _cgis.insert(pos, cgiConfig);
	for (std::deque<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
		_cgiMap[*it] = &*pos;
}

void	Host::addLocation(const std::deque<std::string>& names, Location& location)
{
	std::list<Location>::iterator pos;

	pos = std::find(_locations.begin(), _locations.end(), location);
	if (pos == _locations.end())
		pos = _locations.insert(pos, location);
	for (std::deque<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
		_locationMap[*it] = &*pos;
}

void	Host::printProperties(std::ostream& os) const
{
	os << "		->  Names :";
	for (std::vector<std::string>::const_iterator it = _server_names.begin(); \
		it != _server_names.end(); it++)
		os << " " << *it;
	os << std::endl;
	os << "		Max body size : " << _body_max_size << std::endl;
	os << "		Error dir : " << _dir_errors << std::endl;

}

void	Host::printShort(std::ostream& os) const
{
	printProperties(os);
	os << "		Locations :";
	for (std::map<std::string, Location*>::const_iterator it = _locationMap.begin();\
			it != _locationMap.end(); it++)
		os << " " << it->first;
	os << std::endl;
	os << "		CGIs :";
	for (std::map<std::string, CGIConfig*>::const_iterator it = _cgiMap.begin();\
			it != _cgiMap.end(); it++)
		os << " " << it->first;
	os << std::endl;
	os << "		Clients (" << _clients.size() << ") :";
	// for (std::list<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); it++)
		// os << " " << (*it ? (**it).getfd() : -2);
	os << std::endl;
}

void	Host::printFull(std::ostream& os) const
{
	printProperties(os);
	os << "		Locations :" << std::endl;
	for (std::map<std::string, Location*>::const_iterator it = _locationMap.begin();\
			it != _locationMap.end(); it++)
		os << "	  ->" << it->first << std::endl << it->second;
	os << "		CGIs :";
	for (std::map<std::string, CGIConfig*>::const_iterator it = _cgiMap.begin();\
			it != _cgiMap.end(); it++)
		os << "		  ->" << it->first << std::endl << it->second;
	os << "		Clients (" << _clients.size() << ") :" << std::endl;
	// for (std::list<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); it++)
	// {
		// if (*it)
			// os << "	  ->" << (**it).getfd() << std::endl << **it;
		// else
			// os << "	  => (null)" << std::endl;
	// }
	os << std::endl;
}

std::ostream&	operator<<(std::ostream& os, const Location& location)
{
	os << "			->  root: " << location.root << std::endl;
	os << "			default_uri: " << location.default_uri << std::endl;
	os << "			dir_listing: " << location.dir_listing << std::endl;
	os << "			upload: " << location.upload << std::endl;
	if (location.upload)
		os << "			upload_root: " << location.upload_root << std::endl;
	os << "			allowed methods: ";
	for (int i = 0; i < METHOD_NBR; i++)
		os << ((location.methods & (1 << i)) ? METHOD_STR[i] : "");
	os << std::endl;
	if (location.redir) {
		os << "			redirections (" <<  location.redir << "): \n";
		for (std::vector<std::string>::const_iterator it = location.addr_redir.begin();\
				it != location.addr_redir.end(); it++)
			os << "			  - " << *it << std::endl;
	}
	return (os);
}

std::ostream&	operator<<(std::ostream& os, const CGIConfig& CGIConfig)
{
	os << "			->  root: " << CGIConfig.root << std::endl;
	os << "			exec_path: " << CGIConfig.exec << std::endl;
	os << "			identifier: " << CGIConfig.extension << std::endl;
	os << "			allowed methods: ";
	for (int i = 0; i < METHOD_NBR; i++)
		os << ((CGIConfig.methods & (1 << i)) ? METHOD_STR[i] : "");
	os << std::endl;
	return (os);
}
std::ostream&	operator<<(std::ostream& os, const Host& host)
{
	host.printShort(os);
	return (os);
}

/// @brief Check if the location directory indicates a redirection.
/// Redirection are handled without additionnal checking 
/// @param client 
/// @param request 
/// @return 
int	Host::checkRedirection(Request& request) const
{
	const Location& location = *request.resHints.locationRules;
	switch (location.redir)
	{
		case 0:
			return (0);
	
		case RES_MULTIPLE_CHOICE:
			request.resHints.status = RES_MULTIPLE_CHOICE;
			break;

		case RES_MOVED_PERMANENTLY:
			request.resHints.status = RES_MOVED_PERMANENTLY;
			break;

		case RES_FOUND:
			request.resHints.status = RES_FOUND;
			break;

		case RES_SEE_OTHER:
			request.resHints.status = RES_SEE_OTHER;
			break;

		case RES_TEMPORARY_REDIRECT:
			request.resHints.status = RES_TEMPORARY_REDIRECT;
			break;

		case RES_PERMANENT_REDIRECT:
			request.resHints.status = RES_PERMANENT_REDIRECT;
			break;

		default:
			LOGE("Invalid or unsupported redirection status (%d)", location.redir);
			break;
	}
	request.resHints.redirList = &location.addr_redir;
	return (request.resHints.status);
}

/// @brief Try to match the request to location/cgi rules and to a type
/// using its ```uri``` structure. Uri is parsed again using to determine
/// ```pathinfo``` in case of CGI. Return ```RES_NOT_FOUND``` if error (```resHints```
/// is updated).
/// @param request 
/// @return 
int	Host::matchRequest(Request& request) const
{
	request.resHints.locationRules = matchLocation(request.resHints.parsedUri.path);
	request.resHints.cgiRules = matchCGIConfig(request.resHints.parsedUri.path);
	if (request.resHints.locationRules == NULL && request.resHints.cgiRules == NULL) {
		request.resHints.status = RES_NOT_FOUND;
		request.type = REQ_TYPE_NO_MATCH;
		return (RES_NOT_FOUND);
	}
	else if (request.resHints.cgiRules) {
		request.type = REQ_TYPE_CGI;
		request.extractPathInfo(request.resHints.cgiRules->extension);
		if (request.resHints.parsedUri.pathInfo.find('/') != std::string::npos)
			request.resHints.locationRules = matchLocation(request.resHints.parsedUri.path);
	}
	else if (request.resHints.parsedUri.extension == "/")
		request.type = REQ_TYPE_DIR;
	else
		request.type = REQ_TYPE_STATIC;
	return (0);
}

/// Should be called if the ressource was a directory.
/// Switch path to index file if given by host. Request is then
/// matched again to determine its final type.
/// Return error if dir_listing is not allowed, if methods is not ```GET```
int	Host::checkDirRessource(Request& request) const
{
	const Location& location = *request.resHints.locationRules;
	if (location.default_uri != "") {
		request.parseURI(location.default_uri);
		if (matchRequest(request))
			return (RES_NOT_FOUND);
		return (0);
	} else if (location.dir_listing == true) {
		if (request.method != GET) {
			request.resHints.status = RES_FORBIDDEN;
			return (RES_FORBIDDEN);
		}
		request.resHints.path = request.resHints.parsedUri.path;
	}
	return (0);
}

/// @brief Check allowed methods for given location, and if upload is
/// allowed for static upload.
/// @param location 
/// @param request 
/// @return 
int	Host::checkLocationRules(Request& request) const
{
	if ((request.resHints.locationRules->methods & (1 << request.method)) == 0) {
		request.resHints.status = RES_METHOD_NOT_ALLOWED;
		return (RES_METHOD_NOT_ALLOWED);
	}
	if (request.method == POST && request.type == REQ_TYPE_STATIC) {
		if (request.resHints.locationRules->upload == false) {
			request.resHints.status = RES_FORBIDDEN;
			return (RES_FORBIDDEN);
		}
	}
	return (0);
}

/// @brief Check if methods match allowed methods for CGI
/// @param cgi 
/// @param request 
/// @return 
int	Host::checkCGIRules(Request& request) const
{
	if ((request.resHints.cgiRules->methods & (1 << request.method)) == 0) {
		request.resHints.status = RES_METHOD_NOT_ALLOWED;
		return (RES_METHOD_NOT_ALLOWED);
	}
	return (0);
}

/// @brief Check if the given path exist from the server perspective.
/// Symbolic link or other type of files are ignored.
/// @param path 
/// @param type Can be provided to make additionnal check regarding file type
/// ```REQ_TYPE_DIR``` vs ```REQ_TYPE_CGI||REQ_TYPE_STATIC```.
/// @return 
int	Host::checkRessourcePath(const std::string& path, int type, int checkWrite)
{
	struct stat	f_stat;

	if (stat(path.c_str(), &f_stat))
		return (RES_NOT_FOUND);
	else if (type != REQ_TYPE_DIR && S_ISREG(f_stat.st_mode) == false)
		return (RES_FORBIDDEN);
	else if (type == REQ_TYPE_DIR && S_ISDIR(f_stat.st_mode) == false)
		return (RES_NOT_FOUND);
	if (checkWrite && access(path.c_str(), checkWrite) < 0)
		return (RES_FORBIDDEN);
	return (0);
}

int	Host::checkRessourceExistence(Request& request) const {
	std::string	path;
	int			res;
	if (request.type == REQ_TYPE_CGI) {
		request.resHints.scriptPath = request.resHints.cgiRules->root + request.resHints.parsedUri.path;
		res = checkRessourcePath(request.resHints.scriptPath, REQ_TYPE_CGI, R_OK);
	} else {
		if (request.method == POST) {
			path = (request.resHints.locationRules->upload_root != "" ? \
							request.resHints.locationRules->upload_root : \
							request.resHints.locationRules->root);
			res = checkRessourcePath(path + request.resHints.parsedUri.root, REQ_TYPE_DIR);
		} else {
			request.resHints.path = request.resHints.locationRules->root + request.resHints.parsedUri.path;
			res = checkRessourcePath(request.resHints.path, request.type, R_OK);
		}
	}
	request.resHints.status = res;
	return (res);
}

/** @brief Check request validity from an host perspective.
 * @note get location/cgi match
	- check redirects
	- if dir && default_uri
		- change uri
		- check again for cgi
	- check location rules
	- check ressource path
 **/
int	Host::checkRequest(Request& request) const {
	int	res = 0;

	if ((res = matchRequest(request)))
		return (res);
	if (request.resHints.locationRules && (res = checkRedirection(request)))
		return (res);
	if (request.type == REQ_TYPE_DIR) { //If folder
		if ((res = checkDirRessource(request)))
			return (res);
	}
	if (request.resHints.locationRules && (res = checkLocationRules(request)))
		return (res);
	if (request.type == REQ_TYPE_CGI && (res = checkCGIRules(request)))
		return (res);
	if ((res = checkRessourceExistence(request)))
		return (res);
	return (res);
}
