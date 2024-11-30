#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <ListenServer.hpp>
#include <cstring>
#include <vector>


#define HEADER_MAX_SIZE 4000
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "


enum {none, chuncked, mesured};
enum {GET, POST, DELETE};
enum {TERM, ONGOING, RLINE,NEW};
enum {HEADER, HOST, BODY, TRAILER, CONT, COMPLETE, ERROR};
enum {REQ_TYPE_NO_MATCH = 0, REQ_TYPE_CGI, REQ_TYPE_STATIC, REQ_TYPE_DIR};

static std::string METHOD_STR[] = {"GET", "POST", "DELETE"};
#define METHOD_NBR 3
#define METHOD_IS_INVALID (&METHODS_STR[METHODS_NBR])

std::string	generate_name(const std::string* hostname);
bool nocase_string_eq(const std::string& a, const std::string& b);
int	getInt(std::string str, int base, int &res);
int	getMethodIndex(const std::string& method);

/// @note localhost:8080/server1/imgs/index.php/mer/ocean.png
/// /server1/imgs/*
/// *.php*

typedef struct URI
{
	std::string filename;
	std::string	path;
	std::string	root;
	std::string	pathInfo;
	std::string	extension;
	std::string	query;

	URI(void);
	URI(const URI&);
}	URI;

struct Location;
struct CGIConfig;

struct i_less {
	static inline char	lowercase(char c) {
		if (c >= 'A' && c <= 'Z')
			return (c + ('a' - 'A'));
		return (c);
	};
	static inline int	stricmp(const char* s1, const char* s2) {
		while (*s1 && (lowercase(*s1) == lowercase(*s2)))
		{
			s1++;
			s2++;
		}
		return (((unsigned char) lowercase(*s1)) - ((unsigned char) lowercase(*s2)));
	};
	bool operator() (const std::string& lhs, const std::string& rhs) const {
		return ((stricmp(lhs.c_str(), rhs.c_str()) < 0));
	}
};

//http:localhost:80/cgi-bin/script.php/images/pic.jpg
//path translated = root + pathinfo
typedef struct ResHints {
	URI											parsedUri;
	std::string									path; // /cgi-bin/script.php
	std::string									extension;
	std::string									scriptPath; // root + path
	std::string									bodyFileName;
	bool										alreadyExist;
	bool										unlink;
	bool										hasBody;
	std::string									verboseError;
	int											status;
	int											type;
	int											cgiRedir;
	int											index;
	const Location*								locationRules;
	const CGIConfig*							cgiRules;
	const std::vector<std::string>*				redirList;
	std::map<std::string, std::string, i_less>	headers;
	std::vector<std::string>					cookies;

	ResHints(void);
	ResHints(const ResHints&);
} ResHints;

class	Request
{
	private:

		std::map<std::string, std::string, i_less>	_headers;
		std::string							_formated_headers;
		std::string							_uri;
		int									_status;

		int									_header_size;

		std::string							_line;

		std::string							_hostname;
		int									_body_max_size;
		int									_len;
		int									_content_length;
		bool								_chunked;
		int									_b_status;
		int									_chunked_body_size;
		bool								_chunked_status;
		int									_trailer_status;
		int									_trailer_size;

		int									_final_status;

		int	_checkSizes();
		int	_parseChunked(std::string &buffer, std::ofstream *filestream);
		int	_parseMesured(std::string &buffer, std::ofstream *filestream);

	public:

		Request();
		Request(const Request& copy);
		~Request();

		int									type;
		int									method;
		ResHints							resHints;

		int			getLine(std::string &buffer);
		int			getChunkedSize(std::string &buffer);
		void		trimSpace();

		int			_fillError(int error, std::string const &verbose);

		int					parseInput(std::string &buffer, std::ofstream *filestream);
		int					parseRequestLine();
		int					parseHeaders(std::string &buffer);
		int					parseTrailerHeaders(std::string &buffer);
		int					parseBody(std::string &buffer, std::ofstream *filestream);
		int					getLenInfo();
		int					getStatus(void) const;
		void				setStatus(int status);
		int					getError() const;
		void				setChunked(bool);
		void				setContentLength(int);
		void				setUri(std::string &new_uri);

		const std::string&	getHeader(std::string const &key) const;
		void				removeHeader(const std::string& key);

		bool				checkHeader(const std::string& key) const;

		void	formatHeaders();
		void	printHeaders();
		void	setBodyMaxSize(int size);

		int		parseURI(const std::string& default_uri);
		int		parseURI();
		void	extractPathInfo(const std::string& extension);
		int		checkPath();
		void	prunePath();
		void	prunePercent();

		//DEBUG
		void	printRequest() const;

};



#endif

