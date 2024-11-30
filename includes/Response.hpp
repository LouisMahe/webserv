#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <Request.hpp>
#include <queue>
#include <iterator>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

// 1xx indicates an informational message only
// 2xx indicates success of some kind
// 3xx redirects the client to another URL
// 4xx indicates an error on the client's part
// 5xx indicates an error on the server's part
#define RES_CONTINUE 100
#define RES_OK 200
#define RES_CREATED 201
#define RES_NO_CONTENT 204
#define RES_MULTIPLE_CHOICE 300 //Dynamic => Location headers + in body
#define RES_MOVED_PERMANENTLY 301 //Dynamic => Location headers + in body
#define RES_FOUND 302 //Dynamic => Location headers + in body
#define RES_SEE_OTHER 303 //Dynamic => Location headers + in body
#define RES_TEMPORARY_REDIRECT 307 //Dynamic => Location headers + in body
#define RES_PERMANENT_REDIRECT 308
#define RES_BAD_REQUEST 400 //Dynamic
#define RES_FORBIDDEN 403 //full static
#define RES_NOT_FOUND 404 //full static
#define RES_METHOD_NOT_ALLOWED 405 //full static
#define RES_TIMEOUT 408 //full static
#define RES_LENGTH_REQUIRED 411 //full static
#define RES_REQUEST_ENTITY_TOO_LARGE 413 //full static
#define RES_REQUEST_URI_TOO_LONG 414 //full static
#define RES_UNSUPPORTED_MEDIA_TYPE 415
#define RES_EXPECTATION_FAILED 417 //full static
#define RES_INTERNAL_ERROR 500 //Dynamic
#define RES_NOT_IMPLEMENTED 501 //Dynamic
#define RES_SERVICE_UNAVAILABLE 503 //Dynamic
#define RES_HTTP_VERSION_NOT_SUPPORTED 505 //full static


/*
Response hints:
	- final path
	- was the ressource already existing ?
	- redirection location
	- allowed methods
	- verbose error

*/

/** @brief

Error response

**/
class	AResponse
{
	protected:

		AResponse(void) {}
		int									_status;
		std::string							_description;

	public:

		virtual ~AResponse() {}
		virtual int			writeResponse(std::queue<std::string>& outQueue) = 0;
		static AResponse	*genResponse(ResHints &hints);

};

class	SingleLineResponse : public AResponse
{
	private:

		SingleLineResponse(void);
		std::string	_response;

	public:

		virtual ~SingleLineResponse(void);
		SingleLineResponse(int status, const std::string& description);

		virtual int		writeResponse(std::queue<std::string>& outQueue);
};

class	HeaderResponse : public AResponse
{
	protected:

		std::string									_formated_headers;
		ResHints									&hints;

		void										_formatHeaders();
		int 										_retrieveHeader(std::string key, std::string &value);


	public:

		HeaderResponse(ResHints &hints);
		virtual ~HeaderResponse(void);

		virtual int		writeResponse(std::queue<std::string>& outQueue);
		void			addHintHeaders(ResHints &hints);
		void			addHeader(std::string const &key, std::string const &value);
		void			addUniversalHeaders();
		virtual void			addSpecificHeaders() = 0;
};

class	FileResponse : public HeaderResponse
{
	private:

		FileResponse();

		std::fstream	_fstr;
		std::string		_path;
		int				_retrieveType(std::string key, std::string &value);

	public:

		FileResponse(ResHints &hints);
		~FileResponse();
		int				inspectFile(ResHints &hints);
		void			checkType();
		virtual int		writeResponse(std::queue<std::string>& outQueue);
		virtual void	addSpecificHeaders() {}
};

class	DynamicResponse : public HeaderResponse
{

	private:

		DynamicResponse();
		std::string							_body;


	public:

		// typedef	void (*bodyGenerator_func)(DynamicResponse&, const std::string);

		//dir_listing
		//Redir
		DynamicResponse(ResHints &hints);
		// DynamicResponse(bodyGenerator_func, const std::string&);
		virtual ~DynamicResponse();
		void	generateBody();
		void	generateListBody();
		void	generateRedirBody();
		void	generateSimpleRedirBody();
		void	generateVerboseBody();
		void	generateDirListing();
		virtual void	addSpecificHeaders();
		virtual int		writeResponse(std::queue<std::string>& outQueue);
};


template <typename T>
  std::string itostr ( T num )
  {
     std::ostringstream ss;
     ss << num;
     return ss.str();
  }


#endif
