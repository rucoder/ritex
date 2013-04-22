/*
 * RetexAdapter.cpp
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

//socket
#include <sys/un.h>
#include <sys/socket.h>

//pthread
#include <pthread.h>

#include "RetexAdapter.h"

RetexAdapter::RetexAdapter(CmdLineParser* parser)
	:Adapter(parser)
{

	m_adapterName = "Ritex";
	m_adapterVersion = "v0.1 alpha";
	m_adapterDescription = "Адаптер Ритекс";

	AdapterParameter* p = new AdapterParameter(1050100010, "Канал состояния", true, false, "X:X:X:X");
	AddParameter(p);

	p = new AdapterParameter(1050109000, "Число оборотов ВД", true, false, "X:X:X:X");
	AddParameter(p);

	p = new AdapterParameter(1050110000, "Средний ток ВД", true, false, "X:X:X:X");
	AddParameter(p);

	p = new AdapterParameter(1050111010, "Напряжение сети", true, false, "X:X:X:X");
	AddParameter(p);

	//set socket name
	m_socket = "ritex_socket";
}

bool RetexAdapter::isDaemonRunning() {
	/* if PID file exists and we cannot obtain lock then daemon is running*/
	int fd = open(PID_FILE, O_RDWR);
	if (fd > 0) {
		if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
			if (errno == EWOULDBLOCK) {
				return true;
			}
			// need to release lock and remove the file
		} else {
			flock(fd, LOCK_UN);
		}
	}
	return false;
}

RetexAdapter::~RetexAdapter() {
	if (m_pCmdLineParser != NULL) {
		delete m_pCmdLineParser;
		m_pCmdLineParser = NULL;
	}
}

#define BUF_SIZE 100
#define SV_SOCK_PATH "ritex_socket"
#define BACKLOG 5

void* socket_loop(void* arg) {
	struct sockaddr_un addr;
	int sfd, cfd;
	ssize_t numRead;
	char buf[BUF_SIZE];
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1)
		syslog(LOG_ERR, "socket");
	/* Construct server socket address, bind socket to it,
	 and make this a listening socket */
	//if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
	//	syslog(LOG_ERR, "remove-%s", SV_SOCK_PATH);
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(&addr.sun_path[1], SV_SOCK_PATH, sizeof(addr.sun_path) - 2);
	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		syslog(LOG_ERR, "bind");
		return NULL;
	}
	if (listen(sfd, BACKLOG) == -1) {
		syslog(LOG_ERR, "listen");
		return NULL;
	}
	for (;;) {
		/* Handle client connections iteratively */
		/* Accept a connection. The connection is returned on a new
		 socket, 'cfd'; the listening socket ('sfd') remains open
		 and can be used to accept further connections. */
		cfd = accept(sfd, NULL, NULL);
		if (cfd == -1) {
			syslog(LOG_ERR, "accept");
			return NULL;
		}
		/* Transfer data from connected socket to stdout until EOF */
		while ((numRead = recv(cfd, buf, BUF_SIZE, 0)) > 0) {
			if (numRead == EOF) {
				syslog(LOG_ERR, "EOF GOT");
				break;
			}
			syslog(LOG_ERR, "Read: %d", numRead);

			//sleep(5); for long work emulation
			//send(cfd, "REPLY", 5, 0);
			//write(cfd, buf, numRead); //echo
		}

//			if (write(STDOUT_FILENO, buf, numRead) != numRead)
//				syslog(LOG_ERR,"partial/failed write");
		if (numRead == -1)
			syslog(LOG_ERR, "write: errno=%d", errno);
		if (close(cfd) == -1)
			syslog(LOG_ERR, "close");
		//return NULL;
	}
}

int RetexAdapter::DaemonLoop() {
	pthread_t socket_thread;

	pthread_create(&socket_thread, NULL, socket_loop, NULL);

	int a = 1;
	while (1) {
		::sleep(3);
		::syslog(LOG_ERR, "I'm daemon running for %d sec.", a * 3);
		a++;
	}
	return 0;
}

/* -p
 * Ritex|v1.0.3|Адаптер Ритекс
1050100010|Канал состояния|1|0|X:X:X:X|0
1050109000|Число оборотов ВД|0|0|X:X:X:X|0
1050110000|Средний ток ВД|0|0|X:X:X:X|0
1050111010|Напряжение сети|0|0|X:X:X:X|0
1050100050|Загрузка ВД|0|0|X:X:X:X|0
1050100060|Дисбаланс входных напряжений|0|0|X:X:X:X|0
1050100070|Дисбаланс выходных напряжений|0|0|X:X:X:X|0
1050100080|Дисбаланс выходных токов|0|0|X:X:X:X|0
1050104000|ТМС, температура двигателя|0|0|X:X:X:X|0
1000601000|Наработка оборудования НИ|0|0|X:X:X:X|0
 *
 */

/*
 * -a
 * comdevice|list|/dev/ttySC0|/dev/ttySC1|/dev/ttySC2|/dev/ttySC3|/dev/ttySC4
baudrate|list|75|110|300|1200|2400|4800|9600|19200|38400|57600|115200
debug|list|0|1
address|string
com_timeout|list|50000|70000|90000|120000|150000
 */

/*
 * -cmd
 * 24|Изменить уставку|Команды управления
~1|Изменить уставку|list|1~Число оборотов ВД|2~Ток ВД при перегрузе|3~Защита по перегрузу|4~Ток ВД при недогрузе|5~Защита по недогрузу
~2|Значение уставки|float
2|Команда управления|Команды управления ВД
~1|Команды управления ВД|list|7~Левое вращения ВД|8~Правое вращения ВД|9~Включить ВД|10~начать тарировку|11~Сброс ВД|12~Выключить ВД
 */

int RetexAdapter::ParentLoop() {
	printf("We are in parent process PID: %d\n", ::getpid());
	return 0;
}

