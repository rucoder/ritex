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
#include "Log.h"

#define DEVICE_STATE_CHANNEL_PARAM 1050100010

struct setting_t {
	const char* m_name;
	const char* m_suffix;
	int m_id;
	int m_offset;
	int m_size;
	bool m_isVisible;
	unsigned short m_value;
	bool m_isValueSet;
};

static setting_t all_Settings[] = {
	{"Частота вращения","",31,0,2, true, 0, false},
	{"Ток ВД при перегрузе","",32,2,1, true, 0, false},
	{"Защита по перегрузу","",33,3,1, true, 0, false},
	{"Время блокировки защиты по перегрузке","",34,4,2, true, 0, false},
	{"Время блокировки АПВ по перегрузке","",35,6,1, true, 0, false},
	{"Количество перезапусков","",36,7,1, true, 0, false},
	{"Время,  в течение которого разрешены перезапуски","",37,8,2,true, 0, false},
	{"Ток ВД при недогрузе","",38,10,1, true, 0, false},
	{"Защита по недогрузу","",39,11,1, true, 0, false},
	{"Время блокировки защиты по недогрузу","",40,12,2, true, 0, false},
	{"Время блокировки АПВ по недогрузу","",41,14,2, true, 0, false},
	{"Куст","",42,16,2, true, 0, false},
	{"Скважина","",43,18,2, true, 0, false},
	{"Пароль","",44,20,2, true, 0, false},
	{"Служебный режим","",45,22,1, false, 0, false},
	{"Время работы","",46,23,2, true, 0, false},
	{"Время паузы","",47,25,2, true, 0, false},
	{"Шаг регистрации параметров","",48,27,2, true, 0, false},
	{"Время (часы, минуты)","",49,-1,2,true, 0, false},
	{"Дата (число, месяц)","",50,-1,2,true, 0, false},
	{"Год","",51,-1,2, true, 0, false},
	{"Напряжение вторичной обмотки трансформатора","",52,29,2, true, 0, false},
	{"Время блокировки запуска после включения питания","",53,31,1, true, 0, false},
	{"Защита по Rиз","",54,32,1, true, 0, false},
	{"Дисбаланс по U вх.   лин.","",55,33,1, true, 0, false},
	{"Время срабатывания защиты по дисбалансу U вх.  лин.","",56,34,1, true, 0, false},
	{"Дисбаланс по U вых. лин.","",57,35,1, true, 0, false},
	{"Время срабатывания защиты по дисбалансу U вых. лин.","",58,36,1, true, 0, false},
	{"Дисбаланс по  I  вых. фаз.","",59,37,1, true, 0, false},
	{"Время срабатывания защиты по дисбалансу I  вых. фаз","",60,38,1, true, 0, false},
	{"Дополнительная настройка","",61,39,2, true, 0, false},
	{"Работа с ТМС","",62,41,1, true, 0, false},
	{"Предельная температура ВД ","",63,42,1, true, 0, false},
	{"Давление жидкости на приеме насоса ","",64,43,1, true, 0, false},
	{"Изменение скорости вращения ВД при регулировании по давлению жидкости на приеме насоса","",65,44,1, true, 0, false},
	{"Предел снижения скорости вращения ВД","",66,45,2, true, 0, false},
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
	: Device(pAdapter), m_writeMode(WRITE_MODE_0), m_timeDiviation(DEFAULT_TIME_DIVIATION), m_isDiviationReported(false),
	  m_isFirstSettingsPacket(true)
{
	// add device state channel
	AddChannel(new DeviceChannel(0, false, new AdapterParameter(DEVICE_STATE_CHANNEL_PARAM, "Канал состояния", true, "X:X:X:X", false)));

	// we have one and only sensor
	Sensor* pSensor = new Sensor(1);

	pSensor->AddChannel(new DeviceChannel(2, false, new AdapterParameter(1050100150, "тип ВД", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,2,2))));
	pSensor->AddChannel(new DeviceChannel(3, false, new AdapterParameter(1050000020, "номинальная мощность", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,4,2))));
	pSensor->AddChannel(new DeviceChannel(4, false, new AdapterParameter(1050000030, "количество пар полюсов", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,6,1))));
	pSensor->AddChannel(new DeviceChannel(5, false, new AdapterParameter(1050109020, "минимальные обороты ВД", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,7,2))));
	pSensor->AddChannel(new DeviceChannel(6, false, new AdapterParameter(1050109030, "максимальные обороты ВД", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,9,2))));
	pSensor->AddChannel(new DeviceChannel(7, false, new AdapterParameter(1050110080, "ток по АСХ З", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,11,1))));
	pSensor->AddChannel(new DeviceChannel(8, false, new AdapterParameter(1050101010, "время по АСХ 3", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,12,1))));
	pSensor->AddChannel(new DeviceChannel(9, false, new AdapterParameter(1050110090, "ток по АСХ 2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,13,1))));
	pSensor->AddChannel(new DeviceChannel(10, false, new AdapterParameter(1050101020, "время по АСХ 2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,14,1))));
	pSensor->AddChannel(new DeviceChannel(11, false, new AdapterParameter(1050110100, "ток по АСХ 1", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,15,1))));
	pSensor->AddChannel(new DeviceChannel(12, false, new AdapterParameter(1050101030, "время по АСХ 1", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_5,16,1))));
	pSensor->AddChannel(new DeviceChannel(13, false, new AdapterParameter(1050110030, "минимальный ток", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,7,1))));
	pSensor->AddChannel(new DeviceChannel(14, false, new AdapterParameter(1050110040, "максимальный ток", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,8,1))));
	pSensor->AddChannel(new DeviceChannel(15, false, new AdapterParameter(1050111080, "выходное линейное напряжение UV", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,9,2))));
	pSensor->AddChannel(new DeviceChannel(16, false, new AdapterParameter(1050111090, "выходное линейное напряжениеVW", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,11,2))));
	pSensor->AddChannel(new DeviceChannel(17, false, new AdapterParameter(1050111100, "выходное линейное напряжение WU", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,13,2))));
	pSensor->AddChannel(new DeviceChannel(18, false, new AdapterParameter(1050111110, "минимальное выходное линейное напряжение", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,15,2))));
	pSensor->AddChannel(new DeviceChannel(19, false, new AdapterParameter(1050111120, "максимальное выходное линейное напряжение", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,17,2))));
	pSensor->AddChannel(new DeviceChannel(20, false, new AdapterParameter(1050104000, "температура радиатора 1", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,25,1))));
	pSensor->AddChannel(new DeviceChannel(21, false, new AdapterParameter(1050104010, "температура радиатора 2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,26,1))));
	pSensor->AddChannel(new DeviceChannel(22, false, new AdapterParameter(1050100110, "код состояния ПЧ 0", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,27,1))));
	pSensor->AddChannel(new DeviceChannel(23, false, new AdapterParameter(1050100120, "код состояния ПЧ 1", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,28,1))));
	pSensor->AddChannel(new DeviceChannel(24, false, new AdapterParameter(1050100130, "код состояния ПЧ 2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,29,1))));
	pSensor->AddChannel(new DeviceChannel(25, false, new AdapterParameter(1050100140, "код состояния ПЧ 3", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,30,1))));

	//FIXME: these 2 must be available in all modes
	pSensor->AddChannel(new DeviceChannel(26, false, new AdapterParameter(1050100090, "Код состояния СУ", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,0,1))));
	pSensor->AddChannel(new DeviceChannel(27, false, new AdapterParameter(1050100100, "Код неисправности СУ", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,1,1))));

	pSensor->AddChannel(new DeviceChannel(29, false, new AdapterParameter(1050101000, "Время оставшееся до автозапуска (время АПВ)", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,13,2))));
	pSensor->AddChannel(new DeviceChannel(30, false, new AdapterParameter(1050110050, "Ток фазы А", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,4,1))));
	pSensor->AddChannel(new DeviceChannel(31, false, new AdapterParameter(1050110060, "Ток фазы В", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,5,1))));
	pSensor->AddChannel(new DeviceChannel(32, false, new AdapterParameter(1050110070, "Ток фазы С", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,6,1))));
	pSensor->AddChannel(new DeviceChannel(34, false, new AdapterParameter(1050111000, "Входное напряжение АВ", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,19,2))));
	pSensor->AddChannel(new DeviceChannel(35, false, new AdapterParameter(1050111010, "Входное напряжение ВC", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,21,2))));
	pSensor->AddChannel(new DeviceChannel(36, false, new AdapterParameter(1050111020, "Входное напряжение CA", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_2,23,2))));
	pSensor->AddChannel(new DeviceChannel(37, false, new AdapterParameter(1050111070, "Среднее напряжение на входе СУ", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,5,2))));
	pSensor->AddChannel(new DeviceChannel(38, false, new AdapterParameter(1050110020, "Установившееся значение тока ВД для защит по %", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,15,1))));
	pSensor->AddChannel(new DeviceChannel(39, false, new AdapterParameter(1050100060, "Дисбаланс входных напряжений", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,8,1))));
	pSensor->AddChannel(new DeviceChannel(40, false, new AdapterParameter(1050100070, "Дисбаланс выходных напряжений", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,9,1))));
	pSensor->AddChannel(new DeviceChannel(41, false, new AdapterParameter(1050100080, "Дисбаланс выходных токов", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,10,1))));
	pSensor->AddChannel(new DeviceChannel(43, false, new AdapterParameter(1050110010, "Средний ток ВД", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,4,1))));
	pSensor->AddChannel(new DeviceChannel(44, false, new AdapterParameter(1050100050, "Загрузка двигателя", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,7,1))));
	pSensor->AddChannel(new DeviceChannel(45, false, new AdapterParameter(1050112010, "Полная мощность", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,35,2))));
	pSensor->AddChannel(new DeviceChannel(46, false, new AdapterParameter(1050112020, "Активная мощность", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,18,1))));
	pSensor->AddChannel(new DeviceChannel(47, false, new AdapterParameter(1050117000, "Сопротивление изоляции, кОм", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,11,2))));
	pSensor->AddChannel(new DeviceChannel(48, false, new AdapterParameter(1050109010, "Число оборотов ВД", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,2,2))));
	pSensor->AddChannel(new DeviceChannel(50, false, new AdapterParameter(1050201000, "Общее время работы насоса, час", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,27,2))));
	pSensor->AddChannel(new DeviceChannel(51, false, new AdapterParameter(1050102010, "Общее количество запусков насоса", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,25,2))));
	pSensor->AddChannel(new DeviceChannel(52, false, new AdapterParameter(1050601010, " Время наработки станции в ч", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,31,2))));
	pSensor->AddChannel(new DeviceChannel(53, false, new AdapterParameter(1080105030, "Давление на приеме насоса (пласт. жидкость)", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,17,1))));
	pSensor->AddChannel(new DeviceChannel(54, false, new AdapterParameter(1080104010, "Температура на приеме насоса (масло двигателя), °С", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_0,16,1))));
	pSensor->AddChannel(new DeviceChannel(55, false, new AdapterParameter(1080104020, "Температура обмоток двигателя, °С", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_3,2,1))));
	pSensor->AddChannel(new DeviceChannel(56, false, new AdapterParameter(1080100009, "Вибрация насоса по оси X, м/с2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_3,4,2))));
	pSensor->AddChannel(new DeviceChannel(57, false, new AdapterParameter(1080100010, "Вибрация насоса по оси Y, м/с2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_3,8,2))));
	pSensor->AddChannel(new DeviceChannel(58, false, new AdapterParameter(1080118000, "Вибрация насоса по оси Z, м/с2", true, "X:X:X:X",
		false, new controller_data_t(ACK_INFO_MODE_3,12,2))));

	AddSensor(pSensor);

	// "change setting command group"
	AdapterCommand* pCmd = new AdapterCommand(24, "Изменить уставку", "Изменение уставок");

	std::vector<AdapterCommandArgParam*> params;

	for(unsigned int i = 0; i < NUMBER_OF_SETTINGS; i++) {
		params.push_back(new AdapterCommandArgParam(all_Settings[i].m_name, all_Settings[i].m_id, new cmd_template_t(REQ_SETTING_SET, all_Settings[i].m_id)));
	}

	//argument order makes big difference!!!
	pCmd->AddArgument("уставка", params);
	pCmd->AddArgument("значение уставки", 0.0, 0.0);

	AddExternalCommand(pCmd);

	// "engine control"
	pCmd = new AdapterCommand(2, "Управление ВД", "Команды управления ВД");

	std::vector<AdapterCommandArgParam*> params1;

	params1.push_back(new AdapterCommandArgParam("Включить ВД", 1, new cmd_template_t(REQ_VD_ON, VD_ON_START)));
	params1.push_back(new AdapterCommandArgParam("Выключить ВД", 2, new cmd_template_t(REQ_VD_OFF)));
	params1.push_back(new AdapterCommandArgParam("Сброс", 3, new cmd_template_t(REQ_VD_ON, VD_ON_RESET)));
	params1.push_back(new AdapterCommandArgParam("Начать тарировку", 4, new cmd_template_t(REQ_VD_ON, VD_ON_TUNE)));
	params1.push_back(new AdapterCommandArgParam("Левое вращение", 5, new cmd_template_t(REQ_VD_ROTATION, VD_ROTATE_LEFT)));
	params1.push_back(new AdapterCommandArgParam("Правое вращение", 6, new cmd_template_t(REQ_VD_ROTATION, VD_ROTATE_RIGHT)));

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

	Log( "comdevice: %s", comm.c_str());
	Log( "baudrate: %d", speed);


	unsigned short param2 = 0;
	std::vector<AdapterCommand*>& list = GetExternaCommandList();

#ifdef __DEBUG__
	printf( "List size: %lu\n",list.size());
#endif

	assert(cmd->m_cmdType > 0);

	AdapterCommand* pCmd = NULL;
	Log( "cmd->m_cmdId=%d", cmd->m_cmdType);

	for(unsigned int i = 0; i< list.size(); i++) {
		Log( "cmd[i].m_cmdId=%d", list[i]->m_cmdType);
		if(list[i]->m_cmdType == cmd->m_cmdType) {
			pCmd = list[i];
			break;
		}
	}

	assert(pCmd != NULL);


#ifdef __DEBUG__
	printf( "Param count: %lu\n",pCmd->m_args.size());
#endif

	//now parse "message". it must have the same number of arguments
	std::vector<std::string> params = split(cmd->m_messageRaw, '~');

	assert(pCmd->m_args.size() == params.size());

	//the first item in 'params' is a command.


	// setting number or command number. search for it
	int id = atoi(params[0].c_str());

	cmd_template_t* tm = NULL;

	for(unsigned int i = 0; i < pCmd->m_args[0]->m_list_val.size(); i++) {
		if(pCmd->m_args[0]->m_list_val[i]->m_id == id) {
			tm = (cmd_template_t*)(pCmd->m_args[0]->m_list_val[i]->m_deviceData);
#ifdef __DEBUG__
	printf("First arg: %s\n", pCmd->m_args[0]->m_list_val[i]->m_name.c_str());
#endif
			break;
		}
	}

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
	custom_command_t* pCustomCmd = new custom_command_t(*pCmd);
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
		Log( "WARNING!!!: command not supported: 0x%X", cmd);
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

		Log( "comdevice: %s", comm.c_str());
		Log( "baudrate: %d", speed);

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

		Log( "comdevice: %s", comm.c_str());
		Log( "baudrate: %d", speed);
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
			Log("Param: %d found. ControllerData == %p", param, pParam->getControllerData());
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
		custom_command_t* pCustomCmd = new custom_command_t(*pCmd);
		pCustomCmd->m_pDataPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_INFO_REQUEST_MODE_0, time(NULL));
		m_pProcessor->SendCustomCmd(pCustomCmd);
		m_pProcessor->Start();
	}

	return result;
}



void RitexDevice::ReportDataPacket(DataPacket* packet)
{
#ifdef __DEBUG__
	Log("Reporting packet\n");
#endif

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
			m_pAdapter->getDataLogger()->EnqueData(event, false);
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
	event->setChannelId(GetDeviceStateChannelId());
	event->setRegisterTimeDate(time);
	if (code == 0) {
		event->setTypeId(25);
		//event->setArgument1(); //TODO: set prev. erro
	} else {
		event->setTypeId(8);
		event->setArgument1(getFaultText(code)); //error text
		event->setArgument2(itoa(code)); //error code
	}
	ReportEvent(event);
}

void RitexDevice::ReportStationState(int newState, int oldState, time_t time) {
	DBEventCommon* event = new DBEventCommon();
	event->setChannelId(GetDeviceStateChannelId());
	event->setRegisterTimeDate(time);
	event->setTypeId(10);
	event->setArgument1(itoa(newState));
	event->setArgument2(itoa(oldState));
	if(newState == 0) {
		event->setArgument3(std::string("ВД выключен"));
	} else {
		event->setArgument3(std::string("ВД включен"));
	}
	event->setArgument4("HND");
	ReportEvent(event);
}

void RitexDevice::CheckAndReportTimeDiviation(DataPacket* packet)
{

	time_t system_time = time(NULL);

	Log( "[TIME] Local time: %s. epoch=%lu", TimeToString(system_time).c_str(), system_time);

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

		Log( "[TIME] KSU time: %d-%d-%d %d:%d:%d . epoch=%lu Diviation [%d] sec", year, p[20],p[19], p[23],p[24],0, timestamp, (int)(system_time - timestamp));

		if(abs(system_time - timestamp) > m_timeDiviation) {
			//report this event only once
			if(m_isDiviationReported) {
				return;
			}

			m_isDiviationReported = true;

			DBEventCommon* event = new DBEventCommon();
			event->setChannelId(GetDeviceStateChannelId());
			event->setTypeId(6);
			event->setRegisterTimeDate(system_time);
			event->setArgument1(itoa(abs(system_time - timestamp) / 60) );
			ReportEvent(event);
		} else {
			//reset flag
			m_isDiviationReported = false;
		}
	}
}

unsigned short RitexDevice::GetSettingFromPacket(const DataPacket& pPacket, int offset, int size)
{
	assert(size == 1 || size == 2);
	unsigned char* data = pPacket.GetDataPtr();
	unsigned short value;
	if(size == 1) {
		value = data[offset];
	} else {
		memcpy(&value, data + offset, 2);
		value = swap16(value);
	}
	return value;
}

/**
 * returns true if new value is applied
 */
bool RitexDevice::SetCurrentSettingValue(int id, unsigned short newValue, unsigned short& oldValue)
{
	assert(id >= all_Settings[0].m_id && id <= all_Settings[NUMBER_OF_SETTINGS-1].m_id);

	//find setting index
	for (unsigned int i = 0; i < NUMBER_OF_SETTINGS; i++) {
		if(all_Settings[i].m_id == id) {
			if(!all_Settings[i].m_isValueSet || all_Settings[i].m_value != newValue) {
				oldValue = all_Settings[i].m_value;
				all_Settings[i].m_value = newValue;
				all_Settings[i].m_isValueSet = true;
				return true;
			}
			break;
		}
	}
	return false;
}

std::string RitexDevice::getSettingName(int id) {
	assert(id >= all_Settings[0].m_id && id <= all_Settings[NUMBER_OF_SETTINGS-1].m_id);
	//find setting index
	for (unsigned int i = 0; i < NUMBER_OF_SETTINGS; i++) {
		if(all_Settings[i].m_id == id) {
			return std::string(all_Settings[i].m_name);
		}
	}
	return std::string("");
}

void RitexDevice::CheckSettigsChanged(const DataPacket& newSettings) {
	assert(newSettings.GetCmd() == ACK_ALL_SETTINGS);

	time_t system_time = time(NULL);

	Log( "CheckSettigsChanged -->>");

	for(unsigned int i = 0; i < NUMBER_OF_SETTINGS; i++) {
		if(!all_Settings[i].m_isVisible || all_Settings[i].m_offset == -1)
			continue;

		int offset = all_Settings[i].m_offset;

		unsigned short newValue = GetSettingFromPacket(newSettings, offset, all_Settings[i].m_size);
		unsigned short oldValue =  all_Settings[i].m_value;

		if(newValue != oldValue) {
			all_Settings[i].m_isValueSet = true;
			all_Settings[i].m_value = newValue;
			DBEventCommon* event = new DBEventCommon();
			event->setChannelId(GetDeviceStateChannelId());
			event->setTypeId(11);
			event->setRegisterTimeDate(system_time);
			event->setArgument1(itoa(newValue));
			event->setArgument2(itoa(oldValue));
			event->setArgument3(all_Settings[i].m_name);

			if(m_isFirstSettingsPacket) {
				if(!all_Settings[i].m_isValueSet) {
					event->setArgument4("INIT");
					Log( "CheckSettigsChanged reporting");
					ReportEvent(event);
				}
			} else {
				event->setArgument4("HND");
				Log( "CheckSettigsChanged reporting");
				ReportEvent(event);
			}
		}
	}
	m_isFirstSettingsPacket = false;
	Log( "CheckSettigsChanged -->>");
}

bool RitexDevice::IncrementEventCount() {
    sqlite3* pDb = NULL;
	std::string query = std::string("update tblConfigSettings   set  Value = Value + \"1\"  where Tag = 'EVNT_CNT'");

#if defined(KSU_EMULATOR) || defined(RS485_ADAPTER)
    std::string dbPath = "/home/ruinmmal/workspace/ritex/data/ic_data3.sdb";
#else
    std::string dbPath = "/mnt/www/ControlServer/data/ic_data3.sdb";
#endif

    int rc = sqlite3_open_v2(dbPath.c_str(), &pDb,SQLITE_OPEN_READWRITE, NULL);

    if(rc != SQLITE_OK)
    {
    	Log( "[SQL] couldn't open DB %s\n", dbPath.c_str());
    	sqlite3_close(pDb);
    	return false;
    }

    sqlite3_stmt* pStm;

    if ((rc = sqlite3_prepare_v2(pDb, query.c_str(), - 1, &pStm, NULL)) == SQLITE_OK) {
    	if (pStm != NULL) {
    		if ((rc = sqlite3_step(pStm)) !=  SQLITE_DONE) {
    			Log("[SQL] Error updating event count %d %s", rc, sqlite3_errmsg(pDb));
    		}
    		sqlite3_finalize(pStm);
    	} else {
    		Log( "[SQL] error preparing %d %s for DB: %s", rc, sqlite3_errmsg(pDb), dbPath.c_str());
    	}
    } else {
    	Log( "[SQL] error preparing %d %s for DB: %s", rc, sqlite3_errmsg(pDb), dbPath.c_str());
    }
    sqlite3_close(pDb);
    return (rc == SQLITE_DONE);
}

void RitexDevice::ReportEvent(DBEventCommon* pEvent) {
	m_pAdapter->getEventLogger()->EnqueData(pEvent);

	DBEventCommon* pEvent2 = new DBEventCommon(*pEvent);

	m_pAdapter->getEventLogger2()->EnqueData(pEvent2);

	IncrementEventCount();
}


void RitexDevice::OnResultReady(DeviceCommand* pCmd)
{
	Log( "######### RitexDevice::OnResultReady");
	// add event for HW commands only
	if(pCmd->isHWCommand()) {

	}
}

bool RitexDevice::UpdateSettingsValues() {

    sqlite3* pDb = NULL;

#if defined(KSU_EMULATOR) || defined(RS485_ADAPTER)
    std::string dbPath = "/home/ruinmmal/workspace/ritex/data/ic_data_event3.sdb";
#else
    std::string dbPath = "/mnt/www/ControlServer/data/ic_data_event3.sdb";
#endif

	std::string sEventSqlQuery = std::string("select Argument3,Argument1 from (" \
					"select  *  from tbleventbus as filter inner join" \
				  	" (select Argument3, max(registerdate) as registerdate from tbleventbus where TypeId = \"11\" group by Argument3 )	as filter1" \
					" on filter.Argument3 = filter1.Argument3 and filter.registerdate = filter1.registerdate)");

    int rc = sqlite3_open_v2(dbPath.c_str(), &pDb,SQLITE_OPEN_READONLY, NULL);

    if(rc != SQLITE_OK)
    {
    	Log( "[SQL] UpdateSettingsValues: couldn't open DB %s\n", dbPath.c_str());
    	sqlite3_close(pDb);
    	return false;
    }

    sqlite3_stmt* pStm;

    if ((rc = sqlite3_prepare_v2(pDb, sEventSqlQuery.c_str(), - 1, &pStm, NULL)) == SQLITE_OK) {
    	while ((rc =sqlite3_step(pStm)) ==  SQLITE_ROW) {
    		const unsigned char * paramId = sqlite3_column_text(pStm, 0);
    		double value = sqlite3_column_double(pStm, 1);
    		Log( "Found setting: S:%s V:%g", paramId, value);

    		for(unsigned int i = 0;i < NUMBER_OF_SETTINGS; i++) {
    			if(strcmp(all_Settings[i].m_name,(const char*)paramId) == 0) {
    					all_Settings[i].m_value = value;
    					all_Settings[i].m_isValueSet = true;
    			}
    		}
    	}
    } else {
    	Log( "[SQL] UpdateSettingsValues: error preparing %d %s for DB: %s", rc, sqlite3_errmsg(pDb), dbPath.c_str());
    }
    sqlite3_finalize(pStm);
    sqlite3_close(pDb);
    return (rc == SQLITE_DONE);
}
int RitexDevice::GetDeviceStateChannelId() {
	return m_pAdapter->GetParameterFilter().FindChannel(DEVICE_STATE_CHANNEL_PARAM);
}





