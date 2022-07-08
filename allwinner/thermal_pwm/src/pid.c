#include <stdint.h>
#include "pid.h"
#include "platform.h"

PID pid;

void InitPID(void)
{
	pid.SetTemperature = 125;
	pid.SumError = 0;
	pid.Kp = 700;

	pid.Ki = 0.600;
	pid.Kd = 0.00;

    pid.NowError = 0;
    pid.LastError = 0;

    pid.Out0 = 0;
    pid.Out = 0;
}

int LocPIDCalc(void)
{
	static int out1, out2, out3;

	pid.NowError =pid.ActualTemperature-pid.SetTemperature;
	pid.SumError += pid.NowError;

	out1 = pid.Kp * pid.NowError;
	out2 = pid.Kp * pid.Ki * pid.SumError;
	out3 = pid.Kp * pid.Kd * (pid.NowError - pid.LastError);

	pid.LastError = pid.NowError;

	return out1 + out2 + out3 + pid.Out;
}

void SetPID_Kp(int value)
{
	pid.Kp = (float)value/1000;
}

void SetPID_Ki(int value)
{
	pid.Ki = (float)value/1000;
}

void SetPID_Kd(int value)
{
	pid.Kd = (float)value/1000;
}

void SetPID_Temperature(int value)
{
	pid.SetTemperature = value;
}

int32_t pwm_r2p0(uint32_t cycle)
{
#if 0

#endif
}

void pid_test(uint32_t T)
{
	uint32_t Temperature;
	int temp=0,PWMvalue=0;
	InitPID();



	Temperature=thermal_temperature_get();
	if(Temperature>T)
	{
		pid.SetTemperature =T;
		pid.ActualTemperature = Temperature;
		temp = LocPIDCalc();
		if(temp >3072)
			PWMvalue = 3072;
		else if(temp < 0)
			PWMvalue = 0;
		else
			PWMvalue = (unsigned int)temp;
	}
	else
		PWMvalue = 0;

	pwm_r2p0(PWMvalue);

}
