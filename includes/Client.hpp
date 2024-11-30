#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <IObject.hpp>
#include <Response.hpp>
#include <Host.hpp>
#include <Request.hpp>
#include <ListenServer.hpp>
#include <ctime>
#include <queue>
#include <sys/socket.h>
#include <stdint.h>
#include <sys/time.h>
#include <signal.h>


// class Request;
// class Host;
// class ListenServer;
// class Response;

#define MAX_NBR_OUT_BUFFERS 10
#define CLIENT_TIME_OUT 100000 //timeout in milliseconds for client input


#ifndef BUFFER_SIZE
# define BUFFER_SIZE 4096
#endif

#define HEADER_STATUS_ONGOING 0
#define HEADER_STATUS_READY 1
#define HEADER_STATUS_DONE 2

#define BODY_STATUS_NONE 0
#define BODY_STATUS_ONGOING 1
#define BODY_STATUS_DONE 2


enum {CLIENT_MODE_READ, CLIENT_MODE_WRITE, CLIENT_MODE_ERROR};

struct ClientSocket
{
	int				fd;
	struct sockaddr	addr;
	socklen_t		addrSize;
};

class	CGIProcess;

class	Client : public IObject
{

	private:

		static std::list<Client>	_clientList;

		Client();
		Client(const ClientSocket& socket, ListenServer& listenServer);
		// Client(int fd, Host *host, Request *req, Response *resp);

		ClientSocket		_socket;

		std::string			_addressStr;
		int					_port;

		Host*				_host;
		ListenServer&		_listenServer;
		Request*			_request;
		AResponse*			_response;

		struct timeval		_lastInteraction;

		int					_headerStatus;
		int					_bodyStatus;
		int					_mode;
		std::string			_buffer;
		std::string			_bodyFileName;
		std::ofstream*		_bodyStream;

		// May want something more versatile
		// (If CGI, it would be linked to a pipe
		// If static, it would be linked to an ifstream)

		void	clearBuffers(void);

		static std::list<Client>::iterator	findClient(Client* client);

		friend Client* 			ListenServer::acceptConnection(void);
		friend std::ostream&	operator<<(std::ostream& os, const Client&);


	public:

		std::queue<std::string>	outBuffers;
		CGIProcess*				cgiProcess;

		Client(const Client &Copy);
		virtual ~Client();

		static Client*		newClient(const ClientSocket& socket, ListenServer& listenServer);
		static void			deleteClient(Client* client);

		static int			getTotalNbrClient(void);

		int					getfd() const;
		const Host*			getHost() const;
		const std::string&	getStrAddr(void) const;
		int					getAddrPort(void) const;

		int					getRequestStatus() const; //returns 1 if request has been fully received
		int					getResponseStatus() const; // returns 1 if response is ready to send
		Request*			getRequest(); // returns the current not complete request or allocate a new one
		AResponse*			getResponse(); // returns the current not complete response

		static void			checkTO();
		/// @brief
		/// @param buffer null terminated buffer
		/// @return ```< 0```
		int					parseRequest(const char* buffer, int nread);

		int					getHeaderStatus() const;
		int					getBodyStatus() const;
		int					getMode() const;
		const std::string&	getBodyFile() const;
		void				setMode(int mode);
		void				setHeaderStatus(int);
		void				setBodyStatus(int);
		// void				setBodyStream(std::ofstream*);
		void				setBodyFile(const std::string&);
		void				stashBuffer(std::string &buffer);
		void				retrieveBuffer(std::string &str);
		void				clearBuffer();
		void				clearResponse(void);
		void				updateLastInteraction(void);

		void				setHost(const std::string& hostname);
		void				setResponse(AResponse*);

		void				shutdownConnection(void);
		void				deleteBodyStream();
		void				deleteCGIProcess();
		void				clear(void);
		void				terminate();

};

std::ostream&	operator<<(std::ostream& os, const Client&);

#endif
