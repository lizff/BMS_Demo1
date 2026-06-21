#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "widgets/batterywidget.h"
#include "widgets/statusindicatorwidget.h"
#include "widgets/dataitemwidget.h"
#include "communication/serialworker.h"
#include "models/bmsdata.h"
#include "protocol/voltronicprotocol.h"
#include "protocol/protocolbase.h"

#include <QThread>
#include <QMainWindow>
#include <QFrame>
#include <QProgressBar>
#include <QTimer>
#include <QFrame>
#include <QtSerialPort>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QList>
#include <QWidget>
#include <QGroupBox>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QLabel;

class ProtocolParser;

// 协议UI配置：控制各状态控件是否显示
struct ProtocolConfig
{
    QString protocolName;

    // 对应BMS状态位
    bool showChargeEnable;      // 充电使能
    bool showDischargeEnable;   // 放电使能
    bool showChargeImme1;       // 立即充电1
    bool showChargeImme2;       // 立即充电2
    bool showFullchargeEnable;      // 满充使能

    //电池基本数据信息
    bool totalVoltage;     //总电压
    bool totalCurrent;      //总电流
    bool totalChrCurrent;   //总充电电流
    bool totalDisChrCurrent;   //总充电电流
    bool soc;       //soc
    bool soh;       //soh
    bool cycleCount;        //循环次数
    bool fullCapacity;      //满容量（标称容量）
    bool remainCapacity;        //剩余容量

    //请求电压电流数据
    bool chrLimVoltage;
    bool disChrLimVoltage;
    bool chrLimCurrent;
    bool disChrLimCurrent;

    // 新状态，在此处继续加字段
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    //***************串口模块部分*************

    //顶部通讯菜单
    QMenu *commMenu;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *refreshAction;
    //顶部通信方式菜单
    QMenu *comModeMenu;
    QAction *comSerialportAction;
    QAction *comCanAction;

    QComboBox *portComboBox;
    QComboBox *baudComboBox;
    QComboBox *parityComboBox;

    //布局
    QVBoxLayout *rootLayout;

    //左侧电压 & 右侧温度 & 中间电池部分
    QLabel *cellVoltageLabel[16];
    QLabel *tempLabel[4];

    QProgressBar *batteryBar;

    QTimer *commTimer;

    // FakeData定时器
    QTimer *m_fakeTimer;

    // 当前SOC方向
    bool m_socIncreasing;

    // 刷新UI
    void updateUi();

    //初始化UI界面
    void Ui_Init();

    bool commState;
    bool flashState;

    // ================= 数据模型 =================
    BmsData m_bmsData;

    // ================= 电池控件 =================
    BatteryWidget *batteryWidget;

    // ================= Pack数据显示 =================
    QLabel *totalVoltageLabel;
    QLabel *totalCurrentLabel;

    QLabel *maxVoltageLabel;
    QLabel *minVoltageLabel;
    QLabel *avgVoltageLabel;

    // ================= 单体电压 =================
    QLabel *cellVoltageLabels[16];

    // ================= 温度 =================
    QLabel *tempLabels[16];

    // ========== 新增：状态栏时间显示 ==========
    QLabel* m_timeLabel;        // 显示实时时间
    QTimer* m_timeTimer;        // 每秒更新一次时间
    QLabel* m_statusLabel;      // 显示通信状态

    // ========== 新增：更新状态栏信息 ==========
    void updateStatusBarInfo();

    // ========== 自定义状态控件对象 ==========
    // 布局和状态面板
    QWidget *m_statusPanel;
    QGridLayout *m_statusLayout;

    // 对应 充电使能、放电使能、立即充电1、立即充电2、满充标志
    StatusIndicatorWidget* m_widChargeEn;
    StatusIndicatorWidget* m_widDischargeEn;
    StatusIndicatorWidget* m_widChargeImme1;
    StatusIndicatorWidget* m_widChargeImme2;
    StatusIndicatorWidget* m_widFullcharge;

    // ================= 通信状态 =================
    StatusIndicatorWidget *txdIndicator;
    StatusIndicatorWidget *okIndicator;
    StatusIndicatorWidget *errIndicator;

    // ================= 定时器 =================
    QTimer *fakeTimer;

    // ================= 自定义控件数据项 =================
    // 右侧 电池基本信息组框
    QGroupBox* m_groupBasicInfo;
    QGridLayout* m_gridBasicInfo;
    QGroupBox* m_groupInvReqInfo;
    QGridLayout* m_gridInvReqInfo;

    // 自定义数据项控件（示例：SOH、循环次数、均衡状态、保护次数等）
    DataItemWidget* m_widTotalVol;
    DataItemWidget* m_widTotalCur;
    DataItemWidget* m_widTotalChrCur;
    DataItemWidget* m_widTotalDisChrCur;
    DataItemWidget* m_widSoc;
    DataItemWidget* m_widSoh;
    DataItemWidget* m_widCycleCount;
    DataItemWidget* m_widFullCap;
    DataItemWidget* m_widRemCap;
    DataItemWidget* m_widChrLimVoltage;
    DataItemWidget* m_widDisChrLimVoltage;
    DataItemWidget* m_widChrLimCurrent;
    DataItemWidget* m_widDisChrLimCurrent;

    //********************串口操作部分******************
    void openSerialPort();

    void closeSerialPort();

    QThread *m_serialThread;
    SerialWorker *m_serialWorker;

    QPushButton *connectBtn;
    QPushButton *disconnectBtn;
    QPushButton *refreshBtn;


    //********************协议解析部分******************
    ProtocolBase *m_protocol = nullptr; // 只用一个基类指针（使用多态，继承虚基类并重写虚函数）


    // 新增：根据当前协议配置，动态显示/隐藏状态控件
    void refreshStatusWidgetVisible();
    // 新增：刷新电池基本信息区域显示
    void refreshBasicInfoVisible();
    // 新增：切换协议入口
    void doSwitchProtocol(int protoIndex);
    // 当前协议配置
    ProtocolConfig m_curProtoConfig;

    // 协议配置表（厂家协议在这里统一管理）
    const QList<ProtocolConfig> m_protoTable =
    {
        // 示例1：厂家A(当前在用Voltronic)，全部状态显示
        {
            "厂家A-Voltronic",
            true,  // 充电使能
            true,  // 放电使能
            true,  // 立即充电1
            true,  // 立即充电2
            true,   // 满充使能

            true,   //总电压
            false,  //总电流
            true,   //总充电电流
            true,   //总放电电流
            true,   //SOC
            false,  //SOH
            false,  //循环次数
            true,   //满容量
            false,   //剩余容量

            true,   //充电限制电压
            true,  //放电限制电流
            true,   //充电限制电流
            true,   //放电限制电流
        },
        // 示例2：厂家B，无立即充电、无预充
        {
            "厂家B",
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            false
        }
        // 后续新增厂家，直接在此列表追加即可
    };

private slots:

    void refreshPorts();

    void onConnectClicked();

    void onDisconnectClicked();

    void updatePortList(QStringList ports);

    void onSerialOpenResult(bool ok,
                            QString msg);
    void onSerialCloseResult(bool ok,
                             QString msg);

    void onFrameReceived(QByteArray frame);

    // 新增：接收协议解析的数据，刷新UI
    void onBmsDataUpdated(const BmsData &data);

    // 新增：通信状态变化槽函数
    void onCommunicationStateChanged(bool isOk);

    //新增：更新时间
    void onTimeTimerTimeout();

    void onTimeoutInfoUpdated(int count, qint64 elapsed);
};

#endif // MAINWINDOW_H
