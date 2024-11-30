#include <iostream>
#include <ILogger.hpp>
#include <stdint.h>
#include <sys/epoll.h>
#include <ListenServer.hpp>
#include <IControl.hpp>
#include <IParseConfig.hpp>
#include <IControl.hpp>
#include <CGIProcess.hpp>

#define EPOLL_EVENT_MAX_SIZE 100

// class Pigeon
// {
// 	private:
// 		int age;
// 		int size;
// 	public:
// 		Pigeon();
// 		// virtual	~Pigeon();
// 		// virtual void	function(void);
// };

// class Zebra
// {
// 	private:
// 		int  age;
// 		unsigned long color;
// 	public:
// 		Zebra();
// 		// virtual	~Zebra();
// 		// virtual void	function(void);
// };

// Pigeon::Pigeon() {
// 	(void)age;
// 	(void)size;
// }

// // Pigeon::~Pigeon() {}
// // Zebra::~Zebra() {}

// Zebra::Zebra() {
// 	(void)age;
// 	(void)color;
// }


static void	catchSigPipe() {
	struct sigaction	action_sa;

	memset(&action_sa, 0, sizeof(struct sigaction));
	action_sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action_sa, NULL);
}

static void	initLogs(void)
{
	ILogger::addStream(std::cout, LOG_CONFIG_VERBOSE | LOG_COLORIZE_MSK);
	ILogger::addLogFile("logs/sessions.log", LOG_CONFIG_DEBUG);
	ILogger::addLogFile("logs/error.log", LOG_ERROR_MSK);
	ILogger::logDate(-1);
	ILogger::setInit();
	ILogger::printLogConfig();
}



int	main(int, char **, char **env)
{
	int	epollfd = 0, nbr_events;
	struct epoll_event	events[EPOLL_EVENT_MAX_SIZE];

	initLogs();
	catchSigPipe();
	try {
		epollfd = epoll_create(1);
		if (epollfd < 0) {
			LOGE("Fatal error : could not create epoll");
			return (IControl::cleanExit(1));
		}
		IParseConfig::parseConfigFile("conf/template.conf");
		CGIProcess::_env = env;
		IControl::_epollfd = epollfd;
		if (IControl::registerCommandPrompt())
			return (IControl::cleanExit(1));
		ListenServer::startServers(epollfd);
		LOGI("Servers have started");
		while (ListenServer::getNbrServer()) {
			nbr_events = epoll_wait(epollfd, events, EPOLL_EVENT_MAX_SIZE, 50);
			if (nbr_events) {
				if (IControl::handleEpoll(events, nbr_events) < 0)
					break;
			}
			Client::checkTO();
		}
		return (IControl::cleanExit(0));
	}
	catch(const CGIProcess::child_exit_exception &e)
	{
		LOGE("%s", e.what());
		return (IControl::cleanExit(1));
	}
	catch (const std::exception& e) {
		LOGE("%s", e.what());
		return (IControl::cleanExit(1));
	}
}
