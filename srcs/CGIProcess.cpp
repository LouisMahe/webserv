#include <CGIProcess.hpp>
#include <fstream>
#include <iostream>
#include <Request.hpp>
#include <sys/time.h>
#include <ctime>
#include <sys/wait.h>
#include <IControl.hpp>

char**  CGIProcess::_env;

CGIProcess::CGIProcess(Client& client) : _index(0), _pid(0), _status(0), _client(client), _request(*client.getRequest()) {}

CGIProcess::~CGIProcess(void) {
	unlink(_request.resHints.path.c_str());
}

int CGIProcess::parseHeaders()
{
	std::fstream    fstream(_request.resHints.path.c_str());
	char            *c_buffer = new char[HEADER_MAX_SIZE];
	fstream.read(c_buffer, HEADER_MAX_SIZE);
	std::string     buffer(c_buffer, fstream.gcount());
	delete[] c_buffer;

	while(_getLine(buffer))
	{
		if (_line.empty())
			break;
		if (_extract_header())
		{
			_request.resHints.status = 500;
			//do i need to unlink now?
			return (0);
		}
	}
	_request.resHints.index = _index;
	int res = _inspectHeaders(); // res should give the type of response document [local] redirect
	return (res); // res should allow to discriminate what to do with the request
}

int CGIProcess::_getLine(std::string &buffer)
{
	size_t  idx = buffer.find("\r\n", 0);
	if (idx == std::string::npos)
		return (0);
	_line = buffer.substr(0, idx);
	_index += idx + 2;
	buffer = buffer.substr(idx + 2, std::string::npos);
	return (1);
}

bool	isLowerEqual(const char *a, const char *b)
{
	while (*a && tolower(*a) == tolower(*b))
	{
		a++;
		b++;
	}
	return (((unsigned char) tolower(*a)) - ((unsigned char) tolower(*b)));
}

int CGIProcess::_extract_header()
{
	std::string key;
	std::string value;
	size_t  idx = _line.find(":", 0);
	if (idx == std::string::npos)
		return (500);
	key = _line.substr(0, idx);
	value = _line.substr(idx + 1, _line.size());
	while (!value.empty() && value[0] == 32)
		value.erase(0,1);
	if (!isLowerEqual(key.c_str(), "Set-Cookie"))
	{
		_request.resHints.cookies.push_back(value);
	}
	else
		_cgi_headers[key] = value;
	_line.clear();
	return (0);
}

int CGIProcess::_retrieveHeader(std::string key, std::string &value)
{
	try{
		value = _cgi_headers.at(key);
		return (1);
	}
	catch(std::out_of_range &e)
	{
		return (0);
	}
}


int CGIProcess::_inspectHeaders()
{
	ResHints&   hints = _request.resHints;
	std::string value;
	if (_cgi_headers.size() == 0)
		return (CGI_RES_ERROR);
	_retrieveHeader("Location", value);
	if (_retrieveHeader("Location", value) && value.substr(0, 7) == "http://")
	{
		hints.headers["Location"] = value;
		hints.status = 302;
		if (_retrieveHeader("Content-Type", value))
		{
			hints.headers["Content-Type"] = value;
			hints.hasBody = 1;
		}
		return (CGI_RES_CLIENT_REDIRECT);
	}
	else if (_retrieveHeader("Location", value) && value[0] == '/')
	{
		_request.setUri(value);
		_request.resHints.index = 0;
		return (CGI_RES_LOCAL_REDIRECT);
	}
	else
	{
		if (_retrieveHeader("Status", value) && getInt(value, 10, hints.status))
		{
			hints.status = 500;
			return (CGI_RES_DOC);
		}
		else
			hints.status = 200;
		if (_retrieveHeader("Content-Type", value))
		{
			hints.hasBody = 1;
			hints.headers["Content-Type"] = value;
		}
		else
			hints.hasBody = 0;
	}
	return (CGI_RES_DOC);
}

static long long	getDuration(struct timeval time)
{
	struct timeval	now;
	gettimeofday(&now, NULL);

	return (now.tv_sec * 1000 + now.tv_usec / 1000 - time.tv_sec * 1000 - time.tv_usec / 1000);
}

int	CGIProcess::checkEnd() {
	int	status;

	if (_pid == 0)
		return (-1);
	if (waitpid(_pid, &status, WNOHANG) == 0) {
		if (getDuration(_fork_time) > CGI_TIME_OUT) {
			kill(_pid, SIGKILL);
			_pid = 0;
			return (-1);
		}
		return (0);
	}
	_pid = 0;
	if (WEXITSTATUS(status) != 0 || WIFSIGNALED(status))
		return (-1); //!!TP CHANGE !!!!
	return (1);
}

/// @brief
/// @return ```0``` for success. ```-1``` if error. Child error will be handled
/// elsewhere
int CGIProcess::execCGI()
{
	int pid;
	gettimeofday(&_fork_time, NULL);
	_request.resHints.path = generate_name(NULL);
	pid = fork();
	if (pid == 0)
	{
		close(IControl::_epollfd);
		_launchCGI();
	}
	else if (pid > 0)
	{
		_pid = pid;
		_status = CHILD_RUNNING;
		return (0);
	} else if (pid < 0) {
		LOGE("FAILED TO FORK");
		return (1);
	}
	return (0);
}

static	void deleteArgv(char** argv) {
	if (argv[0])
		delete[] argv[0];
	if (argv[1])
		delete[] argv[1];
	delete[]	argv;
}

void    CGIProcess::_launchCGI()
{
	int     fd_out = open(_request.resHints.path.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
	int     fd_in;
	int		i = 0;
	char    **argv = new char*[3];
	std::string	cgi_root = _request.resHints.cgiRules->root;
	cgi_root.erase(0,1);

	memset(argv, 0, 3 * sizeof(char*));
	char	work_path[1024];
	getcwd(work_path, sizeof(work_path));
	if (!_request.resHints.bodyFileName.empty()){
		_request.resHints.bodyFileName.erase(0,1);
		_request.resHints.bodyFileName = work_path + _request.resHints.bodyFileName;
	}
	_request.resHints.scriptPath.erase(0,1);
	_request.resHints.scriptPath = work_path + _request.resHints.scriptPath;
	cgi_root = work_path + cgi_root;


	if (chdir(cgi_root.c_str())) {
		deleteArgv(argv);
		throw(CGIProcess::child_exit_exception());
	}
	argv[2] = NULL;
	if (_request.resHints.cgiRules->exec != "./"){
		argv[0] = new char[_request.resHints.cgiRules->exec.size() + 1];
		argv[0][_request.resHints.cgiRules->exec.size()] = '\0';
		std::copy(&_request.resHints.cgiRules->exec.c_str()[0], &_request.resHints.cgiRules->exec.c_str()[_request.resHints.cgiRules->exec.size()], &argv[0][0]);
		i++;
	}
	argv[i] = new char[_request.resHints.scriptPath.size() + 1];
	argv[i][_request.resHints.scriptPath.size()] = 0;
	std::copy(&_request.resHints.scriptPath.c_str()[0], &_request.resHints.scriptPath.c_str()[_request.resHints.scriptPath.size()], &argv[i][0]);
	argv[i + 1] = NULL;
	if (!_request.resHints.bodyFileName.empty())
	{
		fd_in = open(_request.resHints.bodyFileName.c_str(), O_RDONLY);
		if (fd_in < 0 || dup2(fd_in, STDIN_FILENO) < 0) {
			close (fd_in);
			deleteArgv(argv);
			throw (CGIProcess::child_exit_exception());
		}
		close(fd_in);
	}
	if (fd_out < 0 || dup2(fd_out, STDOUT_FILENO) < 0) {
		close(fd_out);
		deleteArgv(argv);
		throw (CGIProcess::child_exit_exception());
	}
	close(fd_out);
	_setVariables();
	execv(argv[0], argv);
	deleteArgv(argv);
	LOGE("The script %ss failed", &_request.resHints.scriptPath);
	throw (CGIProcess::child_exit_exception());
}

int CGIProcess::getStatus()
{
	return (_status);
}

int CGIProcess::getPID()
{
	return (_pid);
}

const char* CGIProcess::child_exit_exception::what() const throw() {

	return ("Script failed");
}

void    CGIProcess::_setVariables()
{
	std::string         str;

	setenv("REQUEST_METHOD", METHOD_STR[_request.method].c_str(), 1);
	setenv("QUERY_STRING", _request.resHints.parsedUri.query.c_str(), 1);

	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1); // which version to use ?
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("SERVER_NAME", _request.getHeader("Host").c_str(), 1);
	setenv("REMOTE_ADDR", _client.getStrAddr().c_str(), 1);
	setenv("SERVER_PORT", IntToString(_client.getAddrPort(), 10).c_str(), 1);

	if (!_request.resHints.parsedUri.pathInfo.empty())
	{
		setenv("PATH_INFO", _request.resHints.parsedUri.pathInfo.c_str(), 1);
		str = _request.resHints.cgiRules->root + _request.resHints.parsedUri.pathInfo;
		setenv("PATH_TRANSLATED", str.c_str() , 1); //TODODODODODODO
	}
	else
	{
		unsetenv("PATH_INFO");
		unsetenv("PATH_TRANSLATED");
		// setenv("PATH_INFO", NULL, 1);
		// setenv("PATH_TRANSLATED", NULL, 1);
	}
	setenv("SCRIPT_NAME", _request.resHints.scriptPath.c_str(), 1); //Not sure which variable to use
	if (_client.getBodyFile() != "" && _request.getHeader("Content-Type") != "")
		setenv("CONTENT_TYPE", _request.getHeader("Content-Type").c_str(), 1);
	else
		unsetenv("CONTENT_TYPE");
	if (_request.getHeader("Cookie") != "")
		setenv("HTTP_COOKIE", _request.getHeader("Cookie").c_str(), 1);
	else
		unsetenv("HTTP_COOKIE");
}













