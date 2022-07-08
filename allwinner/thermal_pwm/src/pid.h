#ifndef PID_H_
#define PID_H_


typedef struct PID
{
    int  SetTemperature;
    int  ActualTemperature;

    int SumError;
    float  Kp;
    float  Ki;
    float  Kd;
    int NowError;
    int LastError;

    int Out0;
    int Out;
} PID;

extern PID pid;
void InitPID(void);
int LocPIDCalc(void);
void SetPID_Kp(int value);
void SetPID_Ki(int value);
void SetPID_Kd(int value);
void SetPID_Temperature(int value);

#endif /* PID_H_ */
