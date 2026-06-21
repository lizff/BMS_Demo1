#ifndef BMSDATA_H
#define BMSDATA_H
#include <QtGlobal>

struct BmsData
{
    // 单体电压
    float cellVoltage[16];

    // 温度
    float temperature[4];

    // 总压
    float totalVoltage;
    //最高节电压
    float maxVoltage;
    //最低单体电压
    float minVoltage;

    // 电流
    float current;
    //充电电流
    float chargeCurrent;
    //放电电流
    float dischargeCurrent;

    //模组总容量
    float totalCapacity;
    //模组剩余容量
    float remainCapiticy;

    // SOC
    quint8 soc;

    //SOH
    float soh;

    //充电限制电压
    float voltageLimChr;
    //放电限制电压
    float voltageLimDischr;
    //充电限制电流
    float currentLimchr;
    //放电限制电流
    float currentLimDischr;
    //循环次数
    int cycleCount;

    //强充禁放标志
    bool chargeEnable;       // Bit7
    bool dischargeEnable;    // Bit6
    bool chargeImmediately; // Bit5
    bool chargeImmediately2;// Bit4
    bool fullChargeRequest;  // Bit3

    // MOS状态
    bool chargeMos;

    bool dischargeMos;

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
