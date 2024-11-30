#ifndef LISTENSERVER_HPP
#define LISTENSERVER_HPP

#include <list>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <set>
#include <unistd.h>
#include <string>
#include <string.h>
#include <map>
#include <IObject.hpp>
#include <vector>

class Client;
class Host;

#define MAX_CLIENT_NBR INT32_MAX

class ListenServer : public IObject
{
	private:

		ListenServer(void);
		ListenServer(std::string const &hostAddr, std::string const &hostPort);

		static	std::list<ListenServer>	_serverList;

		std::list<Client*>				_orphanClients;
		/// @warning This list exist only for monitoring purpose and SHOULD NOT be used
		/// for interaction with the clients.
		std::list<Client*>				_connectedClients;

		int								_sockFd;
		std::map<std::string, Host*>	_hostMap;
		std::string						_ip;
		std::string						_port;
		int								_maxClientNbr; //optionnal
		int								_nbrHost;

		static ListenServer	*addServer(const std::string& hostAddr, const std::string& hostPort);
		static void			removeServer(std::list<ListenServer>::iterator it);

		friend std::ostream&	operator<<(std::ostream& os, const ListenServer& ls);

	public:

		ListenServer(const ListenServer&);
		virtual ~ListenServer();

		static std::list<ListenServer>::iterator	findServer(const std::string& hostAddr, const std::string& hostPort);
		static bool	serverExist(const std::string& hostAddr, const std::string& hostPort);

		static int	registerHost(Host* host);
		static int	registerHost(Host*, const std::string& port);
		static void	unregisterHost(Host* host);
		static void	unregisterHost(Host*, const std::set<ListenServer*> serverSet);
		static void	unregisterHost(Host*, const std::string& port);
		static void	unregisterHost(Host*, std::list<ListenServer>::iterator serverPos);

		static int	startServers(int epollfd);
		static void	closeServers(void);
		static void	removeServer(const std::string& hostAddr, const std::string& hostPort);
		static void	removeServers(void);

		static bool	checkServerNames(const std::string& hostAddr, const std::set<std::string>& ports, const std::vector<std::string>& serverNames);

		static std::list<ListenServer>::const_iterator	getServerListBegin(void);
		static std::list<ListenServer>::const_iterator	getServerListEnd(void);
		static int	getNbrServer(void);


		int			removeHost(const std::string& serverName);
		Host*		findHost(const std::string& serverName);
		void		removeClient(Client*);

		void		assignHost(Host* host);
		void		unassignHost(Host* host);

		int			registerToEpoll(int epollfd); //open socket
		void		shutdown(void);

		int			getNbrConnectedClients(void) const;
		int			getNbrHost(void) const;

		Client*		acceptConnection(void);
		Host*		bindClient(Client& client, const std::string& hostName);
		int			getHostMaxSize(std::string &hostname);

		std::ostream&		printShort(std::ostream&) const;
		std::ostream&		printFull(std::ostream&) const;

};

std::ostream&	operator<<(std::ostream& os, const ListenServer& ls);


#endif
