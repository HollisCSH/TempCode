#ifndef __MSG_DEF_H_
#define __MSG_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum _MSG_ID
{
	MSG_TIMEOUT = 1,	//MSG_TIMEOUT(pTimer, timerID)

	MSG_RUN=2,			//MSG_RUN(0, 0)
	MSG_UTP_REQ_DONE=3,	//MSG_RUN(pUtpCmd, UTP_TXF_EVENT)
	MSG_KEY_ON 	= 4,
	MSG_KEY_OFF = 5,
	MSG_BATTERY_PLUG_IN=6,
	MSG_BATTERY_PLUG_OUT=7,
	MSG_SIM_POWEROFF=8,
	MSG_12V_OFF=9,
	MSG_12V_ON=10,
	
	
	MSG_MAX
}MSG_ID;

#ifdef __cplusplus
}
#endif

#endif

