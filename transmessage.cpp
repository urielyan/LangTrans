#include "transmessage.h"

#include <QCryptographicHash>

#include <QDebug>

TransMessage::TransMessage()
{
}


void TransMessage::setmd5()
{
    QString md5Source;
    QCryptographicHash  md5(QCryptographicHash::Md5);

    md5Source = m_sourceText + m_context;
    md5.addData(md5Source.toLocal8Bit().constData(), md5Source.length());

    m_md5 = QString(md5.result().toHex());
}
