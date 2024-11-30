#include <Request.hpp>
#include <ILogger.hpp>

static const std::string dummyString = "";

URI::URI(void) {}

URI::URI(const URI &copy) : filename(copy.filename), path(copy.path), root(copy.root), pathInfo(copy.pathInfo),
							extension(copy.extension), query(copy.query) {}

ResHints::ResHints(void) : alreadyExist(false), unlink(false), hasBody(false), status(0), type(0),
						   cgiRedir(0), index(0), locationRules(NULL), cgiRules(NULL) {}

ResHints::ResHints(const ResHints &copy) : parsedUri(copy.parsedUri), path(copy.path), alreadyExist(copy.alreadyExist),
										   unlink(copy.unlink), hasBody(copy.hasBody), verboseError(copy.verboseError), status(copy.status),
										   type(copy.type), cgiRedir(copy.cgiRedir), index(copy.index), locationRules(copy.locationRules),
										   cgiRules(copy.cgiRules), redirList(copy.redirList), headers(copy.headers), cookies(copy.cookies) {}

Request::Request()
	: _status(NEW), _header_size(0), _body_max_size(0), _len(0),
	  _content_length(-1), _chunked(0), _b_status(NEW), _chunked_body_size(0),
	  _chunked_status(0), _trailer_status(0), _trailer_size(0),
	  _final_status(ONGOING), type(0), method(-1) {}

Request::Request(const Request &copy)
	: _headers(copy._headers), _formated_headers(copy._formated_headers),
	  _uri(copy._uri), _status(copy._status),
	  _header_size(copy._header_size), _line(copy._line), _hostname(copy._hostname),
	  _body_max_size(copy._body_max_size),
	  _len(copy._len), _content_length(copy._content_length),
	  _chunked(copy._chunked), _b_status(copy._b_status),
	  _chunked_body_size(copy._chunked_body_size), _chunked_status(copy._chunked_status),
	  _trailer_status(copy._trailer_status), _trailer_size(copy._trailer_size),
	  _final_status(copy._final_status), type(copy.type),
	  method(copy.method), resHints(copy.resHints)
{
}

Request::~Request()
{

}

void Request::trimSpace()
{
	while (this->_line[0] == 32 || this->_line[0] == 9)
		this->_line.erase(0, 1);
}

const std::string &Request::getHeader(std::string const &key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it == _headers.end())
		return (dummyString);
	return (it->second);
}

void	Request::removeHeader(const std::string& key) {
	_headers.erase(key);
}

int Request::_fillError(int error, std::string const &verbose)
{
	this->resHints.status = error;
	this->resHints.verboseError = verbose;
	this->_line.clear();
	this->_final_status = ERROR;
	return (error);
}

void Request::formatHeaders()
{
	std::string header;
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		header = it->first + ": " + it->second + "\r\n";
		this->_formated_headers += header;
		header = "";
	}
	this->_formated_headers += "\r\n";
}

int Request::getLine(std::string &buffer)
{
	std::string::size_type idx;

	buffer = _line + buffer;
	idx = buffer.find(CRLF, 0);

	if (idx == std::string::npos)
	{
		this->_line = buffer;
		buffer = "";
		return (1);
	}
	this->_line = buffer.substr(0, idx);
	buffer = buffer.substr(idx + strlen(CRLF), std::string::npos);
	return (0);
}

static void skipSpaces(std::string::size_type &idx, std::string &str)
{
	while ((str[idx] == 32 || str[idx] == 9) && idx < str.size())
		idx++;
}



static void nextSpace(std::string::size_type &idx, std::string &str)
{
	while (str[idx] != 32 && str[idx] != 9 && idx < str.size())
		idx++;
}

static int identifyMethod(std::string mtd)
{
	if (mtd == "GET")
		return (0);
	if (mtd == "POST")
		return (1);
	if (mtd == "DELETE")
		return (2);
	return (-1);
}

static int checkVersion(std::string &str)
{
	std::string::size_type idx = 0;
	if ((str.size() < 8) || str.find("HTTP/", idx) != 0)
		return (1);
	idx = 5;
	if (str.substr(idx, str.size()) != "1.1" && str.substr(idx, str.size()) != "1.0")
		return (505);
	// if (!isdigit(str[idx]))
	// 	return (1);
	// while (isdigit(str[idx]))
	// 	idx++;
	// if (idx == str.size())
	// 	return (0);
	// if (str[idx] != '.')
	// 	return (1);
	// idx++;
	// if (!isdigit(str[idx]))
	// 	return (1);
	// while (isdigit(str[idx]))
	// 	idx++;
	// if (idx != str.size())
	// 	return (1);
	return (0);
}

static int getHeaderName(std::string &line, std::string &header)
{
	std::string::size_type idx = 0;

	idx = line.find(":", 0);
	if (idx == std::string::npos)
		return (1);
	header = line.substr(0, idx);
	line = line.substr(idx + 1, std::string::npos);
	return (0);
}

int Request::parseRequestLine()
{
	std::string::size_type idx = 0;
	std::string::size_type r_idx = 0;
	std::string version;
	// LOGD("line = %ss", &_line);
	nextSpace(idx, this->_line);
	if ((this->method = identifyMethod(this->_line.substr(0, idx))) < 0)
		return (this->_fillError(501, "Method Not Implemented"));
	skipSpaces(idx, this->_line);
	r_idx = idx;
	nextSpace(r_idx, this->_line);
	this->_uri = std::string(this->_line, idx, r_idx - idx);
	version = this->_line.substr(r_idx + 1, std::string::npos);
	if (checkVersion(version))
		return (this->_fillError(505, "HTTP version not implemented"));
	return (0);
}

int Request::parseHeaders(std::string &buffer)
{
	std::string header_name;
	std::string header_value;

	if (this->_status == TERM)
		return (0);
	this->_header_size += buffer.size();
	if (this->_status == NEW)
	{
		while (buffer.empty() == false && !isalpha(buffer[0]))
			buffer.erase(0, 1);
		if (!buffer.empty())
		{
			_status = RLINE;
			this->_header_size = buffer.size();
		}
		else
			return (_checkSizes());
	}
	if (_status == RLINE)
	{
		if (this->getLine(buffer))
			return (this->_checkSizes());
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(414, "Request uri too long"));
		if (this->parseRequestLine())
			return (getError());
		this->_line = "";
		this->_status = ONGOING;
	}
	while (!buffer.empty())
	{
		if (this->getLine(buffer))
			return (this->_checkSizes());
		if (this->_line == "")
		{
			this->_status = TERM;
			this->_final_status = HOST;
			this->_line.clear();
			return (-1);
		}
		this->trimSpace();
		if (getHeaderName(this->_line, header_name))
			return (this->_fillError(400, "Invalid header line"));
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(431, "The " + header_name + " header was too large"));
		while (this->_line[0] == 32 || this->_line[0] == 9)
			this->_line.erase(0, 1);
		header_value = this->_line;
		if (getHeader(header_name) != "")
			this->_headers[header_name] += ", " + header_value;
		else
			this->_headers[header_name] = header_value;
		this->_line.clear();
	}
	this->_header_size -= buffer.size();
	return (this->_checkSizes());
}

int Request::_parseMesured(std::string &buffer, std::ofstream *filestream)
{
	size_t left_to_complete;
	size_t cut;
	// size_t	n_writen;
	std::string extract;

	left_to_complete = this->_content_length - this->_len;
	if (left_to_complete > buffer.size())
		cut = buffer.size();
	else
		cut = left_to_complete;
	extract = buffer.substr(0, cut);
	if (filestream)
		filestream->write(extract.c_str(), extract.size());
	this->_len += extract.size();
	buffer = buffer.substr(cut, std::string::npos);
	if (this->_len == this->_content_length)
	{
		this->_b_status = TERM;
		this->_final_status = COMPLETE;
		if (filestream)
			filestream->close();
		filestream = NULL;
		return (-1);
	}
	return (0);
}

int Request::getLenInfo()
{
	std::string str = this->getHeader("Content-Length");
	int res = 0;
	if (nocase_string_eq(this->getHeader("Transfer-Encoding"), "chunked"))
	{
		this->_chunked = 1;
		this->_content_length = -1;
	}
	else if (this->getHeader("Content-Length") != "")
	{
		if (getInt(this->getHeader("Content-Length"), 10, res))
			return (this->_fillError(400, "Incorrect content-length provided"));
		if (res > this->_body_max_size)
			return (this->_fillError(413, "Request entity too large"));
		this->_content_length = res;
		return (0);
	}
	else
	{

		this->_b_status = TERM;
		this->_final_status = COMPLETE;
	}
	return (0);
}

int Request::getChunkedSize(std::string &buffer)
{
	std::string::size_type idx;
	int len = 0;

	if (this->_chunked_status == 1)
		return (0);
	idx = buffer.find(CRLF, 0);
	if (idx == std::string::npos)
	{
		this->_line += buffer;
		buffer = "";
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(400, "Chunked body length line too long"));
		else
			return (0);
	}
	this->_line += buffer.substr(0, idx);
	if (getInt(this->_line, 16, len) || len < 0)
		return (this->_fillError(400, "Syntax error parsing chunked body"));
	this->_chunked_body_size -= this->_line.size();
	this->_len = len;
	this->_line = "";
	buffer = buffer.substr(idx + strlen(CRLF), std::string::npos);
	this->_chunked_status = 1;
	return (0);
}

int Request::_parseChunked(std::string &buffer, std::ofstream *filestream)
{
	if (this->_b_status == TERM)
		return (0);
	this->_chunked_body_size += buffer.size();
	while (buffer != "")
	{
		if (this->getChunkedSize(buffer))
			return (getError());
		if (this->_chunked_status == 1 && this->_len == 0)
		{
			this->_b_status = TERM;
			this->_final_status = TRAILER;
			this->_line.clear();
			if (filestream)
				filestream->close();
			filestream = NULL;
			return (-1);
		}
		if (this->getLine(buffer))
			return (this->_checkSizes());
		if (this->_line.size() != static_cast<unsigned long>(this->_len))
			return (this->_fillError(400, "Size error in chunked body"));
		if (filestream)
			filestream->write(this->_line.c_str(), this->_line.size());
		this->_line = "";
		this->_len = -1;
		this->_chunked_status = 0;
	}
	if (this->_chunked_body_size > this->_body_max_size)
		return (this->_fillError(413, "Request entity too large"));
	return (0);
}

int Request::parseBody(std::string &buffer, std::ofstream *filestream)
{
	if (this->_b_status == TERM || buffer.empty())
		return (-1);
	if (this->_b_status == NEW)
	{
		// if (this->getLenInfo() || this->_b_status == COMPLETE)
		// 	return (this->_error_num);
		// this->_tmp_filename = generate_name(this->_hostname);
		// this->_bodyStream.open(this->_tmp_filename.c_str(), std::ios::out | std::ios::app | std::ios::binary);
		this->_b_status = ONGOING;
	}
	if (this->_chunked)
		return (this->_parseChunked(buffer, filestream));
	else
		return (this->_parseMesured(buffer, filestream));
}

int Request::getStatus(void) const
{
	return (this->_final_status);
}

void	Request::setContentLength(int value) {
	_content_length = value;
}

void	Request::setChunked(bool value) {
	_chunked = value;
}

void Request::printRequest() const
{
	std::cout << "method is: " << this->method << std::endl;
	std::cout << "uri is: " << this->_uri << std::endl;
	std::cout << "status is: " << this->_status << std::endl;
	std::cout << "bstatus is: " << this->_b_status << std::endl;
	std::cout << "error is: " << this->resHints.status << std::endl;
}

void Request::printHeaders()
{
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << "\r\n";
	}
}

int getMethodIndex(const std::string &method)
{
	for (int i = 0; i < METHOD_NBR; i++)
	{
		if (METHOD_STR[i] == method)
			return (i);
	}
	return (-1);
}

int Request::parseTrailerHeaders(std::string &buffer)
{
	std::string header_name;
	std::string header_value;

	if (this->_b_status != TERM)
		return (0);
	if (this->_chunked == 0 || this->getHeader("Trailer") == "")
	{
		this->_trailer_status = TERM;
		this->_final_status = COMPLETE;
		return (-1);
	}
	this->_trailer_size += buffer.size();
	while (!buffer.empty())
	{
		if (this->getLine(buffer))
			return (this->_checkSizes());
		if (this->_line.empty())
		{
			this->_trailer_status = TERM;
			this->_final_status = COMPLETE;
			this->_line.clear();
			return (-1);
		}
		this->trimSpace();
		if (getHeaderName(this->_line, header_name) || header_name == "Trailer" || header_name == "Content-Length" || header_name == "Transfer-Encoding")
			return (this->_fillError(400, "Invalid trailer header line"));
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(431, "The " + header_name + " header was too large"));
		this->trimSpace();
		header_value = this->_line;
		if (getHeader(header_name) != "")
			this->_headers[header_name] += ", ";
		this->_headers[header_name] += header_value;
		this->_line = "";
	}
	return (this->_checkSizes());
}

int Request::_checkSizes()
{
	if (this->_status == ONGOING || this->_status == NEW)
	{
		if (this->_header_size > HEADER_MAX_BUFFER * HEADER_MAX_SIZE || this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(431, "Request header too large"));
		else
			return (0);
	}
	if (this->_status == TERM && this->_b_status == ONGOING)
	{
		if (this->_chunked && this->_chunked_body_size > this->_body_max_size)
			return (this->_fillError(413, "Request entity too large"));
		return (0);
	}
	if (this->_trailer_status == ONGOING)
	{
		if (this->_trailer_size > HEADER_MAX_SIZE)
			return (this->_fillError(400, "Trailer header too large"));
		return (0);
	}
	return (0);
}

int Request::getError() const
{
	return (this->resHints.status);
}

void Request::setBodyMaxSize(int size)
{
	_body_max_size = size;
}

int Request::parseInput(std::string &buffer, std::ofstream *filestream)
{
	int	res = 0;
	if (this->_final_status > TRAILER || buffer.empty())
		return (0);
	if ((res = this->parseBody(buffer, filestream)))
		return (res);
	if ((res = this->parseTrailerHeaders(buffer)))
		return (res);
	return (res);
}

bool Request::checkHeader(const std::string &key) const
{
	std::map<std::string, std::string, i_less>::const_iterator it = _headers.find(key);
	if (it == _headers.end())
		return (false);
	return (true);
}

void Request::setStatus(int status)
{
	this->_final_status = status;
}

int pruneScheme(std::string &uri)
{
	size_t n = uri.find("://", 0);
	if (n != std::string::npos)
	{
		if (uri.substr(0, n + 3) != "http://")
			return (1);
		uri.erase(0, n + 3);
	}
	return (0);
}

int pruneDelim(std::string &uri, std::string delim)
{
	size_t idx = uri.find(delim, 0);
	if (idx != std::string::npos && idx + 1 != uri.length()) //! modified
		uri.erase(0, idx + delim.size());
	return (0);
}

std::string extractPath(std::string &uri)
{
	std::string path;

	size_t idx = uri.find("?", 0);
	if (idx != std::string::npos)
	{
		path = uri.substr(0, idx);
		uri.erase(0, idx + 1);
		return (path);
	}
	path = uri; //! also modified
	uri.clear();
	return (path);
}

int Request::checkPath()
{
	size_t idx = 0;
	std::string subpath;
	std::vector<std::string> v;

	if (resHints.parsedUri.path[0] == '.' && resHints.parsedUri.path[1] == '/')
		resHints.parsedUri.path.erase(0, 2);
	while ((idx = resHints.parsedUri.path.find("//", 0)) != std::string::npos)
		resHints.parsedUri.path.erase(idx, 1);
	while ((idx = resHints.parsedUri.path.find("/./", 0)) != std::string::npos)
		resHints.parsedUri.path.erase(idx, 2);
	idx = 0;
	while ((idx = resHints.parsedUri.path.find("/", 0)) != std::string::npos)
	{
		subpath = resHints.parsedUri.path.substr(0, idx);
		resHints.parsedUri.path.erase(0, idx + 1);
		if (subpath == ".." && v.size() == 0)
			return (1);
		else if (subpath == ".." && v.size())
			v.pop_back();
		else
			v.push_back(subpath);
		subpath.clear();
	}
	v.push_back(resHints.parsedUri.path);
	resHints.parsedUri.path.clear();
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
		resHints.parsedUri.path += "/" + *it;
	resHints.parsedUri.path.erase(0, 1);
	return (0);
}

void Request::prunePath()
{
	if (resHints.parsedUri.path.empty() || resHints.parsedUri.path[resHints.parsedUri.path.length() - 1] == '/')
	{
		resHints.parsedUri.root = resHints.parsedUri.path;
		resHints.parsedUri.extension = "/";
		resHints.parsedUri.filename = "";
		return;
	}
	size_t idx = resHints.parsedUri.path.find_last_of("/");
	if (idx == 0 || idx == std::string::npos) //! modified
		resHints.parsedUri.root = "/";
	else
		resHints.parsedUri.root = resHints.parsedUri.path.substr(0, idx + 1);
	resHints.parsedUri.filename = resHints.parsedUri.path.substr(idx + 1);
	idx = resHints.parsedUri.filename.rfind('.');
	if (idx == std::string::npos)
		resHints.parsedUri.extension = "";
	else
		resHints.parsedUri.extension = resHints.parsedUri.filename.substr(idx);
}

int Request::parseURI(const std::string &suffix)
{
	_uri += suffix;
	return (parseURI());
}

int Request::parseURI()
{
	resHints.parsedUri.query = _uri;
	resHints.parsedUri.pathInfo = "";
	if (pruneScheme(resHints.parsedUri.query))
		return (_fillError(400, "Bad uri"));
	pruneDelim(resHints.parsedUri.query, "@");
	pruneDelim(resHints.parsedUri.query, "/");
	resHints.parsedUri.path = extractPath(resHints.parsedUri.query);
	if (checkPath())
		return _fillError(400, "too many ../ in uri");
	prunePath();
	prunePercent();
	LOGD("path = |%ss| root = |%ss| filename = |%ss| extension = |%ss| query = |%ss|",
		 &resHints.parsedUri.path, &resHints.parsedUri.root, &resHints.parsedUri.filename, &resHints.parsedUri.extension, &resHints.parsedUri.query);
	return (0);
}

void Request::extractPathInfo(const std::string &extension)
{
	size_t idx = resHints.parsedUri.path.rfind(extension);
	resHints.parsedUri.pathInfo = resHints.parsedUri.path.substr(idx + extension.length());
	resHints.parsedUri.path = resHints.parsedUri.path.substr(0, idx + extension.length());
	idx = resHints.parsedUri.path.find_last_of('/');
	if (idx == std::string::npos) {
		resHints.parsedUri.filename = resHints.parsedUri.path;
		resHints.parsedUri.root = "/";
	}
	else {
		resHints.parsedUri.root = resHints.parsedUri.path.substr(0, idx + 1);
		resHints.parsedUri.filename = resHints.parsedUri.path.substr(idx + 1);
	}
	resHints.parsedUri.extension = extension;
	LOGD("path = |%ss| root = |%ss| filename = |%ss| extension = |%ss| query = |%ss| pathinfo = |%ss|",
		 &resHints.parsedUri.path, &resHints.parsedUri.root, &resHints.parsedUri.filename, &resHints.parsedUri.extension, &resHints.parsedUri.query,
		 &resHints.parsedUri.pathInfo);
}

void	translatePercent(std::string &str)
{
	size_t	idx = 0;
	size_t	cur = 0;
	size_t	len = str.size();
	int		res;
	std::string	hexaChar;
	while ((cur = str.find("%", idx)) < len - 2)
	{
		hexaChar = str.substr(cur + 1, 2);
		if (getInt(hexaChar, 16, res) == 0)
		{
			str[cur] = res;
			str.erase(cur + 1, 2);
			len -= 2;
		}
		idx = cur + 1;
	}
}

void	Request::prunePercent()
{
	translatePercent(resHints.parsedUri.path);
	translatePercent(resHints.parsedUri.pathInfo);
	translatePercent(resHints.parsedUri.query);
	translatePercent(resHints.parsedUri.root);
	translatePercent(resHints.parsedUri.extension);
	translatePercent(resHints.parsedUri.filename);
}

void	Request::setUri(std::string &new_uri)
{
	_uri = new_uri;
}
