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

//comm port
#include <termios.h>
#include <stdio.h>


#include "RetexAdapter.h"

RetexAdapter::RetexAdapter(CmdLineParser* parser)
	:Adapter(parser)
{

	m_adapterName = "Ritex";
	m_adapterVersion = "v0.1 alpha";
	m_adapterDescription = "Адаптер Ритекс";
	//base part of socket name
	m_socket = "ritex_socket";

	//populate device tree structure
	m_pDevice = new Device();

	// add device channels
	m_pDevice->AddChannel(new DeviceChannel(0, false, new AdapterParameter(1050100010, "Канал состояния", true, "X:X:X:X")));

	// we have one and only sensor
	Sensor* pSensor = new Sensor(1);

	pSensor->AddChannel(new DeviceChannel(1, false, new AdapterParameter(1050109000, "Число оборотов ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(2, false, new AdapterParameter(1050110000, "Средний ток ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(3, false, new AdapterParameter(1050111010, "Напряжение сети", true, "X:X:X:X")));

	pSensor->AddChannel(new DeviceChannel(4, false, new AdapterParameter(1050100050, "Загрузка ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(5, false, new AdapterParameter(1050100060, "Дисбаланс входных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(6, false, new AdapterParameter(1050100070, "Дисбаланс выходных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(7, false, new AdapterParameter(1050100080, "Дисбаланс выходных токов", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(8, false, new AdapterParameter(1050104000, "ТМС, температура двигателя", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(9, false, new AdapterParameter(1000601000, "Наработка оборудования НИ", true, "X:X:X:X")));

	m_pDevice->AddSensor(pSensor);
}

bool RetexAdapter::isDaemonRunning() {
	/* if PID file exists and we cannot obtain lock then daemon is running*/
	int fd = open(m_pidFileName.c_str(), O_RDWR);
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
#define SV_SOCK_PATH "ritex_socket40"
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

int RetexAdapter::OpenCommPort(std::string port, int speed)
{
	struct termios tios;
    int fd;

	fd = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
            syslog(LOG_ERR, "[COMM]: Can't open port %s\n", port.c_str());
            return -1;
    }
    //tcgetattr(fd, &(tios[TIOS_OLD])); /* save current port settings */
    memset(&tios, 0, sizeof(tios));
    switch (speed) {
    case 1200:
    	speed = B1200;
            break;
    case 2400:
    	speed = B2400;
            break;
    case 4800:
    	speed = B4800;
            break;
    case 19200:
    	speed = B19200;
            break;
    case 38400:
    	speed = B38400;
            break;
    case 57600:
    	speed = B57600;
            break;
    case 115200:
    	speed = B115200;
            break;
    default:
    	speed = B9600;
            break;
    }
    tios.c_cflag = speed | CS8 | CLOCAL | CREAD;
    tios.c_iflag = IGNPAR;
    tios.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    tios.c_lflag = 0;

    tios.c_cc[VTIME] = 1; /* inter-character timer used */
    tios.c_cc[VMIN] = 200; /* blocking read until 1000 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tios);
    return fd;

}

int RetexAdapter::DaemonLoop() {
	/*
	 * -- Before enter to the main loop
	 * 1. Create communication channel to get commands from host process
	 * 2. Build offset table for parameters enabled in DB
	 * 3. Prepare SQL query for storing data to data DB
	 * 4. Open TTY to device as very last step
	 *
	 * -- In the loop
	 * 1. wait for ping-pong cycle. set window_opened = true
	 * 2. push data got in response to the DB-writing thread
	 * 3. check if there is pending command and send if any
	 */

	pthread_t socket_thread;
	pthread_create(&socket_thread, NULL, socket_loop, NULL);


	int fd = OpenCommPort("/dev/ttyUSB0", 9600);

	if (fd < 0) {
		return -1;
	}

	// TODO: crate state machine
	while(true) {
		unsigned char address;
		unsigned short length;
		unsigned char command;
		unsigned char* buffer;
		bool isBadPacket = false;
		int result;

		isBadPacket = false;

		//sync to beginig of the packet
		do {
			result = read(fd, &address, 1);
		} while (result == 1 && address != 0x36);

		result = read(fd,&length, 2); //packet length

		result = read(fd, &command, 1); //command ID

		// simple check
		switch(command) {
		case 0x84: //1
		case 0x94: //1
		case 0x88: //1
		case 0x9C: //1
		case 0xA4: //1
			if (length != 1) {
				isBadPacket = true;
			}
			break;
		case 0x8C: //2
		case 0x80: //2
		case 0xA0: //2
			if (length != 2) {
				isBadPacket = true;
			}
			break;
		case 0x90: //4
			if (length != 4) {
				isBadPacket = true;
			}
			break;
		case 0x98: //7
			if (length != 7) {
				isBadPacket = true;
			}
			break;
		//REPLYS
		case 0x64: //0x28
		case 0x68: //0x0213
		case 0x6C: //0x2E
		case 0x0B: // 2 // ACK
		case 0x70: // 5
			break;
		default:
			isBadPacket = true;
			break;
		}

		if(isBadPacket) {
			continue;
		}

		//we passed sanity check. probably good packet.
		// read 'length' bytes +2 bytes CRC
		buffer = new unsigned char[length + 2]; // + 2 for CRC
		result = read(fd, buffer, length + 2);

		//TODO: check CRC
		//TODO: pass collected data to DB thread


	}

//	int a = 1;
//	while (1) {
//		::sleep(3);
//		::syslog(LOG_ERR, "I'm daemon running for %d sec.", a * 3);
//		a++;
//	}
	return 0;
}

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
#include "EventLoggerThread.h"
int RetexAdapter::ParentLoop(bool isCommOk) {
	printf("We are in parent process PID: %d\n", ::getpid());
//	EventLoggerThread* pLogger = new EventLoggerThread("/home/ruinmmal/workspace/ritex/data/ic_data_event3.sdb");
//	pLogger->Create();
//	pLogger->Join();
	printf("Exiting parent: PID: %d\n", ::getpid());
	return 0;
}

