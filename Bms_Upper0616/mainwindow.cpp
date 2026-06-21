#include "mainwindow.h"
#include "widgets/statusindicatorwidget.h"
#include "protocol/protocolbase.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStatusBar>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1180, 840);
    setMinimumHeight(800);
    setMinimumWidth(1150);

    //****************UI界面初始化************
    Ui_Init();
    doSwitchProtocol(0);

}

MainWindow::~MainWindow()
{
    // 关闭串口
    QMetaObject::invokeMethod(
                m_serialWorker,
                "closeSerial",
                Qt::BlockingQueuedConnection);

    // 退出线程
    m_serialThread->quit();

    // 等待线程结束
    m_serialThread->wait();

    // 释放对象
    delete m_serialThread;
}

void MainWindow::Ui_Init()
{

    for(int i=0;i<16;i++)
    {
        cellVoltageLabel[i] = nullptr;
        tempLabel[i] = nullptr;
    }

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    /**************创建串口线程 & 串口对象 & 刷新串口号*********************/
    m_serialThread = new QThread(this);
    m_serialWorker = new SerialWorker;


    m_serialWorker->moveToThread(m_serialThread);
    m_serialThread->start();
    refreshPorts();
    connect(m_serialWorker,
            &SerialWorker::serialOpenResult,
            this,
            &MainWindow::onSerialOpenResult);
    connect(m_serialWorker,
            &SerialWorker::serialCloseResult,
            this,
            &MainWindow::onSerialCloseResult);

    // ================= 总垂直布局 =================
    rootLayout = new QVBoxLayout(central);

    // ================= 顶部菜单栏布局 =================
    QHBoxLayout *menuLayout = new QHBoxLayout;

    // 文件
    QPushButton *fileButton = new QPushButton("文件");

    // 通讯
    QPushButton *commButton = new QPushButton("通讯");

    // 管理
    QPushButton *managerButton = new QPushButton("管理");

    // 窗口
    QPushButton *windowButton = new QPushButton("窗口");

    // 帮助
    QPushButton *helpButton = new QPushButton("帮助");

    //通信方式
    QPushButton *comModeButton = new QPushButton("通信方式");

    // 端口标签
    QLabel *portLabel = new QLabel("端口：");
    //波特率标签
    QLabel *BaudLabel = new QLabel("波特率：");
    //检验位标签
    QLabel *Parity = new QLabel("校验位：");

    // 串口、波特率、校验位下拉框
    portComboBox = new QComboBox;
    baudComboBox = new QComboBox;
    parityComboBox = new QComboBox;

    /***************串口初始化******************/
   connect(m_serialWorker,
            &SerialWorker::portsReady,
            this,
            &MainWindow::updatePortList);

    baudComboBox->addItem(QString("9600"));
    baudComboBox->addItem(QString("19200"));
    parityComboBox->addItem(QString("None"));
    parityComboBox->addItem(QString("Mark"));

    // ================= 状态栏 =================
    // 1. 创建状态标签（左侧）
    m_statusLabel = new QLabel("未连接");
    m_statusLabel->setStyleSheet("color: #888888; font-size: 12px;");
    statusBar()->addWidget(m_statusLabel);

    // 2. 添加弹性空间
    statusBar()->addPermanentWidget(new QWidget(), 1);

    // 3. 创建时间标签（右侧）
    m_timeLabel = new QLabel;
    m_timeLabel->setStyleSheet("color: #888888; font-size: 12px;");
    m_timeLabel->setMinimumWidth(80);  // 保证宽度
    statusBar()->addPermanentWidget(m_timeLabel);

    // 4. 创建定时器并连接
    m_timeTimer = new QTimer(this);
    m_timeTimer->setInterval(1000);
    connect(m_timeTimer, &QTimer::timeout, this, &MainWindow::onTimeTimerTimeout);
    m_timeTimer->start();

    // 5. 立即更新一次
    onTimeTimerTimeout();

    //================= 文件菜单 =================
    QMenu *fileMenu = new QMenu(this);

    QAction *openProtocolAction =
            new QAction("打开协议文件", this);

    fileMenu->addAction(openProtocolAction);

    fileButton->setMenu(fileMenu);
    //================= 通讯菜单 =================
    commMenu = new QMenu(this);

    connectAction =
            new QAction("连接", this);

    disconnectAction =
            new QAction("断开", this);

    refreshAction =
            new QAction("刷新", this);

    commMenu->addAction(connectAction);
    commMenu->addAction(disconnectAction);
    commMenu->addAction(refreshAction);

    commButton->setMenu(commMenu);

    connect(connectAction,
            &QAction::triggered,
            this,
            &MainWindow::onConnectClicked);
    connect(disconnectAction,
            &QAction::triggered,
            this,
            &MainWindow::onDisconnectClicked);
    connect(refreshAction,
            &QAction::triggered,
            this,
            &MainWindow::refreshPorts);
    //================= 窗口菜单 =================
    QMenu *windowMenu = new QMenu(this);

    QAction *serialAction =
            new QAction("串口", this);

    QAction *canAction =
            new QAction("CAN", this);

    windowMenu->addAction(serialAction);
    windowMenu->addAction(canAction);

    windowButton->setMenu(windowMenu);
    //================= 通信方式菜单 =================
    comModeMenu = new QMenu(this);
    comSerialportAction = new QAction("串口通信",this);
    comCanAction = new QAction("CAN通信",this);
    comModeMenu->addAction(comSerialportAction);
    comModeMenu->addAction(comCanAction);
    comModeButton->setMenu(comModeMenu);

    // 设置宽度
    portComboBox->setMinimumWidth(120);

    // 添加到菜单栏
    menuLayout->addWidget(fileButton);
    menuLayout->addWidget(commButton);
    menuLayout->addWidget(managerButton);
    menuLayout->addWidget(windowButton);
    menuLayout->addWidget(helpButton);
    menuLayout->addWidget(comModeButton);

    // 添加弹簧，把端口推到最右边
    menuLayout->addStretch();

    menuLayout->addWidget(portLabel);
    menuLayout->addWidget(portComboBox);
    menuLayout->addWidget(BaudLabel);
    menuLayout->addWidget(baudComboBox);
    menuLayout->addWidget(Parity);
    menuLayout->addWidget(parityComboBox);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // ==================================================
    // 左侧电压区域
    // ==================================================
    QGroupBox *voltageBox = new QGroupBox("单体电压");
    voltageBox->setMinimumWidth(250);

    QGridLayout *voltageLayout = new QGridLayout(voltageBox);

    for(int i = 0; i < 16; i++)
    {
        QLabel *name = new QLabel(QString("Cell%1").arg(i + 1, 2, 10, QChar('0')));
        cellVoltageLabel[i] = new QLabel("0.000 V");

        name->setMinimumHeight(30);
        cellVoltageLabel[i]->setMinimumHeight(30);

        voltageLayout->addWidget(name, i, 0);
        voltageLayout->addWidget(cellVoltageLabel[i], i, 1);
    }

    // ==================================================
    // 中间区域
    // ==================================================
    QVBoxLayout *centerLayout = new QVBoxLayout;

    // ================= DO状态信息 =================

    QGroupBox *doBox = new QGroupBox("DO状态信息");

    doBox->setMinimumWidth(180);

    QGridLayout *doLayout =
            new QGridLayout(doBox);

    // ================= PACK信息区域 =================

    QGroupBox *infoBox = new QGroupBox("Pack信息");

    infoBox->setMinimumHeight(250);

    QVBoxLayout *infoMainLayout = new QVBoxLayout(infoBox);

    // ================= 通信状态区域 =================

    QGroupBox *commBox = new QGroupBox("通信状态");
    QVBoxLayout *commLayout = new QVBoxLayout;
    commBox->setLayout(commLayout);

    commBox->setMinimumWidth(190);
    commBox->setMaximumWidth(260);

    // ================= LED区域 =================

    txdIndicator =
            new StatusIndicatorWidget("TXD");

    okIndicator =
            new StatusIndicatorWidget("OK");

    errIndicator =
            new StatusIndicatorWidget("ERR");

    txdIndicator->setState(false);
    okIndicator->setState(false);
    errIndicator->setState(false);

    QHBoxLayout *ledLayout =
            new QHBoxLayout;
    ledLayout->setSpacing(10);

    ledLayout->setAlignment(Qt::AlignCenter);

    ledLayout->addWidget(txdIndicator);
    ledLayout->addWidget(okIndicator);
    ledLayout->addWidget(errIndicator);

    // ================= 按钮 =================

    connectBtn = new QPushButton("连接");

    disconnectBtn = new QPushButton("断开");

    refreshBtn = new QPushButton("手动刷新");

    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    connectBtn->setMinimumHeight(50);
    disconnectBtn->setMinimumHeight(50);
    refreshBtn->setMinimumHeight(50);

    commLayout->addLayout(ledLayout);

    commLayout->addSpacing(8);

    commLayout->addWidget(connectBtn);
    commLayout->addWidget(disconnectBtn);
    commLayout->addWidget(refreshBtn);

    commLayout->setSpacing(6);

    commLayout->setContentsMargins(
                8,8,8,8);

    // ================= 电池区域 =================

    QHBoxLayout *batteryLayout = new QHBoxLayout;

    // PACK编号
    QLabel *packLabel = new QLabel("待机");

    packLabel->setStyleSheet(
        "font-size:36px;"
        "font-weight:bold;"
        "color:black;"
    );

    // 电池电量条
    batteryWidget = new BatteryWidget;

    batteryWidget->setValue(75);

    batteryWidget->setValue(50);

    batteryWidget->setFixedHeight(60);

    batteryWidget->setStyleSheet(

    "QProgressBar{"
    "border:2px solid #b8a14a;"
    "border-radius:8px;"
    "background-color:#e8e8e8;"
    "text-align:center;"
    "font-size:24px;"
    "color:black;"
    "}"

    "QProgressBar::chunk{"
    "background-color:#8bc34a;"
    "border-radius:5px;"
    "}"

    );

    // 电池右侧小凸起
    QFrame *batteryHead = new QFrame;

    batteryHead->setFixedSize(12,30);

    batteryHead->setStyleSheet(
        "background-color:#b8a14a;"
        "border-radius:3px;"
    );

    batteryLayout->addWidget(packLabel);
    batteryWidget->setFixedHeight(90);
    batteryLayout->addWidget(batteryWidget);
    batteryLayout->addWidget(batteryHead);

    // ================= 电压电流区域 =================

    QHBoxLayout *dataLayout = new QHBoxLayout;

    totalVoltageLabel = new QLabel("0.00V");

    totalCurrentLabel = new QLabel("0.00A");

    QString dataStyle =
        "font-size:52px;"
        "font-weight:bold;"
        "color:black;";

    totalVoltageLabel->setStyleSheet(dataStyle);

    totalCurrentLabel->setStyleSheet(dataStyle);

    dataLayout->addWidget(totalVoltageLabel);

    dataLayout->addStretch();

    dataLayout->addWidget(totalCurrentLabel);

    // ================= 最值信息 =================

    QHBoxLayout *statLayout = new QHBoxLayout;

    QString boxStyle =
        "QLabel{"
        "font-size:18px;"
        "color:black;"
        "}";

    QFrame *maxFrame = new QFrame;
    maxFrame->setFrameShape(QFrame::Box);
    maxFrame->setStyleSheet("background:white;");

    QVBoxLayout *maxLayout = new QVBoxLayout(maxFrame);

    QLabel *maxTitle = new QLabel("最高:");

    maxVoltageLabel = new QLabel("0.000V");

    maxVoltageLabel->setStyleSheet(
        "font-size:32px;"
        "font-weight:bold;"
        "color:black;"
    );

    maxLayout->addWidget(maxTitle);
    maxLayout->addWidget(maxVoltageLabel);

    QFrame *minFrame = new QFrame;
    minFrame->setFrameShape(QFrame::Box);
    minFrame->setStyleSheet("background:white;");

    QVBoxLayout *minLayout = new QVBoxLayout(minFrame);

    QLabel *minTitle = new QLabel("最低:");

    minVoltageLabel = new QLabel("0.000V");

    minVoltageLabel->setStyleSheet(
        "font-size:32px;"
        "font-weight:bold;"
        "color:black;"
    );

    minLayout->addWidget(minTitle);
    minLayout->addWidget(minVoltageLabel);

    QFrame *avgFrame = new QFrame;
    avgFrame->setFrameShape(QFrame::Box);
    avgFrame->setStyleSheet("background:white;");

    QVBoxLayout *avgLayout = new QVBoxLayout(avgFrame);

    QLabel *avgTitle = new QLabel("平均:");

    avgVoltageLabel = new QLabel("0.000V");

    avgVoltageLabel->setStyleSheet(
        "font-size:32px;"
        "font-weight:bold;"
        "color:black;"
    );

    avgLayout->addWidget(avgTitle);
    avgLayout->addWidget(avgVoltageLabel);

    statLayout->addWidget(maxFrame);
    statLayout->addWidget(minFrame);
    statLayout->addWidget(avgFrame);

    // ================= 添加布局 =================

    infoMainLayout->addLayout(batteryLayout);

    infoMainLayout->addSpacing(20);

    infoMainLayout->addLayout(dataLayout);

    infoMainLayout->addSpacing(20);

    infoMainLayout->addLayout(statLayout);

    // ================= 告警信息区域 =================

    QGroupBox *alarmBox = new QGroupBox("告警信息");

    QTableWidget *alarmTable = new QTableWidget;

    alarmTable->setColumnCount(3);
    alarmTable->setMinimumHeight(250);

    QStringList headers;
    headers << "时间" << "等级" << "告警内容";

    alarmTable->setHorizontalHeaderLabels(headers);

    alarmTable->horizontalHeader()->setStretchLastSection(true);

    alarmTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    alarmTable->verticalHeader()->setVisible(false);

    alarmTable->setRowCount(5);

    alarmTable->setItem(0,0,new QTableWidgetItem("21:35:01"));
    alarmTable->setItem(0,1,new QTableWidgetItem("Warning"));
    alarmTable->setItem(0,2,new QTableWidgetItem("Cell03 Over Voltage"));

    alarmTable->setItem(1,0,new QTableWidgetItem("21:35:03"));
    alarmTable->setItem(1,1,new QTableWidgetItem("Error"));
    alarmTable->setItem(1,2,new QTableWidgetItem("MOS Temperature High"));

    alarmTable->setStyleSheet(
        "QTableWidget{"
        "background-color:#1e1e1e;"
        "color:#00ffcc;"
        "gridline-color:#444444;"
        "font-size:14px;"
        "}"
        "QHeaderView::section{"
        "background-color:#2b2b2b;"
        "color:white;"
        "font-size:14px;"
        "}"
    );

    QVBoxLayout *alarmLayout = new QVBoxLayout(alarmBox);

    alarmLayout->addWidget(alarmTable);

    QHBoxLayout *centerInfoLayout = new QHBoxLayout;

    centerInfoLayout->addWidget(infoBox, 8);

    centerInfoLayout->addWidget(commBox, 2);

    centerLayout->addWidget(doBox);

    centerLayout->addLayout(centerInfoLayout);

    centerLayout->addWidget(alarmBox);

    // ==================================================
    // 右侧温度区域
    // ==================================================
    QVBoxLayout * lightLayout = new QVBoxLayout(this);
    QGroupBox *tempBox = new QGroupBox("单体温度");
    tempBox->setMinimumWidth(250);
    tempBox->setMaximumHeight(1000);

    QGridLayout *tempLayout = new QGridLayout(tempBox);

    for(int i = 0; i < 4; i++)
    {
        QLabel *name = new QLabel(QString("Temp%1").arg(i + 1, 2, 10, QChar('0')));
        tempLabel[i] = new QLabel("25.0 ℃");

        name->setMinimumHeight(30);
        tempLabel[i]->setMinimumHeight(30);

        tempLayout->addWidget(name, i, 0);
        tempLayout->addWidget(tempLabel[i], i, 1);
    }

    // ===================== 右侧 电池基本信息 GroupBox =====================
    // 1. 创建组框
    m_groupBasicInfo = new QGroupBox("电池基本运行信息");
    tempBox->setMinimumWidth(180);
    tempBox->setMaximumHeight(1000);

    // 2. 创建格栅布局
    m_gridBasicInfo = new QGridLayout(m_groupBasicInfo);
    m_gridBasicInfo->setContentsMargins(10,10,10,10);
    m_gridBasicInfo->setHorizontalSpacing(12);
    m_gridBasicInfo->setVerticalSpacing(8);

    // 3. 实例化自定义数据控件
    m_widTotalVol = new DataItemWidget(this);
    m_widTotalVol->setItemName("总电压");
    m_widTotalVol->setItemUnit("V");

    m_widTotalCur = new DataItemWidget(this);
    m_widTotalCur->setItemName("总电流");
    m_widTotalCur->setItemUnit("A");

    m_widTotalChrCur = new DataItemWidget(this);
    m_widTotalChrCur->setItemName("总充电电流");
    m_widTotalChrCur->setItemUnit("A");

    m_widTotalDisChrCur = new DataItemWidget(this);
    m_widTotalDisChrCur->setItemName("总放电电流");
    m_widTotalDisChrCur->setItemUnit("A");

    m_widSoc = new DataItemWidget(this);
    m_widSoc->setItemName("SOC");
    m_widSoc->setItemUnit("A");

    m_widSoh = new DataItemWidget(this);
    m_widSoh->setItemName("SOH");
    m_widSoh->setItemUnit("%");

    m_widCycleCount = new DataItemWidget(this);
    m_widCycleCount->setItemName("循环次数");
    m_widCycleCount->setItemUnit("次");

    m_widFullCap = new DataItemWidget(this);
    m_widFullCap->setItemName("满容量");
    m_widFullCap->setItemUnit("Ah");

    m_widRemCap = new DataItemWidget(this);
    m_widRemCap->setItemName("剩余容量");
    m_widRemCap->setItemUnit("Ah");

    m_widSoh = new DataItemWidget(this);
    m_widSoh->setItemName("SOH");
    m_widSoh->setItemUnit("%");

    // ===================== 右侧 逆变器请求信息 GroupBox =====================
    m_groupInvReqInfo = new QGroupBox("逆变器请求信息");
    tempBox->setMinimumWidth(180);
    tempBox->setMaximumHeight(1000);
    m_gridInvReqInfo = new QGridLayout(m_groupInvReqInfo);
    m_gridInvReqInfo->setContentsMargins(10,10,10,10);
    m_gridInvReqInfo->setHorizontalSpacing(12);
    m_gridInvReqInfo->setVerticalSpacing(8);

    m_widChrLimVoltage = new DataItemWidget(this);
    m_widChrLimVoltage->setItemName("充电限制电压");
    m_widChrLimVoltage->setItemUnit("V");

    m_widDisChrLimVoltage = new DataItemWidget(this);
    m_widDisChrLimVoltage->setItemName("放电限制电压");
    m_widDisChrLimVoltage->setItemUnit("V");

    m_widChrLimCurrent = new DataItemWidget(this);
    m_widChrLimCurrent->setItemName("充电限制电流");
    m_widChrLimCurrent->setItemUnit("A");

    m_widDisChrLimCurrent = new DataItemWidget(this);
    m_widDisChrLimCurrent->setItemName("放电限制电流");
    m_widDisChrLimCurrent->setItemUnit("A");

    // 4. 格栅布局摆放（2行2列）
    m_gridBasicInfo->addWidget(m_widTotalVol);
    m_gridBasicInfo->addWidget(m_widTotalCur);
    m_gridBasicInfo->addWidget(m_widTotalChrCur);
    m_gridBasicInfo->addWidget(m_widTotalDisChrCur);
    m_gridBasicInfo->addWidget(m_widSoc);
    m_gridBasicInfo->addWidget(m_widSoh);
    m_gridBasicInfo->addWidget(m_widCycleCount);
    m_gridBasicInfo->addWidget(m_widFullCap);
    m_gridBasicInfo->addWidget(m_widRemCap);

    m_gridInvReqInfo->addWidget(m_widChrLimVoltage);
    m_gridInvReqInfo->addWidget(m_widDisChrLimVoltage);
    m_gridInvReqInfo->addWidget(m_widChrLimCurrent);
    m_gridInvReqInfo->addWidget(m_widDisChrLimCurrent);


    // 5. 将GroupBox加入你右侧总布局（你的右侧单体温度下方总布局）
    lightLayout->addWidget(tempBox);
    lightLayout->addWidget(m_groupBasicInfo);
    lightLayout->addWidget(m_groupInvReqInfo);

    // ================= 添加到主布局 =================
    mainLayout->addWidget(voltageBox);
    mainLayout->addLayout(centerLayout);
    mainLayout->addLayout(lightLayout);

    rootLayout->addLayout(menuLayout);

    rootLayout->addLayout(mainLayout);

    this->setStyleSheet(

    "QMainWindow{"
    "background-color:#2b2f3a;"
    "}"

    "QGroupBox{"
    "border:1px solid #5a5f6b;"
    "margin-top:10px;"
    "color:white;"
    "font-size:15px;"
    "}"

    "QGroupBox::title{"
    "subcontrol-origin: margin;"
    "left:10px;"
    "padding:0 3px 0 3px;"
    "}"

    "QLabel{"
    "color:#00ffcc;"
    "font-size:16px;"
    "}"

    "QProgressBar{"
    "border:1px solid gray;"
    "background:#1e1e1e;"
    "text-align:center;"
    "color:white;"
    "height:25px;"
    "}"

    "QProgressBar::chunk{"
    "background-color:#00cc66;"
    "}"

    );
    // ========== 顶部控件修改 ==========
    // 1. 创建状态面板容器
    m_statusPanel = new QWidget(this);
    if (m_statusPanel->layout()) {
        delete m_statusPanel->layout();
    }
    m_statusPanel->setObjectName("StatusPanel");

    // 2. 创建网格布局（4列，自动换行，无边距问题）
    m_statusLayout = new QGridLayout(m_statusPanel);
    m_statusLayout->setContentsMargins(15, 15, 15, 15); // 四周留边距
    m_statusLayout->setHorizontalSpacing(30); // 水平间距
    m_statusLayout->setVerticalSpacing(15);   // 垂直间距

    // 3. 实例化控件（按构造函数要求传文字）
    m_widChargeEn      = new StatusIndicatorWidget("允许充电");
    m_widDischargeEn   = new StatusIndicatorWidget("允许放电");
    m_widChargeImme1   = new StatusIndicatorWidget("立即充电1");
    m_widChargeImme2   = new StatusIndicatorWidget("立即充电2");
    m_widFullcharge     = new StatusIndicatorWidget("满充请求");

    // 4. 按4列一行添加控件，自动换行
    // 索引从0开始，每4个控件换一行，自定义控件封装容器
    QList<StatusIndicatorWidget*> widgets = {
        m_widChargeEn,
        m_widDischargeEn,
        m_widChargeImme1,
        m_widChargeImme2,
        m_widFullcharge
    };

    for (int i = 0; i < widgets.size(); ++i) {
        int row = i / 4;  // 每4个控件一行
        int col = i % 4;  // 列数：0~3
        m_statusLayout->addWidget(widgets[i], row, col);
    }

        // 5. 把状态面板添加到主窗口布局中
        // 假设你的主窗口布局是水平布局，把状态面板加到中间位置
    doLayout->addWidget(m_statusPanel);
        // 6. 默认隐藏所有控件，等协议配置加载后再显示
        refreshStatusWidgetVisible();

    // 初始化SOC方向
    m_socIncreasing = false;

    // 创建FakeData定时器
    m_fakeTimer = new QTimer(this);

    // 500ms刷新一次
    m_fakeTimer->start(500);

    // 初始化默认数据

    m_bmsData.soc = 50;

    m_bmsData.totalVoltage = 0.00;

    m_bmsData.current = 0.00;

    for(int i=0;i<16;i++)
    {
        m_bmsData.cellVoltage[i] = 0.000;

        m_bmsData.temperature[i] = 0.00;
    }

    m_bmsData.chargeMos = true;
    m_bmsData.dischargeMos = true;
    m_bmsData.fullChargeRequest = false;

    m_bmsData.fanOn = true;

    m_bmsData.relay1 = false;
    m_bmsData.relay2 = false;

    m_bmsData.gyroEnable = true;

    m_bmsData.heaterOn = false;

    m_bmsData.communicationOk = false;

    if(!txdIndicator)
        return;

    if(!okIndicator)
        return;

    if(!errIndicator)
        return;
}

//void MainWindow::updateStatusBarInfo()
//{
//    if (!m_statusLabel) return;

//        // 获取超时信息
//        int timeoutCount = 0;
//        qint64 elapsedSeconds = 0;

//        if (m_protocol) {
//            VoltronicProtocol* proto = qobject_cast<VoltronicProtocol*>(m_protocol);
//            if (proto) {
//                timeoutCount = proto->getTimeoutCount();
//                elapsedSeconds = proto->getElapsedSeconds();
//            }
//        }

//        QString statusText = QString(" 通信异常，请检查连接  【心跳】超时计数: %1  距上次数据: %2 秒")
//                             .arg(timeoutCount)
//                             .arg(elapsedSeconds);

//        m_statusLabel->setText(statusText);
//        m_statusLabel->setStyleSheet("color: #ff4444; font-size: 12px;");
//}

void MainWindow::updateStatusBarInfo()
{
    if (!m_statusLabel) return;

    // 获取超时信息
    int timeoutCount = 0;
    qint64 elapsedSeconds = 0;

    if (m_protocol) {
        VoltronicProtocol* proto = qobject_cast<VoltronicProtocol*>(m_protocol);
        if (proto) {
            timeoutCount = proto->getTimeoutCount();
            elapsedSeconds = proto->getElapsedSeconds();
        }
    }

    // 构建状态文本
    QString statusText = QString("❌ 通信异常，请检查连接  【心跳】超时计数: %1  距上次数据: %2 秒")
                         .arg(timeoutCount)
                         .arg(elapsedSeconds);

    m_statusLabel->setText(statusText);
    m_statusLabel->setStyleSheet("color: #ff4444; font-size: 12px;");
}

void MainWindow::refreshStatusWidgetVisible()
{
    if(m_widChargeEn)
        m_widChargeEn->setVisible(m_curProtoConfig.showChargeEnable);

    if(m_widDischargeEn)
        m_widDischargeEn->setVisible(m_curProtoConfig.showDischargeEnable);

    if(m_widChargeImme1)
        m_widChargeImme1->setVisible(m_curProtoConfig.showChargeImme1);

    if(m_widChargeImme2)
        m_widChargeImme2->setVisible(m_curProtoConfig.showChargeImme2);

    if(m_widFullcharge)
        m_widFullcharge->setVisible(m_curProtoConfig.showFullchargeEnable);

    //把协议对应配置的数据项，进行显示加载到界面中

    refreshBasicInfoVisible();
}

void MainWindow::refreshBasicInfoVisible()
{
    // 控制整个 GroupBox 的显示/隐藏
        if (m_groupBasicInfo)
        {
            m_groupBasicInfo->setVisible(
                m_curProtoConfig.totalVoltage ||
                        m_curProtoConfig.totalCurrent ||
                m_curProtoConfig.totalChrCurrent ||
                m_curProtoConfig.totalDisChrCurrent ||
                m_curProtoConfig.soc ||
                        m_curProtoConfig.soh ||
                        m_curProtoConfig.cycleCount ||
                        m_curProtoConfig.fullCapacity ||
                        m_curProtoConfig.remainCapacity
            );
        }

        // 控制各个数据项的显示/隐藏
        if (m_widTotalVol)
            m_widTotalVol->setVisible(m_curProtoConfig.totalVoltage);

        if (m_widTotalCur)
            m_widTotalCur->setVisible(m_curProtoConfig.totalCurrent);

        if (m_widTotalChrCur)
            m_widTotalChrCur->setVisible(m_curProtoConfig.totalChrCurrent);

        if (m_widTotalDisChrCur)
            m_widTotalDisChrCur->setVisible(m_curProtoConfig.totalDisChrCurrent);

        if (m_widSoc)
            m_widSoc->setVisible(m_curProtoConfig.soc);

        if (m_widSoh)
            m_widSoh->setVisible(m_curProtoConfig.soh);

        if (m_widCycleCount)
            m_widCycleCount->setVisible(m_curProtoConfig.cycleCount);

        if (m_widFullCap)
            m_widFullCap->setVisible(m_curProtoConfig.fullCapacity);

        if (m_widRemCap)
            m_widRemCap->setVisible(m_curProtoConfig.remainCapacity);

}

void MainWindow::doSwitchProtocol(int protoIndex)
{

    connect(m_protocol, &VoltronicProtocol::timeoutInfoUpdated,
            this, &MainWindow::onTimeoutInfoUpdated);

    if (protoIndex < 0 || protoIndex >= m_protoTable.size())
        return;

    // 安全销毁旧协议
    if (m_protocol)
    {
        // 断开所有与m_protocol相关的连接
        m_protocol->disconnect();
        disconnect(m_protocol);

        delete m_protocol;
        m_protocol = nullptr;
    }

    // 加载新协议配置
    m_curProtoConfig = m_protoTable[protoIndex];

    // 创建新协议对象
    if (protoIndex == 0)
    {
        m_protocol = new VoltronicProtocol(this);
    }
    else
    {
        // 后续其他协议
        m_protocol = new VoltronicProtocol(this);
    }

    // 绑定信号槽
    if (m_protocol)
    {
        connect(m_serialWorker, &SerialWorker::dataReceived,
                m_protocol, &ProtocolBase::inputData);

        connect(m_protocol, &ProtocolBase::bmsDataReady,
                this, &MainWindow::onBmsDataUpdated);

        // ========== 新增：连接通信状态信号 ==========
        connect(m_protocol, &ProtocolBase::communicationStateChanged,
                this, &MainWindow::onCommunicationStateChanged);

        onCommunicationStateChanged(false);

         qDebug() << "【通信状态信号】已连接";  // ← 添加日志
    }

    // 更新UI控件显隐
    refreshStatusWidgetVisible();
}

void MainWindow::refreshPorts()
{
    QMetaObject::invokeMethod(
                m_serialWorker,
                "refreshPorts",
                Qt::QueuedConnection);
}

void MainWindow::onConnectClicked()
{
    qDebug() << "已经进入到串口打开的槽函数"<<endl;

    QString portName =
            portComboBox->currentText();

    int baudRate =
            baudComboBox->currentText().toInt();

    int parity;
    if(parityComboBox->currentIndex() == 1)
    parity = 5;
    else
        parity = 0;

    int dataBits = 8;

    int stopBits = 1;

    // 调用线程中的串口打开
    QMetaObject::invokeMethod(
                m_serialWorker,
                "openSerial",
                Qt::QueuedConnection,
                Q_ARG(QString, portName),
                Q_ARG(int, baudRate),
                Q_ARG(int, dataBits),
                Q_ARG(int, parity),
                Q_ARG(int, stopBits)
                );
}

void MainWindow::onDisconnectClicked()
{
    qDebug() <<"已经入到断开连接的函数" <<endl;
    QMetaObject::invokeMethod(
                m_serialWorker,
                &SerialWorker::closeSerialPort,
                Qt::QueuedConnection
                );
}

void MainWindow::updatePortList(QStringList ports)
{
    portComboBox->clear();

    portComboBox->addItems(ports);
}

void MainWindow::onSerialOpenResult(bool ok, QString msg)
{

    QMessageBox::information(
                this,
                "串口提示",
                msg);
    if(ok)
    {
        // 连接成功后禁用
        connectBtn->setEnabled(false);

        portComboBox->setEnabled(false);

        baudComboBox->setEnabled(false);

        parityComboBox->setEnabled(false);

        disconnectBtn->setEnabled(true);
    }
}

void MainWindow::onSerialCloseResult(bool ok, QString msg)
{
    QMessageBox::information(
                this,
                "串口提示",
                msg);
    if(ok)
    {
        // 恢复可点击
        connectBtn->setEnabled(true);

        portComboBox->setEnabled(true);

        baudComboBox->setEnabled(true);

        parityComboBox->setEnabled(true);

        disconnectBtn->setEnabled(false);
    }
}

void MainWindow::onFrameReceived(QByteArray frame)
{
    qDebug() << "收到完整数据帧:" << frame.toHex(' ');
}

void MainWindow::onBmsDataUpdated(const BmsData &data)
{

        m_bmsData.communicationOk = data.communicationOk;
            updateUi();
    qDebug() << "【UI收到数据】充电限制电压:" << data.voltageLimChr;
        // ========== 增量：刷新自定义状态控件 StatusIndicatorWidget ==========
        // 规则：只有配置为“显示”的控件，才更新状态
    if (m_curProtoConfig.showChargeEnable && m_widChargeEn) {
           m_widChargeEn->setColor(data.chargeEnable ? Green : Gray);
           m_widChargeEn->setState(data.chargeEnable);
       }

       if (m_curProtoConfig.showDischargeEnable && m_widDischargeEn) {
           m_widDischargeEn->setColor(data.dischargeEnable ? Green : Gray);
           m_widDischargeEn->setState(data.dischargeEnable);
       }

       if (m_curProtoConfig.showChargeImme1 && m_widChargeImme1) {
           m_widChargeImme1->setColor(data.chargeImmediately ? Green : Gray);
           m_widChargeImme1->setState(data.chargeImmediately);
       }

       if (m_curProtoConfig.showChargeImme2 && m_widChargeImme2) {
           m_widChargeImme2->setColor(data.chargeImmediately2 ? Green : Gray);
           m_widChargeImme2->setState(data.chargeImmediately2);
       }

       if (m_curProtoConfig.showFullchargeEnable && m_widFullcharge) {
           m_widFullcharge->setColor(data.fullChargeRequest ? Green : Gray);
           m_widFullcharge->setState(data.fullChargeRequest);
       }
       // ===== 更新电池基本信息数据项 =====
           // 总电压
           if (m_widTotalVol) {
               m_widTotalVol->setItemValue(data.totalVoltage, 2);
           }

           // 总电流
           if (m_widTotalCur) {
               m_widTotalCur->setItemValue(data.current, 2);
           }

           // 总充电电流
           if (m_widTotalChrCur) {
               m_widTotalChrCur->setItemValue(data.chargeCurrent, 2);
           }

           // 总放电电流
           if (m_widTotalDisChrCur) {
               m_widTotalDisChrCur->setItemValue(data.dischargeCurrent, 2);
           }

           // SOC
           if (m_widSoc) {
               m_widSoc->setItemValue(data.soc, 0);
           }

           // SOH
           if (m_widSoh) {
               m_widSoh->setItemValue(data.soh, 1);
           }

           // 循环次数
           if (m_widCycleCount) {
               m_widCycleCount->setItemValue(data.cycleCount, 0);
           }

           // 满容量
           if (m_widFullCap) {
               m_widFullCap->setItemValue(data.totalCapacity, 2);
           }

           // 剩余容量
           if (m_widRemCap) {
               m_widRemCap->setItemValue(data.remainCapiticy, 2);
           }
           //
           if(m_widChrLimVoltage){
               m_widChrLimVoltage->setItemValue(data.voltageLimChr, 2);
           }
           if(m_widDisChrLimVoltage){
               m_widDisChrLimVoltage->setItemValue(data.voltageLimDischr, 2);
           }
           if(m_widChrLimCurrent){
               m_widChrLimCurrent->setItemValue(data.currentLimchr, 2);
           }
           if(m_widDisChrLimCurrent){
               m_widDisChrLimCurrent->setItemValue(data.currentLimDischr, 2);
           }
}

//void MainWindow::onCommunicationStateChanged(bool isOk)
//{
//    qDebug() << "【onCommunicationStateChanged】isOk =" << isOk;

//    m_bmsData.communicationOk = isOk;

//    if (!txdIndicator || !okIndicator || !errIndicator) {
//        return;
//    }

//    if (isOk)
//    {
//        txdIndicator->setColor(Green);
//        txdIndicator->setState(true);
//        txdIndicator->setBlink(true);

//        okIndicator->setColor(Green);
//        okIndicator->setState(true);
//        okIndicator->setBlink(false);

//        errIndicator->setState(false);
//        errIndicator->setBlink(false);

//        // 状态栏显示：通信正常
//        if (m_statusLabel) {
//            m_statusLabel->setText("通信正常");
//            m_statusLabel->setStyleSheet("color: #00ff66; font-size: 12px;");
//        }
//    }
//    else
//    {
//        txdIndicator->setColor(Red);
//        txdIndicator->setState(true);
//        txdIndicator->setBlink(true);

//        okIndicator->setState(false);
//        okIndicator->setBlink(false);

//        errIndicator->setColor(Red);
//        errIndicator->setState(true);
//        errIndicator->setBlink(true);

//        // 状态栏显示：通信异常 + 超时信息
//        updateStatusBarInfo();
//    }
//}


void MainWindow::onCommunicationStateChanged(bool isOk)
{
    qDebug() << "【onCommunicationStateChanged】isOk =" << isOk;

    m_bmsData.communicationOk = isOk;

    if (!txdIndicator || !okIndicator || !errIndicator) {
        return;
    }

    if (isOk)
    {
        // ===== 通信正常 =====
        txdIndicator->setColor(Green);
        txdIndicator->setState(true);
        txdIndicator->setBlink(true);

        okIndicator->setColor(Green);
        okIndicator->setState(true);
        okIndicator->setBlink(false);

        errIndicator->setState(false);
        errIndicator->setBlink(false);

        // ✅ 修复：通信正常时，显示正常信息，不显示超时
        if (m_statusLabel) {
            m_statusLabel->setText("✅ 通信正常");
            m_statusLabel->setStyleSheet("color: #00ff66; font-size: 12px;");
        }
    }
    else
    {
        // ===== 通信异常 =====
        txdIndicator->setColor(Red);
        txdIndicator->setState(true);
        txdIndicator->setBlink(true);

        okIndicator->setState(false);
        okIndicator->setBlink(false);

        errIndicator->setColor(Red);
        errIndicator->setState(true);
        errIndicator->setBlink(true);

        // ✅ 通信异常时，显示超时信息
        updateStatusBarInfo();
    }
}

void MainWindow::onTimeTimerTimeout()
{
    if (m_timeLabel) {
        m_timeLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
    }
}

void MainWindow::onTimeoutInfoUpdated(int count, qint64 elapsed)
{
    // 只在通信异常时更新状态栏
        if (!m_bmsData.communicationOk) {
            if (m_statusLabel) {
                QString statusText = QString(" 通信异常，请检查连接  【心跳】超时计数: %1  距上次数据: %2 秒")
                                     .arg(count)
                                     .arg(elapsed);
                m_statusLabel->setText(statusText);
                m_statusLabel->setStyleSheet("color: #ff4444; font-size: 12px;");
            }
        }
}

void MainWindow::updateUi()
{
qDebug() << "updateUi start";
 qDebug() << "【updateUi】进入，communicationOk =" << m_bmsData.communicationOk;

    if(!txdIndicator ||
       !okIndicator ||
       !errIndicator)
    {
        return;
    }
    // ================= SOC =================
    batteryWidget->setValue(m_bmsData.soc);

    // ================= 总压 =================
    totalVoltageLabel->setText(
                QString("%1 V")
                .arg(m_bmsData.totalVoltage,0,'f',2));

    // ================= 电流 =================
    totalCurrentLabel->setText(
                QString("%1 A")
                .arg(m_bmsData.current,0,'f',2));

    // ================= 单体电压 =================
    for(int i=0;i<16;i++)
    {
        cellVoltageLabel[i]->setText(
                    QString("%1 V")
                    .arg(m_bmsData.cellVoltage[i],
                         0,'f',3));
    }

//     ================= 温度 =================
    for(int i=0;i<4;i++)
    {
        tempLabel[i]->setText(
                    QString("%1 ℃")
                    .arg(m_bmsData.temperature[i],
                         0,'f',1));
    }

    // ================= DO状态 =================
    // ================= 状态栏 =================
    // 创建左侧状态标签（显示通信状态）
    m_statusLabel = new QLabel("未连接");
    m_statusLabel->setStyleSheet("color: #888888; font-size: 12px;");
    statusBar()->addWidget(m_statusLabel);

    // 添加弹性空间，把时间推到最右侧
    statusBar()->addPermanentWidget(new QWidget(), 1);

    // 创建右侧时间标签
    m_timeLabel = new QLabel;
    m_timeLabel->setStyleSheet("color: #888888; font-size: 12px;");
    statusBar()->addPermanentWidget(m_timeLabel);

    // 创建定时器，每秒更新一次时间
    m_timeTimer = new QTimer(this);
    m_timeTimer->setInterval(1000);
    connect(m_timeTimer, &QTimer::timeout, this, &MainWindow::onTimeTimerTimeout);
    m_timeTimer->start();

    // 立即更新一次时间
    onTimeTimerTimeout();

    // ================= 通信状态 =================

}
