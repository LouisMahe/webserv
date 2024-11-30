#include <ILogger.hpp>
#include <iterator>

std::list<std::ofstream*>	ILogger::_files;
std::map<std::string, std::list<std::ofstream*>::iterator>	ILogger::_fileTable;
ILogger::LogStream			ILogger::_logStream;
std::time_t					ILogger::_startTime = time(NULL);
bool						ILogger::isInit = false;
const char*					ILogger::colors[LOG_LVL_MAX + 1] = {
								TERM_CL_WHITE, //0 - Ignore level
								TERM_CL_RED, //Error
								TERM_CL_YELLOW, //Warning
								TERM_CL_GREEN, //Info
								TERM_CL_MAGENTA, //Debug
								TERM_CL_WHITE //Verbose
};


ILogger::ILogger(void) {}

void	ILogger::LogStream::addStream(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize = true, bool file = false)
{
	if (findOs(os) != _streams.end())
		return;
	LogStreamEntry	newStream = {
					.os = os,
					.levels = {},
					.colorize = colorize,
					.file = file
	};
	std::copy(&levels[0], &levels[0] + LOG_LVL_MAX, &newStream.levels[0]);
	this->_streams.push_back(newStream);
}

std::list<ILogger::LogStream::LogStreamEntry>::iterator	ILogger::LogStream::findOs(std::ostream& os)
{
	std::list<LogStreamEntry>::iterator	it;
	for (it = _streams.begin(); it != _streams.end(); it++)
	{
		if (&it->os == &os)
			break;
	}
	return (it);
}

void	ILogger::LogStream::removeStream(std::ostream& os)
{
	std::list<LogStreamEntry>::iterator	pos = findOs(os);
	if (pos != _streams.end())
		_streams.erase(pos);
}

bool	ILogger::LogStream::fastCheck(int level) const
{
	std::list<LogStreamEntry>::const_iterator	it;
	for (it = _streams.begin(); it != _streams.end(); it++)
	{
		if (it->levels[(level > LOG_LVL_MAX ? LOG_LVL_MAX : level)])
			return (true);
	}
	return (false);
}

void	ILogger::LogStream::editStream(std::ostream& os, const bool (&levels)[LOG_LVL_MAX], bool colorize)
{
	std::list<LogStreamEntry>::iterator	pos = findOs(os);
	if (pos == _streams.end())
		return;
	pos->colorize = colorize;
	std::copy(&levels[0], &levels[0] + LOG_LVL_MAX, &pos->levels[0]);
}

bool*	ILogger::LogStream::getLevels(std::ostream& os)
{
	std::list<LogStreamEntry>::iterator	pos = findOs(os);
	if (pos == _streams.end())
		return (NULL);
	return (pos->levels);
}

ILogger::LogStream::LogStreamEntry*	ILogger::LogStream::getStreamEntry(std::ostream& os)
{
	std::list<LogStreamEntry>::iterator	pos = findOs(os);
	if (pos == _streams.end())
		return (NULL);
	return (&(*pos));
}

bool	ILogger::LogStream::getColorize(std::ostream& os)
{
	std::list<LogStreamEntry>::iterator	pos = findOs(os);
	if (pos == _streams.end())
		return (false);
	return (pos->levels[0]);
}

void	ILogger::LogStream::colorize(const char* code, int lvl) const
{
	for (std::list<ILogger::LogStream::LogStreamEntry>::const_iterator it = _streams.begin(); it !=  _streams.end(); it++)
	{
		if (((lvl == -1 && it->file) || lvl == 0 || (lvl > 0 && it->levels[lvl - 1])) && it->colorize)
			it->os << code;
	}
}

inline void	ILogger::printCStr(va_list* args, int lvl)
{
	const char*	str = va_arg(*args, const char*);
	ILogger::_logStream.print(str, lvl);
}

inline void	ILogger::printTruncStr(va_list* args, int lvl, int len)
{
	std::string*	var = va_arg(*args, std::string*);
	if (len >= 0)
		ILogger::_logStream.print((var ? (*var).substr(0, len) : "(NULL)"), lvl);
	else
		ILogger::_logStream.print((var ? *var : "(NULL)"), lvl);
}

int	ILogger::LogStream::getNbr(void) const
{
	return (_streams.size());
}

/// @brief Printf equivalent without flag formats.
/// @param lvl Defines the log level of the text to be printed/written.
/// ```0``` to print to all stream. ```-1``` to print to file streams.
/// @param format 
/// @param 
/// @note ```%``` 
void	ILogger::log(int lvl, const char* format, ...)
{
	va_list	args;

	if (isInit == false)
	{
		isInit = true;
		initDefault();
	}
	if (lvl > LOG_LVL_MAX)
		lvl = LOG_LVL_MAX;
	else if (lvl < -1)
		lvl = -1;
	printTimestamp(lvl);
	va_start(args, format);
	parseFormat(format, &args, lvl);
	va_end(args);
}

void	ILogger::parseFormat(const char* format, va_list* args, int lvl)
{
	if (!format || !*format)
		return ;
	while (*format)
	{
		if (*format == '%')
		{
			switch (*(format + 1))
			{
				case '%':
					print('%', lvl);
					break;

				case 'd':
					print<int>(args, lvl);
					break;

				case 'i':
					print<int>(args, lvl);
					break;
				
				case 's':
					if (*(format + 2) == 's')
						printTruncStr(args, lvl);
					else if (*(format + 2) == 'h')
						printTruncStr(args, lvl, 10);
					else if (*(format + 2) == 'l')
						printTruncStr(args, lvl, 20);
					else
					{
						printCStr(args, lvl);
						break;
					}
					++format;
					break;

				case 'f':
					print<double>(args, lvl);
					break;

				case 'u':
					print<unsigned int>(args, lvl);
					break;

				case 'l':
					if (*(format + 2) == 'd')
						print<long>(args, lvl);
					else if (*(format + 2) == 'u')
						print<unsigned long>(args, lvl);
					else if (*(format + 2) == 'f')
						print<double>(args, lvl);
					else
						break;
					++format;
					break;

				case 'H':
					print<Host*>(args, lvl);
					break;
				
				case 'L':
					if (*(format + 2) == 'o')
						print<Location*>(args, lvl);
					else if (*(format + 2) == 's')
						print<ListenServer*>(args, lvl);
					else
						break;
					++format;
					break;

				case 'C':
					if (*(format + 2) == 'g')
						print<CGIConfig*>(args, lvl);
					else if (*(format + 2) == 'l')
						print<Client*>(args, lvl);
					else
						break;
					++format;
					break;

				default:
					return ;
					break;
			}
			++format;
		}
		else
			print(*format, lvl);
		++format;
	}
}

void	ILogger::addStream(std::ostream& os, uint8_t flags)
{
	bool	levels[LOG_LVL_MAX];

	for (uint8_t i = 0; i < LOG_LVL_MAX; i++)
		levels[i] = ((flags & (1 << i)) != 0);
	addStream(os, levels, ((flags & LOG_COLORIZE_MSK) != 0));
}

void	ILogger::addStream(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize = true)
{
	_logStream.addStream(os, levels, colorize);
}

int	ILogger::addLogFile(const std::string& path, uint8_t flags)
{
	bool	levels[LOG_LVL_MAX];

	for (uint8_t i = 0; i < LOG_LVL_MAX; i++)
		levels[i] = ((flags & (1 << i)) != 0);
	return (addLogFile(path, levels, ((flags & LOG_COLORIZE_MSK) != 0)));
}

int	ILogger::addLogFile(const std::string& path, bool (&levels)[LOG_LVL_MAX], bool colorize)
{
	std::ofstream						_emptyStream;

	if (_fileTable.find(path) != _fileTable.end())
		return (0);
	_files.push_front(new std::ofstream());
	std::ofstream&	fileStream = *_files.front();
	_fileTable[path] = _files.begin();
	// _files.insert(std::pair<char*, std::ofstream>(path, _fileStream));
	fileStream.open(path.c_str(), LOG_DFT_OPEN_MODE);
	if (fileStream.fail())
	{
		removeLogFile(path);
		LOGE("Error adding log file : unable to open %ss", &path);
		return (1);
	}
	_logStream.addStream(fileStream, levels, colorize, true);
	return (0);
}

void	ILogger::removeStream(std::ostream& os)
{
	_logStream.removeStream(os);
}

void	ILogger::removeLogFile(const std::string& path)
{
	std::map<std::string, std::list<std::ofstream*>::iterator>::iterator	posTable;
	posTable = _fileTable.find(path);
	if (posTable == _fileTable.end())
		return;
	std::list<std::ofstream*>::iterator	posList = posTable->second;
	std::ofstream* fileStream = *posList;
	removeStream(*fileStream);
	_files.remove(fileStream);
	delete (fileStream);
	_fileTable.erase(posTable);
}

void	ILogger::setLogLvl(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize = true)
{
	_logStream.editStream(os, levels, colorize);
}

void	ILogger::setLogLvl(const std::string& path, bool (&levels)[LOG_LVL_MAX], bool colorize = false)
{
	std::map<std::string, std::list<std::ofstream*>::iterator>::iterator	posTable;
	posTable = _fileTable.find(path);
	if (posTable == _fileTable.end())
		return;
	std::list<std::ofstream*>::iterator	posList = posTable->second;
	std::ofstream* fileStream = *posList;
	setLogLvl(*fileStream, levels, colorize);
}

inline void	ILogger::printTimestamp(int level)
{
	_logStream.colorize(TERM_CL_BOLD, level);
	_logStream.colorize(colors[level], level);
	_logStream.print('[', level);
	_logStream.print(std::time(NULL) - ILogger::_startTime, level);
	_logStream.print("] ", level);
	_logStream.colorize(TERM_CL_RESET, level);
}

void	ILogger::clearFiles(void)
{
	for (std::map<std::string, std::list<std::ofstream*>::iterator>::iterator it = _fileTable.begin(); it != _fileTable.end(); it++)
	{
		std::list<std::ofstream*>::iterator	posList = it->second;
		std::ofstream* fileStream = *posList;
		removeStream(*fileStream);
		_files.remove(fileStream);
		delete (fileStream);
	}
	_fileTable.clear();
}

void	ILogger::printLogConfig(void)
{
	std::cout << TERM_CL_BOLD TERM_CL_MAGENTA "------- [LOG CONFIG] -------" "\n";
	std::cout << _logStream.getNbr() << " stream(s) registered, including " << _files.size() << " file(s) :\n";
	for (std::map<std::string, std::list<std::ofstream*>::iterator>::iterator it = _fileTable.begin(); it != _fileTable.end(); it++)
	{
		std::cout << "	- " << it->first << "\n		";
		ILogger::LogStream::LogStreamEntry*	entry = _logStream.getStreamEntry(**it->second);
		std::cout << (entry->levels[0] ? " | CLIENT_MODE_ERROR" : "");
		std::cout << (entry->levels[1] ? " | WARNING" : "");	
		std::cout << (entry->levels[2] ? " | INFO" : "");
		std::cout << (entry->levels[3] ? " | DEBUG" : "");
		std::cout << (entry->levels[4] ? " | VERBOSE" : "");
		std::cout << "\n		 colorize = " << entry->colorize << std::endl;
	}
	std::cout << TERM_CL_RESET << std::endl;
}

/// @brief Default configuration. Is called at the first call to log()
/// @param  
void	ILogger::initDefault(void)
{
	addStream(std::cout, LOG_CONFIG_DEBUG | LOG_COLORIZE_MSK);
	if (LOG_AUTO_LOGFILE)
		addLogFile(LOG_DFT_LOGFILE_PATH, LOG_CONFIG_INFO);
	logDate(-1);
}

void	ILogger::logDate(int level = -1)
{
	std::time_t	time;
	char time_str[sizeof("YY-YY-MM-DD HH-MM-SS")];

	std::time(&time);
	std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H-%M-%S", std::localtime(&time));
	_logStream.colorize(TERM_CL_BOLD TERM_CL_GREEN, level);
	_logStream.print("\n	---", level);
	_logStream.print(time_str, level);
	_logStream.print("---\n\n", level);
	_logStream.colorize(TERM_CL_RESET, level);
}

void	ILogger::setInit(void)
{
	isInit = true;
}
