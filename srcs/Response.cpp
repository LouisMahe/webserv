#include <Response.hpp>
#include <Request.hpp>
#include <sys/time.h>
#include <fstream>
#include <ctime>
#include <ILogger.hpp>

static	std::string	days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static	std::string	months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", };

std::map<std::string, std::string>	init_mimes()
{
	std::map<std::string, std::string>	mipmap;

	mipmap[".html"] = "text/html";
	mipmap[".htm"] = "text/html";
	mipmap[".shtml"] = "text/html";
	mipmap[".css"] = "text/css";
	mipmap[".xml"] = "text/xml";
	mipmap[".gif"] = "image/gif";
	mipmap[".jpeg"] = "image/jpeg";
	mipmap[".jpg"] = "image/jpeg";
	mipmap[".js"] = "application/javascript";
	mipmap[".atom"] = "application/atom+xml";
	mipmap[".rss"] = "application/rss+xml";
	mipmap[".mml"] = "text/mathml";
	mipmap[".txt"] = "text/plain";
	mipmap[".jad"] = "text/vnd.sun.j2me.app-descriptor";
	mipmap[".wml"] = "text/vnd.wap.wml";
	mipmap[".htc"] = "text/x-component";
	mipmap[".avif"] = "image/avif";
	mipmap[".png"] = "image/png";
	mipmap[".svg"] = "image/svg+xml";
	mipmap[".svgz"] = "image/svg+xml";
	mipmap[".tif"] = "image/tiff";
	mipmap[".tiff"] = "image/";
	mipmap[".wbmp"] = "image/vnd.wap.wbmp";
	mipmap[".webp"] = "image/webp";
	mipmap[".ico"] = "image/x-icon";
	mipmap[".jng"] = "image/x-jng";
	mipmap[".bmp"] = "image/x-ms-bmp";
	mipmap[".woff"] = "font/woff";
	mipmap[".woff2"] = "font/woff2";
	mipmap[".jar"] = "application/java-archive";
	mipmap[".war"] = "application/java-archive";
	mipmap[".ear"] = "application/java-archive";
	mipmap[".json"] = "application/json";
	mipmap[".hqx"] = "application/mac-binhex40";
	mipmap[".doc"] = "application/msword";
	mipmap[".pdf"] = "application/pdf";
	mipmap[".ps"] = "application/postscript";
	mipmap[".eps"] = "application/postscript";
	mipmap[".ai"] = "application/postscript";
	mipmap[".rtf"] = "application/rtf";
	mipmap[".m3u8"] = "application/vnd.apple.mpegurl";
	mipmap[".kml"] = "application/vnd.google-earth.kml+xml";
	mipmap[".kmz"] = "application/vnd.google-earth.kmz";
	mipmap[".xls"] = "application/vnd.ms-excel";
	mipmap[".eot"] = "application/vnd.ms-fontobject";
	mipmap[".ppt"] = "application/vnd.ms-powerpoint";
	mipmap[".odg"] = "application/vnd.oasis.opendocument.graphics";
	mipmap[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mipmap[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mipmap[".odt"] = "application/vnd.oasis.opendocument.text";
	mipmap[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mipmap[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mipmap[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mipmap[".wmlc"] = "application/vnd.wap.wmlc";
	mipmap[".wasm"] = "application/wasm";
	mipmap[".7z"] = "application/x-7z-compressed";
	mipmap[".cco"] = "application/x-cocoa";
	mipmap[".jardiff"] = "application/x-java-archive-diff";
	mipmap[".jnlp"] = "application/x-java-jnlp-file";
	mipmap[".run"] = "application/x-makeself";
	mipmap[".pl"] = "application/x-perl";
	mipmap[".pm"] = "application/x-perl";
	mipmap[".prc"] = "application/x-pilot";
	mipmap[".pdb"] = "application/x-pilot";
	mipmap[".rar"] = "application/x-rar-compressed";
	mipmap[".rpm"] = "application/x-redhat-package-manager";
	mipmap[".sea"] = "application/x-sea";
	mipmap[".swf"] = "application/x-shockwave-flash";
	mipmap[".sit"] = "application/x-stuffit";
	mipmap[".tcl"] = "application/x-tcl";
	mipmap[".tk"] = "application/x-tcl";
	mipmap[".der"] = "application/x-x509-ca-cert";
	mipmap[".pem"] = "application/x-x509-ca-cert";
	mipmap[".crt"] = "application/x-x509-ca-cert";
	mipmap[".xpi"] = "application/x-xpinstall";
	mipmap[".xhtml"] = "application/xhtml+xml";
	mipmap[".xspf"] = "application/xspf+xml";
	mipmap[".zip"] = "application/zip";
	mipmap[".bin"] = "application/octet-stream";
	mipmap[".exe"] = "application/octet-stream";
	mipmap[".dll"] = "application/octet-stream";
	mipmap[".deb"] = "application/octet-stream";
	mipmap[".dmg"] = "application/octet-stream";
	mipmap[".iso"] = "application/octet-stream";
	mipmap[".img"] = "application/octet-stream";
	mipmap[".msi"] = "application/octet-stream";
	mipmap[".msp"] = "application/octet-stream";
	mipmap[".msm"] = "application/octet-stream";
	mipmap[".mid"] = "audio/midi";
	mipmap[".midi"] = "audio/midi";
	mipmap[".kar"] = "audio/midi";
	mipmap[".mp3"] = "audio/mpeg";
	mipmap[".ogg"] = "audio/ogg";
	mipmap[".m4a"] = "audio/x-m4a";
	mipmap[".ra"] = "audio/x-realaudio";
	mipmap[".3gpp"] = "video/3gpp";
	mipmap[".3gp"] = "video/3gpp";
	mipmap[".ts"] = "video/mp2t";
	mipmap[".mp4"] = "video/mp4";
	mipmap[".mpeg"] = "video/mpeg";
	mipmap[".mpg"] = "video/mpeg";
	mipmap[".mov"] = "video/quicktime";
	mipmap[".webm"] = "video/webm";
	mipmap[".flv"] = "video/x-flv";
	mipmap[".m4v"] = "video/x-m4v";
	mipmap[".mng"] = "video/x-mng";
	mipmap[".asx"] = "video/x-ms-asf";
	mipmap[".asf"] = "video/x-ms-asf";
	mipmap[".wmv"] = "video/x-ms-wmv";
	mipmap[".avi"] = "video/x-msvideo";

	return (mipmap);
}


std::map<int, std::string>	init_response()
{
	std::map<int, std::string>	mipmap;

	mipmap.insert(std::pair<int, std::string>(100, "Continue"));
	mipmap.insert(std::pair<int, std::string>(200, "OK"));
	mipmap.insert(std::pair<int, std::string>(201, "CREATED"));
	mipmap.insert(std::pair<int, std::string>(204, "No Content"));
	mipmap.insert(std::pair<int, std::string>(300, "Multiple Choices"));   //DYNAMIC LIST
	mipmap.insert(std::pair<int, std::string>(301, "Move Permanently"));//DYNAMIC
	mipmap.insert(std::pair<int, std::string>(302, "Found"));//DYNAMIC
	mipmap.insert(std::pair<int, std::string>(303, "See other"));//DYNAMIC
	mipmap.insert(std::pair<int, std::string>(307, "Temporary Redirect"));//DYNAMIC
	mipmap.insert(std::pair<int, std::string>(308, "Permanent Redirect"));//DYNAMIC LIST
	mipmap.insert(std::pair<int, std::string>(400, "Bad Request")); //DYNAMIC
	mipmap.insert(std::pair<int, std::string>(401, "Unauthorized"));
	mipmap.insert(std::pair<int, std::string>(403, "Forbidden"));
	mipmap.insert(std::pair<int, std::string>(404, "Not Found"));
	mipmap.insert(std::pair<int, std::string>(405, "Method Not Allowed"));
	mipmap.insert(std::pair<int, std::string>(408, "Request Timeout"));
	mipmap.insert(std::pair<int, std::string>(413, "Content Too Large"));
	mipmap.insert(std::pair<int, std::string>(414, "URI Too Long"));
	mipmap.insert(std::pair<int, std::string>(415, "Unsupported Media Type"));
	mipmap.insert(std::pair<int, std::string>(417, "Expectation Failed"));
	mipmap.insert(std::pair<int, std::string>(500, "Internal Server Error")); //DYNAMIC
	mipmap.insert(std::pair<int, std::string>(501, "Not Implemented"));  //DYNAMIC
	mipmap.insert(std::pair<int, std::string>(503, "Service Unavailable")); // timeout cgi ?
	mipmap.insert(std::pair<int, std::string>(505, "HTTP Version Not Supported")); //check in parsing

	return (mipmap);
}

static std::map<int, std::string> ResponseLine = init_response();
static std::map<std::string, std::string> mimeType = init_mimes();


SingleLineResponse::SingleLineResponse(int status, const std::string& description)
{
	_status = status;
	_description = description;
	_response = "HTTP/1.1 100 Continue\r\n";
}

int	SingleLineResponse::writeResponse(std::queue<std::string>& outQueue)
{
	std::string	portion;

	while(!_response.empty())
	{
		portion = _response.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_response.erase(0, portion.size());
		portion.clear();
	}
	return (0);
}

SingleLineResponse::~SingleLineResponse(void) {}
HeaderResponse::HeaderResponse(ResHints &hints) : hints(hints)
{
	_status = hints.status;
	_description = hints.verboseError;
	addUniversalHeaders();
}

std::string	getTime()
{
	time_t	t = time(0);
	struct tm *ptime = gmtime(&t);
	char	str[30];
	strftime(str, 30, "%a, %d %b %G %T GMT", ptime);
	std::string	time(str);
	return (time);
}


HeaderResponse::~HeaderResponse(void) {}

void	HeaderResponse::addUniversalHeaders()
{
	hints.headers["Accept-Encoding"] = "identity";
	hints.headers["Accept-Ranges"] = "none";
	hints.headers["Date"] = getTime();
}

void	HeaderResponse::addHeader(std::string const &key, std::string const &value)
{
	hints.headers[key] = value;
}

void	HeaderResponse::addHintHeaders(ResHints &hints)
{
	for (std::map<std::string, std::string>::iterator it = hints.headers.begin(); it != hints.headers.end(); it++)
	{
		hints.headers.insert(std::pair<std::string, std::string>(it->first, it->second));
	}
}

void	HeaderResponse::_formatHeaders()
{
	_formated_headers += "HTTP/1.1 " + itostr(_status) + " " + ResponseLine[_status] + "\r\n";
	for (std::map<std::string, std::string>::iterator it = hints.headers.begin(); it != hints.headers.end(); it++)
	{
		_formated_headers += it->first + ": ";
		_formated_headers += it->second + "\r\n";
	}
	for (std::vector<std::string>::iterator it = hints.cookies.begin(); it != hints.cookies.end(); it++)
	{
		_formated_headers += "Set-Cookie: ";
		_formated_headers += *it + "\r\n";
	}
	_formated_headers += "\r\n";
}

int	HeaderResponse::writeResponse(std::queue<std::string>& outQueue)
{
	std::string	portion;

	_formatHeaders();
	while(!_formated_headers.empty())
	{
		portion = _formated_headers.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_formated_headers.erase(0, portion.size());
		portion.clear();
	}
	return (0);
}

FileResponse::~FileResponse(void) {}


FileResponse::FileResponse(ResHints &hints): HeaderResponse(hints)
{
	_path = hints.path;
	//_ifstream.open(_path.c_str(), std::ios_base::in | std::ios_base::binary); // le fichier devrait tjrs etre ouvrable ?
}

int	FileResponse::writeResponse(std::queue<std::string>& outQueue)
{
	std::string	portion;
	char	*buff = new char[HEADER_MAX_SIZE];
	this->_formatHeaders();
	while(!_formated_headers.empty())
	{
		portion = _formated_headers.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_formated_headers.erase(0, portion.size());
		portion.clear();
	}
	portion.clear();
	do
	{
		_fstr.read(buff, HEADER_MAX_SIZE);
		if (_fstr.gcount() == 0)
			break;
		std::string	outBuff(buff, _fstr.gcount());
		outQueue.push(outBuff);
		outBuff.clear();
	} while (_fstr.good());
	delete[] buff;
	return (0);
}



DynamicResponse::~DynamicResponse(void) {}

int	DynamicResponse::writeResponse(std::queue<std::string>& outQueue) {

	std::string	portion;
	this->_formatHeaders();
	while (!_formated_headers.empty())
	{
		portion = _formated_headers.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_formated_headers.erase(0, portion.size());
		portion.clear();
	}
	portion.clear();
	while (!_body.empty())
	{
		portion = _body.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_body.erase(0, portion.size());
		portion.clear();
	}
	return (0);
}

AResponse*	AResponse::genResponse(ResHints &hints)
{
	AResponse	*response = NULL;

	if (hints.status == 100)
	{
		response = new SingleLineResponse(hints.status, hints.verboseError);
		return (response);
	}
	else if (hints.path.empty() || hints.type == REQ_TYPE_DIR)
	{
		response = new DynamicResponse(hints);
		static_cast<DynamicResponse *>(response)->addHintHeaders(hints);
		static_cast<DynamicResponse *>(response)->addSpecificHeaders();
		static_cast<DynamicResponse *>(response)->generateBody();
	}
	else
	{
		response = new FileResponse(hints);
		static_cast<FileResponse *>(response)->addHintHeaders(hints);
		static_cast<FileResponse *>(response)->inspectFile(hints);
		static_cast<FileResponse *>(response)->addSpecificHeaders();
	}
	return (response);
}

DynamicResponse::DynamicResponse(ResHints &hints) : HeaderResponse(hints)
{
}


int	FileResponse::inspectFile(ResHints &hints)
{
	struct	stat	st;

	if (stat(hints.path.c_str(), &st)) {
		delete (this);
		throw(std::domain_error(("Can't get stat of target file: " + hints.path).c_str()));
	}
	size_t	length = st.st_size;
	length -= hints.index;
	_fstr.open(hints.path.c_str(), std::ios::in | std::ios::binary);
	if (!_fstr.is_open()) {
		delete (this);
		throw(std::domain_error(("Could not open target file: " + hints.path).c_str()));
	}
	_fstr.seekg((hints.index));
	hints.index = 0;
	hints.headers["Content-Length"] = itostr(length);
	this->checkType();
	return (0);
}


void	FileResponse::checkType()
{
	std::string	type;
	if (_retrieveType(hints.extension, type) && hints.status < 400)
	{
		hints.headers["Content-Type"] = type;
	}
}


int HeaderResponse::_retrieveHeader(std::string key, std::string &value)
{
    try{
        value = hints.headers.at(key);
        return (1);
    }
    catch(std::out_of_range &e)
    {
        value.clear();
		return (0);
    }
}

int FileResponse::_retrieveType(std::string key, std::string &value)
{
    try{
        value = mimeType.at(key);
        return (1);
    }
    catch(std::out_of_range &e)
    {
        value.clear();
		return (0);
    }
}


void	DynamicResponse::generateBody()
{
	if (hints.type == REQ_TYPE_DIR)
	{
		generateDirListing();
		return ;
	}
	switch(hints.status / 100)
	{
		case(3):
			generateRedirBody();
			break;
		default:
			generateVerboseBody();
	}
}

std::string	getRedirPath(const std::string& redir, const std::string& filename) {
	if (redir[redir.size() - 1] == '/')
		return (redir + filename);
	else
		return (redir);
};

void	DynamicResponse::generateRedirBody()
{
	switch(hints.status % 100)
	{
		case(0):
			generateListBody();
			break;
		case(8):
			generateListBody();
			break;
		default:
			generateSimpleRedirBody();
	}
}

void	DynamicResponse::generateSimpleRedirBody()
{
	std::string	line = itostr(hints.status) + " " + ResponseLine[hints.status];
	std::string	redir = getRedirPath(hints.redirList->front(), hints.parsedUri.filename);
	_body += "<html><head><title> " + line + " </title><meta http-equiv=\"refresh\"content=\"0; url=" + redir + "\"></head><body><style>";
	_body += "#one{color:darkred;text-align:center;font-size:300%;}";
	_body += "#two{text-align:center;font-size:150%;}</style>";
	_body += "<p id=\"one\">" + line + "</p>";
	_body += "<p id=\"two\"> The document can be found at <a href=\"" + redir + "\">" + redir + "</a></p></body></html>";

	addHeader("Content-Length", itostr(_body.size()));
	addHeader("Content-Type", "text/html");
}

void	DynamicResponse::generateVerboseBody()
{
	std::string	line;
	line = itostr(hints.status) + " " + ResponseLine[hints.status];
	_body += "<html><head><title> " + line + " </title></head><body><style>";
	_body += "#one{color:darkred;text-align:center;font-size:300%;}";
	_body += "#two{color:brown;text-align:center;font-size:150%;}</style>";
	_body += "<p id=\"one\">" + line + "</p>";
	_body += "<p id=\"two\">" + hints.verboseError + "</p></body></html>";
	addHeader("Content-Length", itostr(_body.size()));
	addHeader("Content-Type", "text/html");
}

void	DynamicResponse::generateListBody()
{
	std::string	line = itostr(hints.status) + " " + ResponseLine[hints.status];
	_body += "<html><head><title> " + line + " </title></head><body><style>";
	_body += "#one{color:darkred;text-align:center;font-size:300%;}";
	_body += "#two{text-align:center;font-size:150%;}</style>";
	_body += "<p id=\"one\">" + line + "</p>";
	_body += "<p id=\"two\"> The document has been moved to: </p>";
	_body += "<ul>";
	for (std::vector<std::string>::const_iterator it = hints.redirList->begin(); it != hints.redirList->end(); it++)
	{
		_body += "<li><a href=\"" + getRedirPath(*it, hints.parsedUri.filename) + "\">" + getRedirPath(*it, hints.parsedUri.filename) + "</a></li>";
	}
	_body += "</ul>";
	_body += "</body></html>";
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", itostr(_body.size()));

}

void	DynamicResponse::addSpecificHeaders()
{
	if (hints.status / 100 == 3)
		addHeader("Location",getRedirPath(hints.redirList->front(), hints.parsedUri.filename));
}


std::string	extractDir(std::string &path)
{
	std::string	dir;

	if (path.size() == 1)
		return ("root");
	size_t	idx = path.find_last_of("/", path.size() - 2);
	return (path.substr(idx + 1));

}

void	DynamicResponse::generateDirListing()
{
	std::string	dir = hints.path;
	std::string dir_name = extractDir(dir);
	DIR	*d = opendir(dir.c_str());
	if (!d)
		throw(std::exception());
	struct dirent	*files;
	_body += "<html><head><title> dir list </title></head><body><style>";
	_body += "#two{text-align:center;font-size:200%;}";
	_body += "#one{list-style-type:disc; line-height:150%;}</style>";
	_body += "<p id=\"two\"> Contents of " + dir_name +  " </p>";
	_body += "<ul id=\"one\">";
	while ((files = readdir(d)))
	{
		if (files->d_type != 8 && files->d_type != 4)
			continue;
		_body += "<li><a href = \"";
		_body += files->d_name;
		_body += (files->d_type == 4 ? "/" : "");
		_body += "\">";
		_body += files->d_name;
		_body += "</a></li>";
	}
	_body += "</ul></body></html>";
	closedir(d);
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", itostr(_body.size()));
}
