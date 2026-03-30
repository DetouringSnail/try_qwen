#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socket(new QTcpSocket(this))
    , m_logFile(nullptr)
{
    ui->setupUi(this);
    
    // Connect signals and slots
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    
    ui->statusLabel->setText("未连接");
}

MainWindow::~MainWindow()
{
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
    }
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    QString ip = ui->ipLineEdit->text().trimmed();
    QString portStr = ui->portLineEdit->text().trimmed();
    
    if (ip.isEmpty() || portStr.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址和端口号");
        return;
    }
    
    quint16 port = portStr.toUShort();
    if (port == 0) {
        QMessageBox::warning(this, "警告", "端口号无效");
        return;
    }
    
    m_socket->connectToHost(ip, port);
    ui->statusLabel->setText("正在连接...");
    ui->connectButton->setEnabled(false);
}

void MainWindow::on_disconnectButton_clicked()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void MainWindow::onConnected()
{
    ui->statusLabel->setText("已连接");
    ui->disconnectButton->setEnabled(true);
    ui->messageTextEdit->append("已连接到服务器");
}

void MainWindow::onDisconnected()
{
    ui->statusLabel->setText("未连接");
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->messageTextEdit->append("已断开连接");
    
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
    }
}

void MainWindow::onError(QAbstractSocket::SocketError socketError)
{
    QString errorStr;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorStr = "连接被拒绝";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorStr = "远程主机关闭连接";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorStr = "主机未找到";
        break;
    case QAbstractSocket::SocketAccessError:
        errorStr = "套接字访问错误";
        break;
    case QAbstractSocket::SocketResourceError:
        errorStr = "套接字资源错误";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorStr = "套接字超时";
        break;
    case QAbstractSocket::NetworkError:
        errorStr = "网络错误";
        break;
    default:
        errorStr = m_socket->errorString();
        break;
    }
    
    ui->statusLabel->setText("错误：" + errorStr);
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->messageTextEdit->append("错误：" + errorStr);
    
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
    }
}

void MainWindow::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        QByteArray data = m_socket->readLine();
        if (!data.isEmpty()) {
            QString message = QString::fromUtf8(data.trimmed());
            
            // Get current date for log file name
            QString currentDate = QDateTime::currentDateTime().toString("yyyyMMdd");
            
            // Open new log file if date changed
            if (currentDate != m_currentDate) {
                if (m_logFile && m_logFile->isOpen()) {
                    m_logFile->close();
                }
                openLogFile(currentDate);
                m_currentDate = currentDate;
            }
            
            // Display message in text edit (auto-scroll to bottom)
            ui->messageTextEdit->append(message);
            
            // Write to log file with timestamp
            writeLog(message);
        }
    }
}

void MainWindow::openLogFile(const QString &date)
{
    // Create logs directory if not exists
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("logs")) {
        dir.mkdir("logs");
    }
    
    QString logFileName = dir.filePath("logs/") + date + ".log";
    
    if (m_logFile) {
        delete m_logFile;
    }
    
    m_logFile = new QFile(logFileName);
    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Log file opened:" << logFileName;
    } else {
        qDebug() << "Failed to open log file:" << logFileName;
        m_logFile = nullptr;
    }
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logFile && m_logFile->isOpen()) {
        QTextStream out(m_logFile);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        out << "[" << timestamp << "] " << message << "\n";
        m_logFile->flush();
    }
}
