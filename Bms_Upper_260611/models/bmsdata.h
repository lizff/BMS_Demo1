#ifndef BMSDATA_H
#define BMSDATA_H

struct BmsData
{
    // 单体电压
    float cellVoltage[16];

    // 温度
    float temperature[16];

    // 总压
    float totalVoltage;

    // 电流
    float current;

    // SOC
    int soc;

    // MOS状态
    bool chargeMos;

    bool dischargeMos;

    bool prechargeMos;

    // 风扇
    bool fanOn;

    // 干接点
    bool relay1;

    bool relay2;

    // 陀螺仪
    bool gyroEnable;

    // 加热
    bool heaterOn;

    // 通信状态
    bool communicationOk;
};

#endif // BMSDATA_H
