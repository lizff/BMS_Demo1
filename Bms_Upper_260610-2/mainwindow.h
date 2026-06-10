#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "widgets/batterywidget.h"
#include "widgets/statusindicatorwidget.h"
//#include "communication/serialportmanager.h"
#include "communication/serialworker.h"
#include "models/bmsdata.h"

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
#include <QMenuBar>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QLabel;

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

//    SerialPortManager *m_serialManager;
    QComboBox *portComboBox;
    QComboBox *baudComboBox;
    QComboBox *parityComboBox;

    //左侧电压 & 右侧温度 & 中间电池部分
    QLabel *cellVoltageLabel[16];
    QLabel *tempLabel[16];

    QProgressBar *batteryBar;

    QTimer *commTimer;

    // FakeData定时器
    QTimer *m_fakeTimer;

    // 当前SOC方向
    bool m_socIncreasing;

    // 刷新UI
    void updateUi();

    // 生成模拟数据
    void generateFakeData();

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

    // ================= DO状态 =================
    StatusIndicatorWidget *chargeMosIndicator;
    StatusIndicatorWidget *dischargeMosIndicator;
    StatusIndicatorWidget *prechargeMosIndicator;

    StatusIndicatorWidget *fanIndicator;
    StatusIndicatorWidget *relay1Indicator;
    StatusIndicatorWidget *relay2Indicator;

    StatusIndicatorWidget *gyroIndicator;
    StatusIndicatorWidget *heaterIndicator;

    // ================= 通信状态 =================
    StatusIndicatorWidget *txdIndicator;
    StatusIndicatorWidget *okIndicator;
    StatusIndicatorWidget *errIndicator;

    // ================= 定时器 =================
    QTimer *fakeTimer;

    //********************串口操作函数******************
    void openSerialPort();

    void closeSerialPort();

    QThread *m_serialThread;
    SerialWorker *m_serialWorker;

    QPushButton *connectBtn;
    QPushButton *disconnectBtn;
    QPushButton *refreshBtn;

private slots:

    void refreshPorts();

    void onConnectClicked();

    void onDisconnectClicked();

    void updatePortList(QStringList ports);
};

#endif // MAINWINDOW_H
