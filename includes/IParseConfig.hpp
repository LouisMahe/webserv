#ifndef IPARSECONFIG_HPP
# define IPARSECONFIG_HPP

#include <Host.hpp>
#include <iostream>
#include <fstream>
#include <ILogger.hpp>
#include <sstream>
#include <string>
#include <deque>
#include <stdint.h>
#include <IControl.hpp>

class IControl;

class IParseConfig
{
	private:

		typedef void (*handleTokenFunc)(std::stringstream&, const std::string& token, void *obj);

		virtual ~IParseConfig() = 0;

		static std::ifstream	_fileStream;
		static int				_lineNbr;
		static int				_lineNbrEndOfBlock;
		static std::string		_lineBuffer;


		static bool				checkFilePath(const char* path);
		static int				openFile(const char* path);

		static int					getNextWord(std::istream& istream, std::string& word);
		static std::stringstream&	getNextBlock(std::istream& istream, std::stringstream& ostream);
		static inline void			skipSpace(std::istream& istream);
		static void					skipComment(std::istream& istream);
		static void					parseQuote(std::istream& istream, std::string& dest);
		static inline bool			checkSemiColon(std::istream& istream);

		static void				parseHostName(std::istream& istream, Host& host);
		template <typename T>
		static void				parseValues(std::istream& istream, T& words, int maxNbr = INT32_MAX);

		static void				parseBlock(std::stringstream& blockStream, void* obj, handleTokenFunc handler);
		// static void				parseHostBlock(std::stringstream& hostBlock, Host& host);

		static void				handleConfigToken(const std::string& token);
		static void				handleHostToken(std::stringstream& istream, const std::string& token, void* hostPtr);
		static void				handleCGIConfigToken(std::stringstream& istream,  const std::string& token, void* CGIConfigPtr);
		static void				handleLocationToken(std::stringstream& istream, const std::string& token, void* locationPtr);

		static void				checkHost(Host& host);
		static void				checkLocation(Location& location);
		static void				checkCGIConfig(CGIConfig& cgiConfig);
		// template <typename T>
		// static void				checkLocationNames(T container);

		static void				parseHost(std::istream& istream);
		static void				parseLocation(std::stringstream& istream, Host& host);
		static void				parseCGIConfig(std::stringstream& istream, Host& host);

		static void				parsePorts(std::istream& istream, Host& host);
		static void				parseServerName(std::istream& istream, Host& host);
		static void				parseExtension(std::istream& istream, std::string& dest);
		static void				parseBodyMaxSize(std::istream& istream, Host& host);
		static void				parseAllowedMethods(std::istream& istream, int& methods);
		static void				parseRedirection(std::istream& istream, Location& location);
		static void				parsePath(std::istream& istream, std::string& dest, const char* id = NULL, char restrictEnd = 0);
		static void				parseBoolean(std::istream& istream, bool& dest, const char* id = NULL);
		static void				parseUri(std::istream& istream, std::string& dest, const char* id = NULL);
		
		friend int				IControl::handleCommandPrompt(epoll_event* event);

	public:

		static std::string		default_error_pages;

		static int				parseConfigFile(const char* path);

		class IParseConfigException : public std::exception {
			public:
				virtual const char*	what(void) const throw() = 0;
		};

		class FileException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() = 0;
		};

		class InvalidFileTypeException : public FileException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid config file type");
				}
		};

		class FileOpenException : public FileException {
			public:
				virtual const char*	what(void) const throw() {
					return ("could not open config file");
				}
		};

		class StreamException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("error with the stream");
				}
		};

		class UnclosedQuoteException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("unclosed quotes");
				}
		};

		class UnclosedBlockException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("block has unclosed braces");
				}
		};

		class UnexpectedBraceException : public IParseConfigException {
			public:
				virtual const char* what(void) const throw() {
					return ("closing brace without prior opening brace");
				}
		};

		class LastBlockException : public IParseConfigException {
			public:
				virtual const char* what(void) const throw() {
					return ("last block reached");
				}
		};

		class MissingSemicolonException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("missing semicolon (`;')");
				}
		};

		class MissingOpeningBraceException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("missing opening brace `{'");
				}
		};

		class InvalidPortException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid port");
				}
		};

		class InvalidMaxBodySizeException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid max body size");
				}
		};

		class InvalidMethodException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid method");
				}
		};

		class InvalidBooleanException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid boolean");
				}
		};

		class InvalidRedirectionException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid redirection status");
				}
		};

		class DuplicateServerNameException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("an host with an identical server_name is already listening to this port");
				}
		};

		class UnexpectedTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				UnexpectedTokenException(const std::string& param)
					: message("unexpected token (`" + param + "')") {}
				virtual ~UnexpectedTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class MissingTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				MissingTokenException(const std::string& param)
					: message("missing token or field (`" + param + "')") {}
				virtual ~MissingTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class UnknownTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				UnknownTokenException(const std::string& param)
					: message("unknown token (`" + param + "')") {}
				virtual ~UnknownTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class TooManyValuesException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				TooManyValuesException(const std::string& param)
					: message("too many values for given token (`" + param + "')") {}
				virtual ~TooManyValuesException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

};

/// @brief Parse ```maxNbr``` words in ```istream``` and store them into a container
/// ```words``` of ```std::string```
/// @param istream
/// @param words
/// @param maxNbr
template <typename T>
void	IParseConfig::parseValues(std::istream& istream, T& words, int maxNbr)
{
	std::string	word;
	int			i = 0, ret = 0;

	do
	{
		word.clear();
		ret = getNextWord(istream, word);
		if (ret == 0)
			words.push_back(word);
		i++;
	} while (ret == 0 && i < maxNbr);
}

// template <typename T>
// void	IParseConfig::checkLocationNames(T names) {
// 	for (T::iterator it = names.begin(); it != names.end(); it++) {
// 		if ((*it)[0] == '\\')
// 			throw IParseConfig::InvalidLocationNameException(*it);
		
// 	}
// }

#endif // ICONFIG_HPP
