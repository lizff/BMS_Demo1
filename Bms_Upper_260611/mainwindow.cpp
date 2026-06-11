#include "mainwindow.h"
#include "widgets/statusindicatorwidget.h"
#include "protocol/protocolparser.h"

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
    resize(1180, 750);
    setMinimumHeight(710);
    setMinimumWidth(1150);

    //****************UI界面初始化************
    Ui_Init();

//     刷新一次UI
    updateUi();
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

void MainWindow::generateFakeData()
{
    // ================= SOC动态变化 =================

    if(m_socIncreasing)
    {
        m_bmsData.soc++;

        if(m_bmsData.soc >= 100)
        {
            m_socIncreasing = false;
        }
    }
    else
    {
        m_bmsData.soc--;

        if(m_bmsData.soc <= 5)
        {
            m_socIncreasing = true;
        }
    }

    // ================= 电压变化 =================

    m_bmsData.totalVoltage =
            48.0f +
            QRandomGenerator::global()->bounded(40) / 10.0f;

    // ================= 电流变化 =================

    m_bmsData.current =
            -20.0f +
            QRandomGenerator::global()->bounded(400) / 10.0f;

    // ================= 单体电压 =================

    for(int i=0;i<16;i++)
    {
        m_bmsData.cellVoltage[i] =
                3.0f +
                QRandomGenerator::global()->bounded(120) / 100.0f;
    }

    // ================= 温度变化 =================

    for(int i=0;i<16;i++)
    {
        m_bmsData.temperature[i] =
                20.0f +
                QRandomGenerator::global()->bounded(150) / 10.0f;
    }

    // ================= DO状态随机变化 =================

    m_bmsData.fanOn =
            QRandomGenerator::global()->bounded(2);

    m_bmsData.relay1 =
            QRandomGenerator::global()->bounded(2);

    m_bmsData.relay2 =
            QRandomGenerator::global()->bounded(2);

    // ================= 通信状态 =================

    m_bmsData.communicationOk =
            QRandomGenerator::global()->bounded(10) > 1;

    // 更新UI
    updateUi();
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

    /**************创建协议解析类*********************/
    m_parser = new ProtocolParser(this);
    connect(m_serialWorker,
            &SerialWorker::dataReceived,
            m_parser,
            &ProtocolParser::inputData);

    connect(m_parser,
            &ProtocolParser::frameReceived,
            this,
            &MainWindow::onFrameReceived);


    // ================= 总垂直布局 =================
    QVBoxLayout *rootLayout = new QVBoxLayout(central);

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

    chargeMosIndicator =
            new StatusIndicatorWidget("充电MOS");

    dischargeMosIndicator =
            new StatusIndicatorWidget("放电MOS");

    prechargeMosIndicator =
            new StatusIndicatorWidget("预充MOS");

    fanIndicator =
            new StatusIndicatorWidget("风扇状态");

    relay1Indicator =
            new StatusIndicatorWidget("干接点1");

    relay2Indicator =
            new StatusIndicatorWidget("干接点2");

    gyroIndicator =
            new StatusIndicatorWidget("陀螺仪使能");

    heaterIndicator =
            new StatusIndicatorWidget("加热状态");

    doLayout->addWidget(chargeMosIndicator,0,0);
    doLayout->addWidget(dischargeMosIndicator,0,1);
    doLayout->addWidget(prechargeMosIndicator,0,2);
    doLayout->addWidget(fanIndicator,0,3);

    doLayout->addWidget(relay1Indicator,1,0);
    doLayout->addWidget(relay2Indicator,1,1);
    doLayout->addWidget(gyroIndicator,1,2);
    doLayout->addWidget(heaterIndicator,1,3);

    chargeMosIndicator->setState(true);

    dischargeMosIndicator->setState(true);

    prechargeMosIndicator->setState(false);

    fanIndicator->setState(true);

    relay1Indicator->setState(false);

    relay2Indicator->setState(false);

    gyroIndicator->setState(true);

    heaterIndicator->setState(false);

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

  //  模拟正常连接
    txdIndicator->setColor(Green);
    okIndicator->setColor(Green);
    errIndicator->setColor(Gray);

    txdIndicator->setBlink(true);
    okIndicator->setBlink(true);
//    //模拟超时
//    txdIndicator->setColor(Red);
//    errIndicator->setColor(Red);

//    txdIndicator->setBlink(true);
//    errIndicator->setBlink(true);

//    okIndicator->setColor(Gray);

    // ================= 电池区域 =================

    QHBoxLayout *batteryLayout = new QHBoxLayout;

    // PACK编号
    QLabel *packLabel = new QLabel("充电");

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
    QGroupBox *tempBox = new QGroupBox("单体温度");
    tempBox->setMinimumWidth(250);

    QGridLayout *tempLayout = new QGridLayout(tempBox);

    for(int i = 0; i < 16; i++)
    {
        QLabel *name = new QLabel(QString("Temp%1").arg(i + 1, 2, 10, QChar('0')));
        tempLabel[i] = new QLabel("25.0 ℃");

        name->setMinimumHeight(30);
        tempLabel[i]->setMinimumHeight(30);

        tempLayout->addWidget(name, i, 0);
        tempLayout->addWidget(tempLabel[i], i, 1);
    }

    // ================= 添加到主布局 =================
    mainLayout->addWidget(voltageBox);
    mainLayout->addLayout(centerLayout);
    mainLayout->addWidget(tempBox);

    rootLayout->addLayout(menuLayout);

    rootLayout->addLayout(mainLayout);

    // ================= 状态栏 =================
    statusBar()->showMessage("未连接");

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

    // 初始化SOC方向
    m_socIncreasing = false;

    // 创建FakeData定时器
    m_fakeTimer = new QTimer(this);

    // 定时刷新
    connect(m_fakeTimer,
            &QTimer::timeout,
            this,
            &MainWindow::generateFakeData);

    // 500ms刷新一次
    m_fakeTimer->start(500);

    // 初始化默认数据

    m_bmsData.soc = 80;

    m_bmsData.totalVoltage = 51.2f;

    m_bmsData.current = 12.5f;

    for(int i=0;i<16;i++)
    {
        m_bmsData.cellVoltage[i] = 3.2f;

        m_bmsData.temperature[i] = 25.0f;
    }

    m_bmsData.chargeMos = true;
    m_bmsData.dischargeMos = true;
    m_bmsData.prechargeMos = false;

    m_bmsData.fanOn = true;

    m_bmsData.relay1 = false;
    m_bmsData.relay2 = false;

    m_bmsData.gyroEnable = true;

    m_bmsData.heaterOn = false;

    m_bmsData.communicationOk = true;

    if(!txdIndicator)
        return;

    if(!okIndicator)
        return;

    if(!errIndicator)
        return;
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
//    if()
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

void MainWindow::updateUi()
{
qDebug() << "updateUi start";

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
    for(int i=0;i<16;i++)
    {
        tempLabel[i]->setText(
                    QString("%1 ℃")
                    .arg(m_bmsData.temperature[i],
                         0,'f',1));
    }

    // ================= DO状态 =================
    chargeMosIndicator->setState(
                m_bmsData.chargeMos);

    dischargeMosIndicator->setState(
                m_bmsData.dischargeMos);

    prechargeMosIndicator->setState(
                m_bmsData.prechargeMos);

    fanIndicator->setState(
                m_bmsData.fanOn);

    relay1Indicator->setState(
                m_bmsData.relay1);

    relay2Indicator->setState(
                m_bmsData.relay2);

    gyroIndicator->setState(
                m_bmsData.gyroEnable);

    heaterIndicator->setState(
                m_bmsData.heaterOn);

    // ================= 通信状态 =================
    if(m_bmsData.communicationOk)
    {
        // TXD
        txdIndicator->setColor(Green);
        txdIndicator->setState(true);
        txdIndicator->setBlink(true);

        // OK
        okIndicator->setColor(Green);
        okIndicator->setState(true);
        okIndicator->setBlink(true);

        // ERR
        errIndicator->setBlink(false);
        errIndicator->setState(false);
    }
    else
    {
        // TXD
        txdIndicator->setColor(Red);
        txdIndicator->setState(true);
        txdIndicator->setBlink(true);

        // OK
        okIndicator->setBlink(false);
        okIndicator->setState(false);

        // ERR
        errIndicator->setColor(Red);
        errIndicator->setState(true);
        errIndicator->setBlink(true);
    }
}
