#include <iostream>
#include <string>
#include <iterator>
#include <map>
#include <stdlib.h>
#include <Request.hpp>

class Client;
class Request;
typedef struct ResHints ResHints;

#define CGI_TIME_OUT 10000

enum	CGI_RES_TYPE {CGI_RES_ERROR = -1, CGI_RES_NONE, CGI_RES_DOC, CGI_RES_LOCAL_REDIRECT, CGI_RES_CLIENT_REDIRECT};
enum	CHILD_STATUS {CHILD_RUNNING, CHILD_TERM};

std::string	IntToString(int x, int base);

class CGIProcess {

	private:

		CGIProcess();

		std::string							_line;
		size_t								_index;
		std::map<std::string, std::string, i_less>	_cgi_headers;
		struct timeval						_fork_time;
		int									_pid;
		int									_status;
		Client&								_client;
		Request&							_request;

		int		_getLine(std::string &buffer);
		int		_extract_header();
		int		_inspectHeaders();
		int		_retrieveHeader(std::string key, std::string &value);
		void	_launchCGI();
		void	_setVariables();

	public:

		~CGIProcess();
		CGIProcess(Client& client);

		static char**						_env;
		/// @brief
		/// @return ```0``` if not finished, ```> 0``` if finished, ```< 0```
		/// if error.
		int	checkEnd();

		/// @brief Add potential header to resHints, change the status
		/// @param
		/// @return Identify document type
		int	parseHeaders();
		int	execCGI();
		int	getStatus();
		int	getPID();

		class	child_exit_exception: public std::exception
		{
			public:
				child_exit_exception() {}
				const char* what() const throw();
		};

};
