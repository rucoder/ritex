/*
 * Adapter.cpp
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#include "Adapter.h"
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <pthread.h>

#include "DeviceCommandFactory.h"

#define PID_FILE "/tmp/retex.pid"
#define BD_MAX_CLOSE 8192


Adapter::Adapter(CmdLineParser* parser) :
		m_adapterName("Dummy adapter. REDEFINE ME!"), m_adapterVersion("V 0.0"), m_adapterDescription(
				"REDEFINE ME!"), m_pCmdLineParser(parser), m_pCommChannel(NULL) {
}

Adapter::~Adapter() {
	// TODO Auto-generated destructor stub
}



int fcntl_lock(int fd, int cmd, int type, int whence, int start, int len)
{
	struct flock lock[1];

	lock->l_type = type;
	lock->l_whence = whence;
	lock->l_start = start;
	lock->l_len = len;

	return fcntl(fd, cmd, lock);
}


int Adapter::LockPidFile(const char* pidfile) {
	mode_t mode = 0;//S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	mode_t newMode =  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	struct stat statbuf_fd[1], statbuf_fs[1];
	int pid_fd;
	start:

	/* This is broken over NFS (Linux). So pidfiles must reside locally. */

	if ((pid_fd = open(pidfile, O_RDWR | O_CREAT | O_EXCL, mode)) == -1) {
		if (errno != EEXIST)
			return -1;

		/*
		 ** The pidfile already exists. Is it locked?
		 ** If so, another invocation is still alive.
		 ** If not, the invocation that created it has died.
		 ** Open the pidfile to attempt a lock.
		 */

		if ((pid_fd = open(pidfile, O_RDWR)) == -1) {
			/*
			 ** We couldn't open the file. That means that it existed
			 ** a moment ago but has been since been deleted. Maybe if
			 ** we try again now, it'll work (unless another process is
			 ** about to re-create it before we do, that is).
			 */

			if (errno == ENOENT)
				goto start;

			return -1;
		}
	}

#if 0
	if (flock(pid_fd, LOCK_EX | LOCK_NB) == -1) {
		if (errno == EWOULDBLOCK) {
			close(pid_fd);
			return -1;
		}
	}

#else
	if (fcntl_lock(pid_fd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0) == -1) {
		close(pid_fd);
		return -1;
	}
	fchmod(pid_fd, newMode);
#endif

	/*
	 ** The pidfile may have been unlinked, after we opened, it by another daemon
	 ** process that was dying between the last open() and the fcntl(). There's
	 ** no use hanging on to a locked file that doesn't exist (and there's
	 ** nothing to stop another daemon process from creating and locking a
	 ** new instance of the file. So, if the pidfile has been deleted, we
	 ** abandon this lock and start again. Note that it may have been deleted
	 ** and subsequently re-created by yet another daemon process just starting
	 ** up so check that that hasn't happened as well by comparing inode
	 ** numbers. If it has, we also abandon this lock and start again.
	 */

	if (fstat(pid_fd, statbuf_fd) == -1) {
		/* This shouldn't happen */
		close(pid_fd);
		return -1;
	}

	if (stat(pidfile, statbuf_fs) == -1) {
		/* The pidfile has been unlinked so we start again */

		if (errno == ENOENT) {
			close(pid_fd);
			goto start;
		}

		close(pid_fd);
		return -1;
	} else if (statbuf_fd->st_ino != statbuf_fs->st_ino) {
		/* The pidfile has been unlinked and re-created so we start again */

		close(pid_fd);
		goto start;
	}

	return pid_fd;
}

Adapter::eExecutionContext Adapter::BecomeDaemon() {
	int maxfd, fd, pid_fd;
	char pid_s[32];

	switch (::fork()) {
	case -1:
		return CONTEXT_ERROR;
	case 0:
		break;
	default:
		return CONTEXT_PARENT;
		break;
	}

	/* Become leader of new session */
	if (setsid() == -1)
		return CONTEXT_ERROR;

	/* Ensure we are not session leader */
	switch (fork()) {
	case -1:
		return CONTEXT_ERROR;
	case 0:
		break;
	default:
		_exit(EXIT_SUCCESS); //exit from the second child
		break;
	}

	umask(0);
	chdir("/");

	//we are in daemon code now
	maxfd = sysconf(_SC_OPEN_MAX);
	if (maxfd == -1)
		/* Limit is indeterminate... */
		maxfd = BD_MAX_CLOSE;
	/* so take a guess */
	for (fd = 0; fd < maxfd; fd++)
		close(fd);


	close(STDIN_FILENO);
	/* Reopen standard fd's to /dev/null */
	fd = open("/dev/null", O_RDWR);
	if (fd != STDIN_FILENO) {
		/* 'fd' should be 0 */
		return CONTEXT_ERROR;
	}
	if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
		return CONTEXT_ERROR;
	}
	if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
		return CONTEXT_ERROR;
	}

#if 1
	fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0);
	if (fd < 0) {
		syslog(LOG_ERR, "Cannot open PID file exclusivly %s\n", PID_FILE);
		fd = open(PID_FILE, O_RDWR);
		if (fd < 0)
		{
			syslog(LOG_ERR, "Still cannot open PID file for RD %s\n", PID_FILE);
			return CONTEXT_ERROR;
		}
	}

	// file opened or created
	if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		int saveErrno = errno;
		if (errno == EWOULDBLOCK) {
			close(fd);
			syslog(LOG_ERR, "Already running as daemon.Exiting...\n");
			_exit(EXIT_SUCCESS);
		} else {
			syslog(LOG_ERR, "Couldn't lock file.Exiting...\n");
			_exit(EXIT_SUCCESS);
		}
	}

	fchmod(fd, 0600);

	snprintf(pid_s, 32, "%d\n", (int)getpid());
	syslog(LOG_ERR, "PID: 0x%d", getpid());
	write(fd, pid_s, strlen(pid_s));
#endif

#if 0
	//syslog(LOG_ERR, "PID: pint 15 %d",getpid());
	ptry(pthread_mutex_lock(&lock));

	if((pid_fd = LockPidFile(PID_FILE)) == -1 ) {
		//syslog(LOG_ERR, "Cannot lock PID file %s errno=%d\n", PID_FILE, errno);
		//syslog(LOG_ERR, "Cannot lock PID file %s errno=%d\n", PID_FILE, errno);
		if(errno == EWOULDBLOCK) {//cannot lock the file so we are running
			syslog(LOG_ERR, "Already running as daemon.Exiting...\n");
			_exit(EXIT_SUCCESS);
		}
	}
	/* Store our pid */
//	syslog(LOG_ERR, "PID: pint 16 %d",getpid());

	snprintf(pid_s, 32, "%d\n", (int)getpid());
//	syslog(LOG_ERR, "PID: pint 17 %d",getpid());

	if (write(pid_fd, pid_s, strlen(pid_s)) != strlen(pid_s))
	{
		//syslog(LOG_ERR, "PID: pint 18 %d",getpid());
		//daemon_close();
		return CONTEXT_ERROR;
	}
//	syslog(LOG_ERR, "PID: pint 19 %d",getpid());
	ptry(pthread_mutex_unlock(&lock));
#endif
	return CONTEXT_DAEMON;
}

int Adapter::Run() {
	//TODO: validate command according to adapter properties

	CmdLineCommand* pCommand = m_pCmdLineParser->GetCommand();

	if(pCommand != NULL) {
		pCommand->Dump();

		DeviceCommand* pDevCmd = DeviceCommandFactory::CreateDeviceCommand(*pCommand, this);

		//execute and exit
		if(!pDevCmd->isNeedDaemon()) {
			bool result = pDevCmd->Execute(); //blocking call
			//TODO: get command result and print here instead of execute?
			delete pDevCmd;
			return result ? 0 : -1;
		}
		// need launch daemon and establish communication channel before execution
		else
		{
			switch (BecomeDaemon()) {
				case CONTEXT_ERROR: //error. may happen from either child or parent
					::syslog(LOG_ERR, "Couldn't run daemon. Exiting");
					_exit(EXIT_FAILURE);
					break;
				case CONTEXT_PARENT: //we are in parent. wait for daemon init compleate and run command if neccessary
					{
						m_pCommChannel = new DaemonCommChannel();
						if(m_pCommChannel != NULL) {
							if(m_pCommChannel->open(m_socket) == 0) {
								printf("Channel opened\n");
								//channel opened, execute command
								//TODO: not a good idea to have implicit dependency to CommChannel
								bool result = pDevCmd->Execute();
								if(result) {
									//TODO: get return value
								}
							}
						}
						ParentLoop();
					}
					break;
				case CONTEXT_DAEMON: //in daemon
					//TODO: create communication thread
					//TODO: create RS-485 processor
					//TODO: create DB flusher
					DaemonLoop();
					break;
			}

		}


		//TODO: preprocess before become daemon ?
		//TODO: check that daemon is running?
#if 0
		switch (BecomeDaemon()) {
		case CONTEXT_ERROR: //error
			::syslog(LOG_ERR, "Couldn't run daemon. Exiting");
			_exit(EXIT_FAILURE);
			break;
		case CONTEXT_PARENT: //we are in parent. wait for daemon init compleate and run command if neccessary
			ParentLoop();
			break;
		case CONTEXT_DAEMON: //in daemon
			int a = 0;
			//TODO: create communication thread
			//TODO: create RS-485 processor
			//TODO: create DB flusher
			DaemonLoop();
			break;
		}
#endif
	}

	return 0;
}

void Adapter::printSupportedParameters()
{
    for(std::list<AdapterParameter*>::iterator itr = m_parameterList.begin(); itr != m_parameterList.end(); itr++)
    {
        std::string s = (*itr)->FormatString();
        printf("%s\n", s.c_str());
    }
}

bool Adapter::AddParameter(AdapterParameter* parameter)
{
	m_parameterList.push_back(parameter);
	return true;
}

bool Adapter::UpdateChannelList(int devId)
{
    while(!m_channelList.empty())
    {
        delete m_channelList.front();
        m_channelList.pop_front();
    }

	//TODO: get settings from DB. stub for now
    DeviceChannel* pChannel = new DeviceChannel();
    m_channelList.push_back(pChannel);
}



