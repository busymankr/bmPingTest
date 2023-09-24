#include <QtGlobal>
#include <QTextCodec>
#include <QTextDecoder>
#include <QRegularExpression>

#include "pingtest.h"

/*

--------
Ubuntu
--------
$ ping www.google.com
PING www.google.com (142.250.207.100) 56(84) bytes of data.
64 bytes from kix06s11-in-f4.1e100.net (142.250.207.100): icmp_seq=1 ttl=110 time=44.8 ms

$ ping 127.0.0.1
PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.099 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.098 ms

$ ping www.naver.com
PING www.naver.com.nheos.com (223.130.195.95) 56(84) bytes of data.

--------
Windows
--------
>ping 127.0.0.1

Ping 127.0.0.1 32바이트 데이터 사용:
127.0.0.1의 응답: 바이트=32 시간<1ms TTL=128
127.0.0.1의 응답: 바이트=32 시간<1ms TTL=128

>ping www.google.com

Ping www.google.com [172.217.31.132] 32바이트 데이터 사용:
172.217.31.132의 응답: 바이트=32 시간=64ms TTL=113
172.217.31.132의 응답: 바이트=32 시간=66ms TTL=113

>ping www.naver.com

Ping www.naver.com.nheos.com [223.130.195.95] 32바이트 데이터 사용:
요청 시간이 만료되었습니다.
요청 시간이 만료되었습니다.

*/

PingTest::PingTest(QObject *parent) : QObject(parent)
{
    m_pProcessPing = nullptr;

    init();
}

PingTest::~PingTest()
{
    deinit();
}

int PingTest::init()
{
    if (nullptr != m_pProcessPing) {
        m_pProcessPing->kill();
        delete m_pProcessPing;
        m_pProcessPing = nullptr;
    }

    m_pProcessPing = new QProcess(this);
    connect(m_pProcessPing, SIGNAL(started()), this, SLOT(processStarted()));
    connect(m_pProcessPing,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
    connect(m_pProcessPing, SIGNAL(finished(int)), this, SLOT(processFinished()));

    return 0;
}

int PingTest::deinit()
{
    if (nullptr != m_pProcessPing) {
        m_pProcessPing->kill();
        delete m_pProcessPing;
        m_pProcessPing = nullptr;
    }

    return 0;
}

int PingTest::ping(bool bStart)
{
    if (true == bStart) {
        start();
    }
    else {
        stop();
    }

    return 0;
}

void PingTest::setIP(QString ip)
{
    m_IP = ip;
}

QString PingTest::getIP()
{
    return m_IP;
}

int PingTest::start()
{
    if (nullptr == m_pProcessPing) {
        return -1;
    }

    if (true == getIP().isEmpty()) {
        return -2;
    }

    QString command = "ping";
    QStringList args;

#if defined(Q_OS_WIN32)
    args << getIP() << "-t";
#elif defined(Q_OS_LINUX)
    args << "-O" << getIP();
#else
    #error Not_Supported
#endif

    m_pProcessPing->waitForStarted(100);
    m_pProcessPing->setProcessChannelMode(QProcess::MergedChannels);
    m_pProcessPing->start(command, args);

    return 0;
}

int PingTest::stop()
{
    if (nullptr == m_pProcessPing) {
        return -1;
    }

    m_pProcessPing->kill();
    return 0;
}

void PingTest::processStarted()
{
    qDebug() << "processStarted()";
}

bool PingTest::isWinEnOutputString(QString s, QString &r)
{
    Q_UNUSED(s);
    Q_UNUSED(r);
    return false;
}

bool PingTest::isWinKrOutputString(QString s, QString &r)
{
    // 127.0.0.1의 응답: 바이트=32 시간<1ms TTL=128
    // 172.217.31.132의 응답: 바이트=32 시간=64ms TTL=113

    QRegularExpression re(" 시간[=<](\\d)*(\\.){0,1}(\\d)*ms TTL=");
    QRegularExpressionMatch match = re.match(s, 0, QRegularExpression::PartialPreferFirstMatch);

    bool hasMatch = match.hasMatch();
    if (true == hasMatch) {
        qDebug() << "match : " << match.capturedTexts();

        QString responseTime = match.captured(0);
        responseTime.replace(QString(" 시간"), QString(""));
        responseTime.replace(QString(" TTL="), QString(""));
        responseTime.replace(QString("ms"), QString(""));

        r = responseTime;
        //qDebug() << "responseTime : " << r;

        return true;
    }

    return false;
}

bool PingTest::isLinuxEnOutputString(QString s, QString &r)
{
    // 64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.099 ms

    QRegularExpression re(" time[=](\\d)*(\\.){0,1}(\\d)* ms");
    QRegularExpressionMatch match = re.match(s, 0, QRegularExpression::PartialPreferFirstMatch);

    bool hasMatch = match.hasMatch();
    if (true == hasMatch) {
        qDebug() << "match : " << match.capturedTexts();

        QString responseTime = match.captured(0);
        responseTime.replace(QString(" "), QString(""));
        responseTime.replace(QString("time="), QString(""));
        responseTime.replace(QString("ms"), QString(""));

        r = responseTime;
        //qDebug() << "responseTime : " << r;

        return true;
    }

    return false;
}

bool PingTest::checkOutputString(QString s, QString &r)
{
    QString tmp = "";

#if defined(Q_OS_WIN32)
    if (isWinKrOutputString(s, tmp) == true) {
        r = tmp;
        return true;
    }
    else if (isWinEnOutputString(s, tmp) == true) {
        r = tmp;
        return true;
    }
#elif defined(Q_OS_LINUX)
    if (isLinuxEnOutputString(s, tmp) == true) {
        r = tmp;
        return true;
    }
#else
    #error Not_Supported
#endif

    return false;
}

QStringList PingTest::convToLineByLine(QString s)
{
    s.replace(QString("\r"), QString("\n"));
    QStringList r = s.split(QLatin1Char('\n'), QString::SkipEmptyParts);
    return r;
}

qlonglong PingTest::getResponseTime(QString s)
{
    // =123
    // <1

    qlonglong responseTime = -1;
    if (true == s.startsWith(QString("="))) {
        s.replace(QString("="), QString(""));
        QStringList t = s.split(QString("."));
        if (2 == t.count()) {
            responseTime = (t[0].toLongLong() * 1000) + t[1].toLongLong();
        }
        else if (1 == t.count()) {
            responseTime = (t[0].toLongLong() * 1000);
        }
    }
    else if (true == s.startsWith(QString("<"))) {
        s.replace(QString("<"), QString(""));
        QStringList t = s.split(QString("."));
        if (2 == t.count()) {
            responseTime = (t[0].toLongLong() * 1000) + t[1].toLongLong() - 1;
        }
        else if (1 == t.count()) {
            responseTime = (t[0].toLongLong() * 1000) - 1;
        }
    }

    return responseTime;
}

void PingTest::readyReadStandardOutput()
{
    if (m_pProcessPing->isReadable()) {
        QByteArray mOutputByteArray = m_pProcessPing->readAllStandardOutput();
        m_OutputString = QString::fromLocal8Bit(mOutputByteArray);
        QStringList outputStrings = convToLineByLine(m_OutputString);
        //qDebug() << m_OutputString;

        for (auto & line : outputStrings) {
            qDebug() << line;
            QString responseTimeString = "";
            qlonglong responseTime = -1;
            bool result = checkOutputString(line, responseTimeString);
            if (true == result) {
                responseTime = getResponseTime(responseTimeString);
                qDebug() << "responseTime : " << responseTimeString << ", " << responseTime;
            }

            emit sigGetResponseTime(responseTime);
        }
    }
}

void PingTest::processFinished()
{
    qDebug() << "processFinished()";
}
