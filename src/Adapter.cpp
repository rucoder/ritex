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

#include <sqlite3.h>
#include <assert.h>
#include <string>

#include "DaemonCommServer.h"
#include "Utils.h"

#define BD_MAX_CLOSE 8192

static char const *priov[] = {
"EMERG:",   "ALERT:",  "CRIT:", "ERR:", "WARNING:", "NOTICE:", "INFO:", "DEBUG:"
};

static __ssize_t writer(void *cookie, char const *data, size_t leng)
{
    (void)cookie;
    int     p = LOG_DEBUG, len;
    do len = strlen(priov[p]);
    while (memcmp(data, priov[p], len) && --p >= 0);

    if (p < 0) p = LOG_INFO;
    else data += len, leng -= len;
    while (*data == ' ') ++data, --leng;

    syslog(p, "%.*s", leng, data);
    return  leng;
}

//static int noop(void *cookie, char const *data, size_t leng) {return 0;}
static cookie_io_functions_t log_fns = {
    NULL, writer, NULL, NULL
};

void tolog(FILE **pfp)
{
    setvbuf(*pfp = fopencookie(NULL, "w", log_fns), NULL, _IOLBF, 0);
}

Adapter::Adapter(std::string name, std::string version, std::string description, CmdLineParser* parser)
	: m_adapterName(name), m_adapterVersion(version), m_adapterDescription(description),
	  m_pCmdLineParser(parser), m_pDevice(NULL), m_pCommChannel(NULL),
	  m_pidFilePath(PID_FILE_PATH), m_pEventLogger(NULL), m_pDataLogger(NULL)
{
	//generate base part of socket name
	m_socket = name + "_socket";
}

Adapter::~Adapter() {
	if (m_pDevice) {
		delete m_pDevice;
		m_pDevice = NULL;
	}
	if (m_pCommChannel) {
		delete m_pCommChannel;
		m_pCommChannel = NULL;
	}

	for(std::map<std::string, struct additional_parameter_t*>::iterator itr = m_additionalParameters.begin(); itr != m_additionalParameters.end(); itr++) {
		delete itr->second;
	}
	m_additionalParameters.clear();
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
#ifdef KSU_EMULATOR
	return CONTEXT_DAEMON;
#else
	int maxfd, fd, pid_fd;

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

	fd = open(m_pidFileName.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0);
	if (fd < 0) {
		syslog(LOG_ERR, "Cannot open PID file exclusivly %s\n", m_pidFileName.c_str());
		fd = open(m_pidFileName.c_str(), O_RDWR);
		if (fd < 0)
		{
			syslog(LOG_ERR, "Still cannot open PID file for RD %s\n", m_pidFileName.c_str());
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


	syslog(LOG_ERR, "PID: 0x%d", getpid());

	std::string sPid = itoa(getpid());
	write(fd, sPid.c_str(), sPid.length());

	return CONTEXT_DAEMON;
#endif
}

int Adapter::Run() {

	//must not ever happen. derived class MUST initialize
	assert(m_pDevice != NULL);
	// paranoid check
	assert(m_pCmdLineParser != NULL);


	//TODO: validate command according to adapter properties

	CmdLineCommand* pCmdLineCommand = m_pCmdLineParser->GetCommand();

	if(pCmdLineCommand != NULL && pCmdLineCommand->Compile(GetAdditionalParameterMap())) {
#ifdef STDIO_DEBUG
		pCmdLineCommand->Dump();
#endif

		//TODO: declare as interface and move to Device class
		DeviceCommand* pDevCmd = m_pDevice->CreateCommand(pCmdLineCommand);

		if (pDevCmd == NULL) {
			syslog(LOG_ERR, "ERROR: couldn't create  DeviceCommand()\n");
			return -1;
		}

		//execute and exit
		if(!pDevCmd->isHWCommand()) {
			bool result = pDevCmd->Execute();
			delete pCmdLineCommand;
			delete pDevCmd;
			return result ? 0 : -1;
		}
		// need launch daemon and establish communication channel before execution
		else
		{
			//set device ID first so daemon will know it. It cannot be changed until daemon restart
			if(pCmdLineCommand->m_deviceId < 0) {
				syslog(LOG_ERR, "ERROR: deviceID invalid: %d\n", pCmdLineCommand->m_deviceId);
				delete pCmdLineCommand;
				delete pDevCmd;
				return -1;
			}
			m_pDevice->setDeviceId(pCmdLineCommand->m_deviceId);
			//create device-oriented socket since several adapter daemons can coexist
			m_socket+=pCmdLineCommand->m_deviceIdRaw;
			//generate PID file name
			GeneratePidFileName(pCmdLineCommand->m_deviceId);
			//TODO: how to handle -a params the best way?

			syslog(LOG_ERR, "PID FILE: %s\n", m_pidFileName.c_str());

			switch (BecomeDaemon()) {
				case CONTEXT_ERROR: //error. may happen from either child or parent so call syslog
					syslog(LOG_ERR, "Couldn't run daemon. Exiting");
					_exit(EXIT_FAILURE);
					break;
				case CONTEXT_PARENT: //we are in parent. wait for daemon init compleate and run command if neccessary
					{
						m_pCommChannel = new DaemonCommChannel();
						if(m_pCommChannel != NULL) {
#ifdef STDIO_DEBUG
							printf("opening channel socket %s\n", m_socket.c_str());
#endif
							if(m_pCommChannel->open(m_socket) == 0) {
#ifdef STDIO_DEBUG
								printf("Channel opened on socket %s\n", m_socket.c_str());
#endif
								//send raw command line
								std::string cmdLine = m_pCmdLineParser->GetCmdLine();
#ifdef STDIO_DEBUG
								printf("RAW::: %s\n", cmdLine.c_str());
#endif

								m_pCommChannel->send((unsigned char*)cmdLine.c_str(), cmdLine.length() + 1); //send sero terminated command line

								unsigned length;
								m_pCommChannel->recv(&length, 4); //length
#ifdef STDIO_DEBUG
								printf("Got reply length: %d\n", length);
#endif

								if (length > 0) {
									unsigned char* resp = new unsigned char[length];
									m_pCommChannel->recv(resp,length); //length

									//output command result
									printf("%s", resp);
									delete resp;
								}

							} else {
								syslog(LOG_ERR, "Error connecting to daemon on %s\n", m_socket.c_str());
							}
						} else {
							syslog(LOG_ERR, "OOM creating DaemonCommChannel\n");
							delete pCmdLineCommand;
							delete pDevCmd;
							return -1;
						}

						ParentLoop(m_pCommChannel != NULL && m_pCommChannel->isOpened());

						if(m_pCommChannel) {
							delete m_pCommChannel;
							m_pCommChannel = NULL;
						}
						delete pCmdLineCommand;
						delete pDevCmd;
					}
					break;
					/*
					 * !!!!!!! VERY IMPORTANT NOTE !!!!!!!
					 * Due to specific behavior of pthreads and fork()
					 * all objects inherited from Thread class must be created in
					 * CONTEXT_DAEMON
					 */
				case CONTEXT_DAEMON: //in daemon
					/*
					 * following objects do not have any sense in daemon context.
					 * they are just copies of parent objects. they actually hold
					 * command line which was passed to host process
					 * when daemon was launched. Real command will be passed over AF_UNIX socket
					 * if needed
					 */
					delete pCmdLineCommand;
					delete pDevCmd;

					//tolog(&stderr);
					//tolog(&stdout);

					syslog(LOG_ERR, "[DAEMON] Starting daemon for devId=%d", m_pDevice->getDeviceId());

					/************************ The work starts here ******************/
					assert(m_pDevice->getDeviceId() > 0);
					/*
					 * 1. create comm server
					 */

					//Update channels information from DB
					if (!UpdateParameterFilter(m_pDevice->getDeviceId())) {
						//cannot continue. cleanup and exit
						DaemonCleanup();
						syslog(LOG_ERR, "[DAEMON] Couldn't get parameter filter for devId=%d", m_pDevice->getDeviceId());
						break;
					}


					/*
					 * Now create all necessary communication facilities:
					 * - EventLogger
					 * - DataLogger
					 *
					 */

					if(CreateLoggerFacility()) {
						/*
						 * now create server socket and start listening. there must be a command alredy
						 * which initiated daemonization! Here we have to go to some kind of endless loop
						 * so we go into command processing loop
						 */
						syslog(LOG_ERR, "[DAEMON] Opening server socket.. ");
						DaemonCommServer* pServer = new DaemonCommServer(m_pDevice, this);
						pServer->Create();
						syslog(LOG_ERR, "[DAEMON] Waiting for Server to complete");
						pServer->Join();//stuck here till the end of daemon live
						syslog(LOG_ERR, "[DAEMON] Server [DONE]");
						delete pServer;
						DaemonCleanup();
					} else {
						syslog(LOG_ERR, "[DAEMON] Couldn't create data loggers for devId=%d", m_pDevice->getDeviceId());
					}

					break;
			}

		}
	}

	return 0;
}

/*
 * works in daemon context
 */
bool Adapter::UpdateParameterFilter(int devId)
{
    sqlite3* pDb;

    syslog(LOG_ERR, "[SQL]: getting filter for device %d", devId);

#if defined(KSU_EMULATOR) || defined(RS485_ADAPTER)
    std::string dbPath = "/home/ruinmmal/workspace/ritex/data/ic_data3.sdb";
#else
    std::string dbPath = "/mnt/www/ControlServer/data/ic_data3.sdb";
#endif


    int rc = sqlite3_open_v2(dbPath.c_str(), &pDb,SQLITE_OPEN_READONLY, NULL);

    if(rc != SQLITE_OK)
    {
    	syslog(LOG_ERR, "[SQL] couldn't open DB %s\n", dbPath.c_str());
    	sqlite3_close(pDb);
    	return false;
    }

    sqlite3_stmt* pStm;

    char* query = new char[1024];

    if (query == NULL) {
    	syslog(LOG_ERR, "[SQL] OOM creating query\n");
    	sqlite3_close(pDb);
    	return false;
    }
    snprintf(query, 1024, "select ParamId,ChanelId from tblChanelInfo where DeviceId == %d AND isOn == 1", devId);

    if ((rc = sqlite3_prepare_v2(pDb, query, - 1, &pStm, NULL)) == SQLITE_OK) {
    	if (pStm != NULL) {
    		int cols = sqlite3_column_count(pStm);
    		syslog(LOG_ERR, "[SQL] cols=%d\n", cols);
    		while (sqlite3_step(pStm) ==  SQLITE_ROW) {
    			int paramId = sqlite3_column_int(pStm, 0);
    			int channelId = sqlite3_column_int(pStm, 1);
    			syslog(LOG_ERR, "Add channel to filter: P:%d C:%d", paramId, channelId);
    			m_paramFilter.AddItem(channelId, paramId);
    		}
    		sqlite3_finalize(pStm);
    	} else {
    		syslog(LOG_ERR, "[SQL] error preparing %d %s for DB: %s\n", rc, sqlite3_errmsg(pDb), dbPath.c_str());
    	}
    } else {
    	syslog(LOG_ERR, "[SQL] error preparing %d %s for DB: %s\n", rc, sqlite3_errmsg(pDb), dbPath.c_str());
    }
    delete [] query;
    return true;
}

void Adapter::GeneratePidFileName(int deviceId)
{
	char filename[32];
	snprintf(filename, 32, "%s-%d.pid",m_adapterName.c_str(), deviceId);
	m_pidFileName = m_pidFilePath+filename;
}

void Adapter::DeletePidFile()
{
	unlink(m_pidFileName.c_str());
}

int Adapter::ParentLoop(bool isCommOk)
{
	//dummy implementation. does nothing
	return 0;
}

bool Adapter::CreateLoggerFacility()
{

#if defined(KSU_EMULATOR) || defined(RS485_ADAPTER)
	m_pEventLogger = new EventLoggerThread("/home/ruinmmal/workspace/ritex/data/ic_data_event3.sdb");
	m_pDataLogger = new DataLoggerThread("/home/ruinmmal/workspace/ritex/data/ic_data_value3.sdb");
#else
	m_pEventLogger = new EventLoggerThread("/mnt/www/ControlServer/data/ic_data_event3.sdb");
	m_pDataLogger = new DataLoggerThread("/mnt/www/ControlServer/data/ic_data_value3.sdb");
#endif

	if(m_pEventLogger && m_pDataLogger) {
		if(m_pEventLogger->Create() && m_pDataLogger->Create()) {
			return true;
		}
	}

	if(m_pEventLogger) {
		delete m_pEventLogger;
		m_pEventLogger = NULL;
	}
	if(m_pDataLogger) {
		delete m_pDataLogger;
		m_pDataLogger = NULL;
	}
	return false;
}

bool Adapter::AddAdditionalParameter(std::string name, int a, int b)
{
	struct additional_parameter_t* param = new additional_parameter_t();
	param->value.i.min = a;
	param->value.i.max = b;
	param->type = PARAM_TYPE_INT;
	m_additionalParameters[name] = param;
	return true;
}

bool Adapter::AddAdditionalParameterFloat(std::string name, float a, float b)
{
	struct additional_parameter_t* param = new additional_parameter_t();
	param->value.f.min = a;
	param->value.f.max = b;
	param->type = PARAM_TYPE_FLOAT;
	m_additionalParameters[name] = param;
	return true;
}

bool Adapter::AddAdditionalParameter(std::string name, std::string value)
{
	struct additional_parameter_t* param = new additional_parameter_t();
	param->value.str = strdup(value.c_str());
	param->type = PARAM_TYPE_STRING;
	m_additionalParameters[name] = param;
	return true;
}

bool Adapter::AddAdditionalParameter(std::string name, const char* const list[], int size, eListValueType type)
{
	struct additional_parameter_t* param = new additional_parameter_t();
	param->value.list.data = list;
	param->value.list.size = size;
	param->value.list.dataType = type;
	param->type = PARAM_TYPE_LIST;
	m_additionalParameters[name] = param;
	return true;
}

void Adapter::DaemonCleanup() {
	if (m_pDevice) {
		delete m_pDevice;
		m_pDevice = NULL;
	}
	if(m_pDataLogger) {
		delete m_pDataLogger;
		m_pDataLogger = NULL;
	}
	if(m_pEventLogger) {
		delete m_pEventLogger;
		m_pEventLogger = NULL;
	}
	DeletePidFile();
}


