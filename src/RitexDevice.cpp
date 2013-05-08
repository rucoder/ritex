/*
 * RitexDevice.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "RitexDevice.h"
#include "CmdStartMeasurement.h"
#include "CmdTestDevice.h"
#include "CmdExternal.h"
#include "Utils.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

struct setting_t {
	const char* m_name;
	const char* m_suffix;
	int m_id;
	int m_offset;
	bool m_isVisible;
};

static setting_t all_Settings[] = {
	{"Частота вращения", " об/мин", 31, 0, true},
	{"Ток ВД при перегрузе", " А", 32, 0, true},
	{"Защита по перегрузу", " %", 33, 0, true},
	{"Время блокировки защиты", " сек", 34, 0, true},
	{"Время блокировки АПВ", " мин", 35, 0, true},
	{"Количество перезапусков", "", 36, 0, true},
	{"Время, в течение которого разрешены перезапуски", " мин", 37, 0, true},
	{"Ток ВД при недогрузе", " А", 38, 0, true},
	{"Защита по недогрузу", " %", 39, 0, true},
	{"Время блокировки защиты", " сек", 40, 0, true},
	{"Время блокировки АПВ", " мин", 41, 0, true},
	{"Куст", " №", 42, 0, true},
	{"Скважина", " №", 43, 0, true},
	{"Пароль", "", 44, 0, true},
	{"Служебный режим", "", 45, 0, true},
	{"Время работы", " мин", 46, 0, true},
	{"Время паузы", " мин", 47, 0, true},
	{"Шаг регистрации параметров", " сек", 48, 0, true},
	{"Текущее время (часы", " минуты)", 49, 0, true},
	{"Текущая дата (число", " месяц)", 50, 0, true},
	{"Текущая дата (год)", "", 51, 0, true},
	{"Напряжение вторичной обмотки трансформатора", " В", 52, 0, true},
	{"Время блокировки запуска после включения питания", " мин", 53, 0, true},
	{"Защита по Rиз: 0 – включена", " 1 – выключена", 54, 0, true},
	{"Дисбаланс по U вх.   лин.", "", 55, 0, true},
	{"Время срабатывания защиты по дисбалансу U вх.  лин.", "", 56, 0, true},
	{"Дисбаланс по U вых. лин.", "", 57, 0, true},
	{"Время срабатывания защиты по дисбалансу U вых. лин.", "", 58, 0, true},
	{"Дисбаланс по  I  вых. фаз.", "", 59, 0, true},
	{"Время срабатывания защиты по дисбалансу I  вых. фаз", "", 60, 0, true},
	{"Дополнительная настройка", "", 61, 0, true},
	{"Работа с ТМС", "", 62, 0, true},
	{"Предельная температура ВД ", "", 63, 0, true},
	{"Давление жидкости на приеме насоса ", "", 64, 0, true},
	{"Изменение скорости вращения ВД при регулировании по давлению жидкости на приеме насоса", "", 65, 0, true},
	{"Предел снижения скорости вращения ВД **", "", 66, 0, true},
	{"Смена пароля инженера", "", 67, 0, true},
	{"Смена пароля инженера–наладчика", "", 68, 0, true},
	{"Тест станции", "", 69, 0, true},
	{"Восстановление заводских уставок", "", 70, 0, true},
};

#define NUMBER_OF_SETTINGS (sizeof(all_Settings) / sizeof(setting_t))


typedef struct {
	int m_code;
	const char* m_text;
} fault_code_t;

static fault_code_t fault_codes[] = {
	{0,"Авария устранена"},
	{1,"Ошибка связи ПИУ-ЦУУ"},
	{2,"Ошибка связи ЦУУ-ПЧ"},
	{3,"Ошибка связи ЦУУ-ПЧ"},
	{4,"Ошибка связи ЦУУ-ТМС"},
	{20,"Ошибка ввода уставки"},
	{21,"Не введен пароль"},
	{22,"Ввод уставки запрещен"},
	{30,"Защита по I GND (Аварийный останов для СУПН-М)"},
	{31,"Защита по U макс."},
	{32,"Защита драйвера ШИМ пли КОМ"},
	{33,"Защита по I макс."},
	{10,"Неисправность БЦУУ"},
	{40,"Неисправность ПЧ"},
	{41,"Неисправность вентиляторов ПЧ"},
	{42,"Неисправность датчика Rиз"},
	{43,"Неисправность ТМС"},
	{44,"Неисправность ПИнС"},
	{50,"Обрыв фазы"},
	{51,"Напряжение сети ниже допуска"},
	{52,"Напряжение сети выше допуска"},
	{53,"Дисбаланс входных напряжений"},
	{54,"Дисбаланс выходных напряжений"},
	{55,"Дисбаланс выходных токов"},
	{56,"Перегруз по току"},
	{57,"Недогруз по току"},
	{58,"Турбинное вращение"},
	{59,"rиз ниже допуска"},
	{60,"Давление на устье выше допуска"},
	{61,"Давление на устье ниже допуска"},
	{62,"Давление на приеме насоса ниже допуска"},
	{63,"Температура двигателя выше допуска"},
	{64,"Вибрация выше допуска"},
	{65,"Программный сбой"}
};

#define NUMBER_OF_FAULT_CODES (sizeof(fault_codes) / sizeof(fault_code_t))


struct controller_data_t {
	int m_offset;
	int m_size;
	int m_cmd;
	controller_data_t(int cmd, int offset, int size) {
		m_cmd = cmd;
		m_offset = offset;
		m_size = size;
	}
	virtual ~controller_data_t() {}
};

RitexDevice::RitexDevice(IAdapter* pAdapter)
	: Device(pAdapter), m_writeMode(WRITE_MODE_0), m_timeDiviation(DEFAULT_TIME_DIVIATION)
{
	// add device channels
	AddChannel(new DeviceChannel(0, false, new AdapterParameter(1050100010, "Канал состояния", true, "X:X:X:X")));

	// we have one and only sensor
	Sensor* pSensor = new Sensor(1);

	pSensor->AddChannel(new DeviceChannel(1, false, new AdapterParameter(1050109000, "Число оборотов ВД", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,2,2))));


	pSensor->AddChannel(new DeviceChannel(2, false, new AdapterParameter(1050110000, "Средний ток ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(3, false, new AdapterParameter(1050111010, "Напряжение сети", true, "X:X:X:X")));

	pSensor->AddChannel(new DeviceChannel(4, false, new AdapterParameter(1050100050, "Загрузка ВД", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,7,1))));

	pSensor->AddChannel(new DeviceChannel(5, false, new AdapterParameter(1050100060, "Дисбаланс входных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(6, false, new AdapterParameter(1050100070, "Дисбаланс выходных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(7, false, new AdapterParameter(1050100080, "Дисбаланс выходных токов", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(8, false, new AdapterParameter(1050104000, "ТМС, температура двигателя", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(9, false, new AdapterParameter(1000601000, "Наработка оборудования НИ", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,33,2))));

	AddSensor(pSensor);

	// "change setting command group"
	AdapterCommand* pCmd = new AdapterCommand(1, "Изменить уставку", "Изменение уставок");

	std::vector<AdapterCommandArgParam*> params;

	for(unsigned int i = 0; i < NUMBER_OF_SETTINGS; i++) {
		params.push_back(new AdapterCommandArgParam(all_Settings[i].m_name, new cmd_template_t(REQ_SETTING_SET, all_Settings[i].m_id)));
	}

	//argument order makes big difference!!!
	pCmd->AddArgument("уставка", params);
	pCmd->AddArgument("значение уставки", 0.0, 0.0);

	AddExternalCommand(pCmd);

	// "engine control"
	pCmd = new AdapterCommand(2, "Управление ВД", "Команды управления ВД");

	std::vector<AdapterCommandArgParam*> params1;

	params1.push_back(new AdapterCommandArgParam("Включить ВД", new cmd_template_t(REQ_VD_ON, VD_ON_START)));
	params1.push_back(new AdapterCommandArgParam("Выключить ВД", new cmd_template_t(REQ_VD_OFF)));
	params1.push_back(new AdapterCommandArgParam("Сброс", new cmd_template_t(REQ_VD_ON, VD_ON_RESET)));
	params1.push_back(new AdapterCommandArgParam("Начать тарировку", new cmd_template_t(REQ_VD_ON, VD_ON_TUNE)));
	params1.push_back(new AdapterCommandArgParam("Левое вращение", new cmd_template_t(REQ_VD_ROTATION, VD_ROTATE_LEFT)));
	params1.push_back(new AdapterCommandArgParam("Правое вращение", new cmd_template_t(REQ_VD_ROTATION, VD_ROTATE_RIGHT)));

	pCmd->AddArgument("Команда ВД", params1);
	AddExternalCommand(pCmd);


	m_pProcessor = new ComTrafficProcessor(this);
}

RitexDevice::~RitexDevice() {
	if(m_pProcessor) {
		delete m_pProcessor;
		m_pProcessor = NULL;
	}
}

DeviceCommand* RitexDevice::CreateExternalCommand(CmdLineCommand* cmd)
{
	//we need COM name and COM speed
	std::string comm;
	int speed;
	cmd->GetParameter("comdevice", comm);
	cmd->GetParameter("baudrate", speed);

	syslog(LOG_ERR, "comdevice: %s", comm.c_str());
	syslog(LOG_ERR, "baudrate: %d", speed);


	unsigned short param2 = 0;
	std::vector<AdapterCommand*>& list = GetExternaCommandList();

	printf( "List size: %d\n",list.size());

	assert((cmd->m_cmdType > 0) && (cmd->m_cmdType -1 < list.size()));

	AdapterCommand* pCmd = list[cmd->m_cmdType-1];

	printf( "Param count: %d\n",pCmd->m_args.size());
	//now parse "message". it must have the same number of arguments
	std::vector<std::string> params = Utils::split(cmd->m_messageRaw, '~');

	assert(pCmd->m_args.size() == params.size());

	//the first item in 'params' is a command.

	printf("First arg: %s\n", pCmd->m_args[0]->m_list_val[atoi(params[0].c_str()) - 1]->m_name.c_str());

	// setting number or command number
	cmd_template_t* tm = (cmd_template_t*)(pCmd->m_args[0]->m_list_val[atoi(params[0].c_str()) - 1]->m_deviceData);

	// if second arg exists -- get it
	if(pCmd->m_args.size() > 1) {
		param2 = (unsigned short)atof(params[1].c_str());
	}

	//	CmdExternal(RitexDevice* device, std::string commport, int speed, CmdLineCommand* cmd, unsigned short cmdId, unsigned char param1, unsigned short param2 = 0);
	CmdExternal* pExtCmd = new CmdExternal(this,comm, speed,cmd,tm->m_cmd, tm->m_param, param2);

	if (pExtCmd)
		pExtCmd->AddResultListener(this);

	return pExtCmd;

}

bool RitexDevice::ExecuteCustomCommand(DeviceCommand* pCmd, std::string com, int speed, unsigned char cmd, unsigned char p1, unsigned short p2) {

	bool result = true;
	custom_command_t* pCustomCmd = new custom_command_t();
	pCustomCmd->m_pParentCommand = pCmd;
	pCustomCmd->m_pDataPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, cmd, time(NULL));

	switch(cmd) {
	case REQ_SETTING_SET:
	{
		unsigned char* pData = pCustomCmd->m_pDataPacket->Allocate(3);
		pData[0] = p1;
		pData[1] = MSB(p2);
		pData[2] = LSB(p2);
	}
		break;
	case REQ_VD_ON:
	{
		unsigned char* pData = pCustomCmd->m_pDataPacket->Allocate(1);
		pData[0] = p1;
	}
		break;
	case REQ_VD_OFF:
		// no params
		break;
	case REQ_VD_ROTATION:
	{
		unsigned char* pData = pCustomCmd->m_pDataPacket->Allocate(1);
		pData[0] = p1;
	}
		break;
	default:
		syslog(LOG_ERR, "WARNING!!!: command not supported: 0x%X", cmd);
		delete pCustomCmd;
		return false;
	}

	if(!m_pProcessor->isRunning())
		result = m_pProcessor->Create(com, speed,true);

	if(result) {
		m_pProcessor->SendCustomCmd(pCustomCmd);
		m_pProcessor->Start();
	}

	return result;
}

DeviceCommand* RitexDevice::CreateCommand(CmdLineCommand* cmd)
{
	//handle device specific commands
	switch(cmd->m_cmdLineCommandType)
	{
	case CMD_GET_MESUREMENTS:
	{
		//we need COM name and COM speed
		std::string comm;
		int speed;
		cmd->GetParameter("comdevice", comm);
		cmd->GetParameter("baudrate", speed);
		if(cmd->GetParameter("sync_time", m_timeDiviation))
			m_timeDiviation*=60; //parameter is given in min. we operate in sec.

		syslog(LOG_ERR, "comdevice: %s", comm.c_str());
		syslog(LOG_ERR, "baudrate: %d", speed);

		CmdStartMeasurement* pCmd = new CmdStartMeasurement(this, comm, speed);

		//FIXME: do we need to log this command?
		//pCmd->AddResultListener(this);

		return pCmd;
	}
	case CMD_TEST_DEVICE:
	{
		//we need COM name and COM speed
		std::string comm;
		int speed;
		cmd->GetParameter("comdevice", comm);
		cmd->GetParameter("baudrate", speed);

		syslog(LOG_ERR, "comdevice: %s", comm.c_str());
		syslog(LOG_ERR, "baudrate: %d", speed);
		return new CmdTestDevice(this, comm, speed);
	}
	// get command from supported command table
	case CMD_COMMAND:
	{
		return CreateExternalCommand(cmd);
	}
	break;
	default:
		return Device::CreateCommand(cmd);
	}
}

void RitexDevice::CreateOffsetTable()
{
	ParameterFilter filter = m_pAdapter->GetParameterFilter();

	for(int i = 0; i < filter.GetSize(); i++) {
		int param = filter[i].m_paramId;

		//find parameter in topology
		AdapterParameter* pParam = FindGarameter(param);
		if(pParam) {
			//TODO: templetize it!!!!
			controller_data_t* pData = reinterpret_cast<controller_data_t*>(pParam->getControllerData());
			syslog(LOG_ERR,"Param: %d found. ControllerData == %p", param, pParam->getControllerData());
			if(pData) {
				//store data in offset table
				offset_table_entry_t entry;
				entry.m_channelId = filter[i].m_channelId;
				entry.m_paramId = param;
				entry.m_cmd = pData->m_cmd;
				entry.m_size = pData->m_size;
				entry.m_offset = pData->m_offset;
				m_offsetTable.push_back(entry);
			}
		}
	}

	printf("CreateOffsetTable: size = [%lu]\n", m_offsetTable.size());
}

bool RitexDevice::StartMesurements(DeviceCommand* pCmd, std::string com, int speed)
{
	if(m_pProcessor->isRunning())
		return false;
	CreateOffsetTable();
	return m_pProcessor->Create(com, speed);
}

/*
 * cannot be called when data capture is in progress
 */
bool RitexDevice::TestDevice(DeviceCommand* pCmd, std::string com, int speed)
{
	if(m_pProcessor->isRunning())
		return false;
	bool result = m_pProcessor->Create(com, speed,true);

	if(result) {
		custom_command_t* pCustomCmd = new custom_command_t();
		pCustomCmd->m_pDataPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_INFO_REQUEST_MODE_0, time(NULL));
		pCustomCmd->m_pParentCommand = pCmd;
		m_pProcessor->SendCustomCmd(pCustomCmd);
		m_pProcessor->Start();
	}

	return result;
}



void RitexDevice::ReportDataPacket(DataPacket* packet)
{
	printf("Reporting packet\n");

	for(unsigned int i = 0; i < m_offsetTable.size(); i++) {
		// parameter value is stored in this packet
		if(m_offsetTable[i].m_cmd == packet->GetCmd()) {
			DBDataPacket* event = new DBDataPacket();

			unsigned char* pData = packet->GetDataPtr();

			event->setChannelId(m_offsetTable[i].m_channelId);
			event->setParamId(m_offsetTable[i].m_paramId);
			event->setRegisterDate(packet->GetTimestamp());

			if(m_offsetTable[i].m_size == 1) {
				unsigned char value;
				memcpy(&value, pData+m_offsetTable[i].m_offset, 1);
				event->setValue((float)value);
			} else if (m_offsetTable[i].m_size == 2) {
				unsigned short value;
				memcpy(&value, pData+m_offsetTable[i].m_offset, 2);
				value = swap16(value);
				event->setValue((float)value);
			}
			m_pAdapter->getDataLogger()->EnqueData(event);
		}
	}

}

std::string RitexDevice::getFaultText(int code) {
	for (unsigned int i = 0; i< NUMBER_OF_FAULT_CODES; i++) {
		if(fault_codes[i].m_code == code)
			return std::string(fault_codes[i].m_text);
	}
	return std::string("");
}

void RitexDevice::ReportFault(int code, time_t time) {
	DBEventCommon* event = new DBEventCommon();
	event->setChannelId(1024);
	event->setRegisterTimeDate(time);
	if (code == 0) {
		event->setTypeId(25);
		//event->setArgument1(); //TODO: set prev. erro
	} else {
		event->setTypeId(8);
		event->setArgument1(getFaultText(code)); //error text
		event->setArgument2(itoa(code)); //error code
	}
	m_pAdapter->getEventLogger()->EnqueData(event);
}

void RitexDevice::CheckAndReportTimeDiviation(DataPacket* packet)
{

	time_t system_time = time(NULL);

	syslog(LOG_ERR, "[TIME] Local time: %s. epoch=%lu", Utils::TimeToString(system_time).c_str(), system_time);

	//only this packet has KSU time
	if(packet->GetCmd() == ACK_INFO_MODE_0) {
		unsigned char* p = packet->GetDataPtr();

		int year = p[21] << 8 | p[22];

		//TODO: introduce #define's
		struct tm tm;
		tm.tm_year = year - 1900;
		tm.tm_sec = 0;
		tm.tm_min = p[24];
		tm.tm_hour = p[23];
		tm.tm_mday = p[19];
		tm.tm_mon = p[20] - 1;
		tm.tm_isdst = 0;

		time_t timestamp = mktime(&tm);

		syslog(LOG_ERR, "[TIME] KSU time: %d-%d-%d %d:%d:%d . epoch=%lu Diviation [%d] sec", year, p[20],p[19], p[23],p[24],0, timestamp, system_time - timestamp);

		if(abs(system_time - timestamp) > m_timeDiviation) {
			DBEventCommon* event = new DBEventCommon();
			event->setChannelId(1024); //FIXME: temporrary workaround
			event->setTypeId(6);
			event->setRegisterTimeDate(system_time);
			event->setArgument1(itoa(abs(system_time - timestamp) / 60) );
			m_pAdapter->getEventLogger()->EnqueData(event);
		}
	}
}

void RitexDevice::OnResultReady(DeviceCommand* pCmd)
{
	syslog(LOG_ERR, "######### RitexDevice::OnResultReady");
	// add event for HW commands only
	if(pCmd->isHWCommand()) {
//		DBEventCommand* event = new DBEventCommand();
//
//		event->m_arrivalTime = pCmd->GetArrivalTime();
//		event->m_finishedTime = pCmd->GetFinishedTime();
//		event->m_cmdId = 0; //FIXME????
//		event->m_devId = getDeviceId();
//		event->m_msg = pCmd->GetParentCmd()->m_messageRaw;
//		event->m_prio = 0; //FIXME???
//		event->m_result = pCmd->getRawResult();
//		event->m_src = pCmd->GetParentCmd()->m_sourceRaw;
//		event->m_state = 0; //FIXME???
//		event->m_type = pCmd->GetParentCmd()->m_cmdType;
//
//		//m_pAdapter->getCmdLogger()->EnqueData(event);
	}
}

