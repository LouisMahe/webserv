#ifndef ILOGGER_HPP
# define ILOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <stdarg.h>
#include <ctime>
#include <string.h>
#include <map>
#include <cstdlib>
#include <list>
#include <iomanip>
#include <Host.hpp>

#define TERM_CL_RED "\033[31m"
#define TERM_CL_GREEN "\033[32m"
#define TERM_CL_YELLOW "\033[33m"
#define TERM_CL_BLUE "\033[34m"
#define TERM_CL_MAGENTA "\033[35m"
#define TERM_CL_CYAN "\033[36m"
#define TERM_CL_WHITE "\033[37m"
#define TERM_CL_RESET "\033[0m"
#define TERM_CL_BK_GREEN "\033[42m"
#define TERM_CL_BK_RED "\033[41m"
#define TERM_CL_BOLD "\033[1m"

enum LOG_LVL
{
	LOG_FILE = -1,
	LOG_DISABLE,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
	LOG_VERBOSE
};

#define LOG_LVL_MAX LOG_VERBOSE
#define LOG_AUTO_LOGFILE 0 //A logfile will be generated without calling openLogFile()
#define LOG_DFT_LOGFILE_PATH "logs/webserv.log"
#define LOG_LOGFILE_COLOR 0 //Toggle colors inside log file
#define LOG_DFT_OPEN_MODE (std::ios_base::trunc)

#define LOG_CONFIG_ARRAY_VERBOSE {true, true, true, true, true}
#define LOG_CONFIG_ARRAY_DEBUG {true, true, true, true, false}
#define LOG_CONFIG_ARRAY_INFO {true, true, true, false, false}
#define LOG_CONFIG_ARRAY_WARNING {true, true, false, false, false}
#define LOG_CONFIG_ARRAY_ERROR {true, false, false, false, false}
#define LOG_CONFIG_ARRAY_DISABLE {false, false, false, false, false}

#define LOG_DFT_LVL_CONSOLE LOG_CONFIG_DEBUG
#define LOG_DFT_LVL_LOGFILE LOG_CONFIG_INFO

#define LOG_ERROR_BIT (0)
#define LOG_WARNING_BIT (1)
#define LOG_INFO_BIT (2)
#define LOG_DEBUG_BIT (3)
#define LOG_VERBOSE_BIT (4)
#define LOG_COLORIZE_BIT (5)

#define LOG_ERROR_MSK (1 << LOG_ERROR_BIT)
#define LOG_WARNING_MSK (1 << LOG_WARNING_BIT)
#define LOG_INFO_MSK (1 << LOG_INFO_BIT)
#define LOG_DEBUG_MSK (1 << LOG_DEBUG_BIT)
#define LOG_VERBOSE_MSK (1 << LOG_VERBOSE_BIT)
#define LOG_COLORIZE_MSK (1 << LOG_COLORIZE_BIT)

#define LOG_CONFIG_VERBOSE (LOG_ERROR_MSK | LOG_WARNING_MSK | LOG_INFO_MSK | LOG_DEBUG_MSK | LOG_VERBOSE_MSK)
#define LOG_CONFIG_DEBUG (LOG_ERROR_MSK | LOG_WARNING_MSK | LOG_INFO_MSK | LOG_DEBUG_MSK)
#define LOG_CONFIG_INFO (LOG_ERROR_MSK | LOG_WARNING_MSK | LOG_INFO_MSK)
#define LOG_CONFIG_WARNING (LOG_ERROR_MSK | LOG_WARNING_MSK)
#define LOG_CONFIG_ERROR (LOG_ERROR_MSK)

#define LOG_LVL_NO_CHANGE LOG_NO_CHANGE

typedef	unsigned char	uint8_t;

/// @brief Provide an interface for logging messages in console and a log file
/// provided a logging level.
class ILogger
{

	class LogStream
	{

		// struct LogStreamEntry;
		public:

				struct LogStreamEntry {
				std::ostream&				os;
				bool						levels[LOG_LVL_MAX];
				bool						colorize;
				bool						file;
			};


			bool									fastCheck(int level) const;

			void									addStream(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize, bool file);
			void									removeStream(std::ostream& os);

			void									editStream(std::ostream& os, const bool (&levels)[LOG_LVL_MAX], bool colorize);

			template<typename T>
			const LogStream&						operator<<(const T& param) const;

			template<typename T>
			void									print(const T& param, int lvl) const;

			void									colorize(const char* code, int lvl) const;

			// bool									(&getLevels(std::ostream& os) const)[LOG_LVL_MAX];
			bool*									getLevels(std::ostream& os);
			bool									getColorize(std::ostream& os);
			LogStreamEntry*							getStreamEntry(std::ostream& os);

			int										getNbr(void) const;

		private:

			std::list<LogStreamEntry>					_streams;

			std::list<LogStreamEntry>::iterator		findOs(std::ostream& os);
	

	};

	private:

		ILogger(void);
		virtual ~ILogger(void) = 0;

		static LogStream													_logStream;
		static std::list<std::ofstream*>									_files;
		static std::map<std::string, std::list<std::ofstream*>::iterator>	_fileTable;

		static std::time_t			_startTime;

		static inline void			printTimestamp(int level);

		static void					parseFormat(const char* format, va_list* args, int lvl);
		
		template <typename T>
		static inline void			print(T param, int lvl);
		template <typename T>
		static inline void			print(T* param, int lvl);
		// static inline void			print(Host* param, int lvl, int len = -1);
		// static inline void			print(Location* param, int lvl, int len = -1);
		// static inline void			print(CGIConfig* param, int lvl, int len = -1);

		template <typename T>
		static inline void			print(va_list* args, int lvl);
		static inline void			printCStr(va_list* args, int lvl);
		static inline void			printTruncStr(va_list* args, int lvl, int len = -1);

		static bool					isInit;

		static void					initDefault(void);

		static const char*			colors[LOG_LVL_MAX + 1];

	public:

		static void	log(int lvl, const char *format, ...);

		static void addStream(std::ostream& os, uint8_t flags);
		static void	addStream(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize);
		static void	removeStream(std::ostream& os);

		static int	addLogFile(const std::string& path, uint8_t flags);
		static int	addLogFile(const std::string& path, bool (&levels)[LOG_LVL_MAX], bool colorize);
		static void	removeLogFile(const std::string& path);

		static void	setLogLvl(const std::string& path, bool (&levels)[LOG_LVL_MAX], bool colorize);
		static void	setLogLvl(std::ostream& os, bool (&levels)[LOG_LVL_MAX], bool colorize);

		static void	printLogConfig(void);
		static void	clearFiles(void);

		static void	logDate(int level);
		static void	setInit(void);

};

#ifndef LOG_LVL_MAX
# define LOG_LVL_MAX (ILogger::LOG_VERBOSE)
#endif

#define LOGE(format, ...) ILogger::log(LOG_ERROR, format"\n" __VA_OPT__(,) __VA_ARGS__)
#define LOGW(format, ...) ILogger::log(LOG_WARNING, format"\n" __VA_OPT__(,) __VA_ARGS__)
#define LOGI(format, ...) ILogger::log(LOG_INFO, format"\n" __VA_OPT__(,) __VA_ARGS__)
#define LOGD(format, ...) ILogger::log(LOG_DEBUG, format"\n" __VA_OPT__(,) __VA_ARGS__)
#define LOGV(format, ...) ILogger::log(LOG_VERBOSE, format"\n" __VA_OPT__(,) __VA_ARGS__)

template<typename T>
const ILogger::LogStream&	ILogger::LogStream::operator<<(const T& param) const
{
	for (std::list<ILogger::LogStream::LogStreamEntry>::const_iterator it = _streams.begin(); it !=  _streams.end(); it++)
	{
		it->os << param;
	}
	return (*this);
}

template<typename T>
void	ILogger::LogStream::print(const T& param, int lvl) const
{
	for (std::list<ILogger::LogStream::LogStreamEntry>::const_iterator it = _streams.begin(); it !=  _streams.end(); it++)
	{
		if ((lvl == -1 && it->file) || lvl == 0 || (lvl > 0 && it->levels[lvl - 1]))
		{
			it->os << param;
		}
	}
}

template <typename T>
inline void	ILogger::print(va_list* args, int lvl)
{
	T	var = va_arg(*args, T);
	print(var, lvl);
}

template <typename T>
inline void	ILogger::print(T* param, int lvl)
{
	if (param == NULL)
		ILogger::_logStream.print("(NULL)", lvl);
	else
		ILogger::_logStream.print(*param, lvl);
}

template <typename T>
inline void	ILogger::print(T param, int lvl)
{
	ILogger::_logStream.print(param, lvl);
}

#endif
