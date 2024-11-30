#include <fstream>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 500
#define PAUSE_TIME 0
#define ADDRESS "127.0.0.1"
#define PORT 8000

// enum STATUS {SENDING_HEADER, RECEIVING_CONTINUE, SENDING_BODY};

// static char*	bodyFile = NULL;
// int				status = SENDING_BODY;
// const char		continueBuffer[] = "HTTP/1.1 100 Continue";

int	error(const char* context) {
	std::cout << context << std::endl;
	return (1);
}

bool	checkTimer(void) {
	static struct timeval		lastTime = {.tv_sec = 0, .tv_usec = 0};
	struct timeval				currentTime;
	long long					uDiff = 0;

	gettimeofday(&currentTime, NULL);
	if (currentTime.tv_sec == lastTime.tv_sec)
		uDiff = currentTime.tv_usec - lastTime.tv_usec;
	else if (currentTime.tv_sec > lastTime.tv_sec)
		uDiff = (1000000 - lastTime.tv_usec) + (1000000 * (currentTime.tv_sec - lastTime.tv_sec - 1)) \
				+ currentTime.tv_usec;
	if (uDiff >= PAUSE_TIME) {
		lastTime = currentTime;
		return (true);
	}
	return (false);
}

int	sendFile(std::ifstream& infile, int sock) {
	char	buffer[BUFFER_SIZE];
	int		nread, nwrite;

	nread = infile.readsome(buffer, BUFFER_SIZE);
	if (nread <= 0)
		return (-1);
	nwrite = write(sock, buffer, nread);
	if (nwrite < 0) {
		perror("");
		// recv(sock, NULL, 10, 0);
		return (error("write error"));
	}
	else if (nwrite != nread)
		std::cout << "warning: number of written characters is different" \
			"from number of read characters" << std::endl;
	else if (infile.eof())
		return (-1);
	else if (infile.bad())
		return (error("infile error"));
	usleep(PAUSE_TIME);
	return (0);
}

int	readSock(int sock) {
	static std::string	response;
	char		buffer[BUFFER_SIZE]; 
	int			nread = 0;
	if ((nread = read(sock, buffer, BUFFER_SIZE - 1)) < 0)
		return (error("reading socket"));
	else if (nread == 0)
		return (-1);
	buffer[nread] = '\0';
	printf("%s", buffer);
	fflush(stdout);
	if (nread != BUFFER_SIZE)
		printf("\n");
	// if (status == RECEIVING_CONTINUE) {
	// 	response += std::string(buffer, nread);
	// 	if (response.find("HTTP/1.1 100 Continue") != std::string::npos) {
	// 		printf("Received 100-Continue status, resume sending body\n");
	// 		status = SENDING_BODY;
	// 	}
	// }
	return (0);
}

int	openFile(std::ifstream& infile,  char* path) {
	infile.open(path);
	if (infile.is_open() == false || infile.bad())
		return (1);
	return (0);
}

int	connectSock(int* sock) {
	struct sockaddr_in	addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, ADDRESS, &addr.sin_addr) <= 0)
		return (error("invalid address"));
	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock < 0)
		return (error("Failed to create socket"));
	if (connect(*sock, (struct 	sockaddr*)&addr, sizeof(addr)) < 0)
		return (error("failed to connect"));
	return (0);
}

int	registerToEpoll(int epollfd, int fd, int flags) {
	struct epoll_event	event;

	event.events = flags;
	event.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event))
		return (-1);
	return (0);
}

int	changeEpoll(int epollfd, int fd, int flags) {
	struct epoll_event	event;

	event.events = flags;
	event.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) < 0)
		return (-1);
	return (0);
}

int	readStdin(std::ifstream& infile, int sock, int epollfd) {
	char	line[128];
	size_t	nread = 0;

	nread = read(STDIN_FILENO, line, 127);
	if (nread <= 0)
		return (1);
	line[nread - 1] = '\0';
	infile.close();
	if (openFile(infile, line))
		printf("could not open file (%s)\n", line);
	if (changeEpoll(epollfd, sock, EPOLLIN | EPOLLHUP | EPOLLOUT))
		return (1);
	return (0);
}

int	handleEpollIn(int epollfd, epoll_event* event, int sock, std::ifstream& infile) {
	if (event->data.fd == STDIN_FILENO) {
		if (readStdin(infile, sock, epollfd)) {
			close (sock);
			return (-1);
		}
			
	} else {
		switch (readSock(sock))
		{
			case -1:
				printf("Read 0 characters. Closing connection\n");
				close(sock);
				return (-1);

			case 1:
				return (1);
		}
	}
	return (0);
}

int	handleEpollOut(int epollfd, epoll_event* event, int sock, std::ifstream& infile) {
	if (checkTimer() == false)
		return (0);
	switch (sendFile(infile, sock))
	{
		case -1: //File is fully sent
			if (changeEpoll(epollfd, sock, EPOLLIN | EPOLLHUP))
				return (1);
			break;

		case 1:
			return (1);
	}
	return (0);
}

int	epoll_loop(int epollfd, int sock, std::ifstream& infile) {
	struct	epoll_event	events[10];
	int					nbr_events = 0, res = 0;

	while ((nbr_events = epoll_wait(epollfd, events, 10, -1)) >= 0) {
		for (int i = 0; i < nbr_events; i++) {
			if (events[i].events & EPOLLOUT) {
				if (handleEpollOut(epollfd, &events[i], sock, infile))
					return (1);
			}
			if (events[i].events & EPOLLIN) {
				if ((res = handleEpollIn(epollfd, &events[i], sock, infile)) > 0)
					return (1);
				else if (res < 0)
					return (0);
			}
			if (events[i].events & EPOLLHUP) {
				close(sock);
				printf("EPOLLHUP !");
				return (0);
			}
		}
	}
	return (nbr_events);
}

int	main(int argc, char** argv) {
	int				sock = -1, epollfd = -1, res = 0;

	std::ifstream	infile;
	if (argc < 2)
		return (error("invalid number of arguments"));
	if (openFile(infile, argv[1]))
		return (error("failed to open file"));
	if (connectSock(&sock))
		return (1);
	if ((epollfd = epoll_create(1)) < 0)
		return (error("failed to create epoll"));
	if (registerToEpoll(epollfd, sock, EPOLLIN | EPOLLOUT | EPOLLHUP))
		return (error("failed to register to epollout"));
	if (registerToEpoll(epollfd, STDIN_FILENO, EPOLLIN))
		return (error("failed to register to stdin"));
	if (epoll_loop(epollfd, sock, infile))
		return (error("error in loop"));
	close (epollfd);
	return (0);
}