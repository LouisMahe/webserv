#ifndef HOST_HPP
# define HOST_HPP

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <deque>
#include <Request.hpp>
#include <Client.hpp>
#include <Request.hpp>
// #include <Client.hpp>

class IParseConfig;
class Client;

#define DFT_VALUE_ADDR "localhost"
#define DFT_VALUE_PORT "8080"
#define DFT_VALUE_MAX_BODY_SIZE INT32_MAX
#define DFT_VALUE_METHOD (1 << GET)

typedef struct Location
{
	Location(void);
	Location(const Location&);

	std::string					root;
	int							methods;
	bool						dir_listing; //ignore is default_uri != 0
	std::string					default_uri;

	bool						upload; //ignore if POST not allowed
	std::string					upload_root;

	// bool	redirection;
	int							redir;
	std::vector<std::string>	addr_redir;

	bool						operator==(const Location&) const;
} Location;

std::ostream&	operator<<(std::ostream& os, const Location& location);

typedef struct CGIConfig
{
	CGIConfig(void);
	CGIConfig(const CGIConfig&);

	std::string					exec; //example '/bin/bash'
	std::string					root;
	int							methods; //GET-POST-DELETE
	std::string					extension;

	bool						operator==(const CGIConfig&) const;
} CGIConfig;

std::ostream&	operator<<(std::ostream& os, const CGIConfig& CGIConfig);

class	Host {

	private:

		friend class IParseConfig;
		friend std::ostream&	operator<<(std::ostream& os, const Host& host);

		Host	&operator=(Host &rhs);

		// friend class IParseConfig		Host(const std::string& block); //Throw an exception if block is invalid

		// friend class	IParseConfig;

		static std::list<Host>				_hostList;
		static std::list<Location>			_locations;
		static std::list<CGIConfig>			_cgis;

		std::string							_addr;
		std::set<std::string>				_ports;
		int									_body_max_size;
		std::string							_dir_errors;
		std::vector<std::string>			_server_names;
		std::map<std::string, Location*>	_locationMap;
		std::map<std::string, CGIConfig*>	_cgiMap;
		std::list<Client*>					_clients; //<fd, Client> ?
		std::set<ListenServer*>				_listenServers;
		// a log file per Host ?

		void				addCGIConfig(const std::deque<std::string>& names, CGIConfig& cgiConfig);
		void				addLocation(const std::deque<std::string>& names, Location& location);

		void				printProperties(std::ostream& os) const;

		int					checkRedirection(Request& request) const;
		int					checkDirRessource(Request& request) const;
		int					checkLocationRules(Request& request) const;
		int					checkCGIRules(Request& request) const;
		int					checkRessourceExistence(Request& request) const;
		static int			checkRessourcePath(const std::string& path, int type = 0, int checkWrite = 0);

		template <typename T>
		const T				getMapObjectByKey(const typename std::map<std::string, T>&, const std::string& key) const;

		template <typename T>
		T*					getObjectMatch(const typename std::map<std::string, T*>&, const std::string& uri) const;

	public:

		Host(); //Throw an exception if block is invalid
		//Construct from what ?
		Host(const Host &Copy);
		~Host();

		static void							addHost(Host& host);
		static void							removeHost(Host* host);
		static std::list<Host>::iterator	findHost(Host* host);

		static void							printLocationsDebug(void);

		void							addListenServer(ListenServer*);
		void							removeListenServer(ListenServer*);

		const std::set<std::string>&	getPorts() const;
		const std::set<ListenServer*>&	getListenServers() const;
		int								getMaxSize() const;
		const std::string&				getAddr() const;
		Client							*getClientByFd(int fd) const;
		const Location*					getLocation(std::string const &uri) const; //return the "/" loc if no match
		const CGIConfig*				getCGIConfig(std::string const &uri) const;
		const Location*					matchLocation(std::string const &uri) const; //return the "/" loc if no match
		const CGIConfig*				matchCGIConfig(std::string const &uri) const;
		const std::vector<std::string>&	getServerNames(void) const;
		std::list<Client*>::const_iterator	getClientListBegin(void) const;
		std::list<Client*>::const_iterator	getClientListEnd(void) const;
		const std::string&				getDirErrorPages(void) const;
		bool							checkServerName(std::string const &name) const; // check if the name refers to a server_name

		void							removeClient(Client*); //remove the Client associated with fd
		void							addClient(Client *newClient);

		void							shutdown(void);

		void							printShort(std::ostream& os) const;
		void							printFull(std::ostream& os) const;

		int								checkRequest(Request& request) const;

		int								matchRequest(Request&) const;
};

std::ostream&	operator<<(std::ostream& os, const Host& host);

template <typename T>
const T	Host::getMapObjectByKey(const typename std::map<std::string, T>& map, const std::string& key) const {
	typename std::map<std::string, T>::const_iterator	pos = map.find(key);
	if (pos == map.end())
		return (NULL);
	return (pos->second);
}

#include <ILogger.hpp>
bool	isUriMatch(const std::string& uriRef, const std::string& expression, const std::string* previousMatch);

template <typename T>
T*	Host::getObjectMatch(const typename std::map<std::string, T*>& map, const std::string& uri) const {
	const typename std::pair<const std::string, T*>*	bestMatch = NULL;

	for (typename std::map<std::string, T*>::const_iterator it = map.begin(); it != map.end(); it++)
	{
		if (isUriMatch(uri, it->first, (bestMatch ? &bestMatch->first : NULL)))
			bestMatch = &*it;
	}
	if (bestMatch)
		return (bestMatch->second);
	return (NULL);

}


// size_t	evaluateMatchStrength(const std::string& expression, const std::string& uri);


#endif
