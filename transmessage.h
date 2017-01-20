#ifndef TRANSMESSAGE_H
#define TRANSMESSAGE_H

#include <QString>

class TransMessage
{
public:
    enum Type { Unfinished, Finished, Vanished, Obsolete };

    TransMessage();

    QString md5() const { return m_md5; }
    void setmd5();
    void setmd5(const QString str) {m_md5 = str;}

    QString context() const { return m_context; }
    void setContext(const QString &context) { m_context = context; }

    QString sourceText() const { return m_sourceText; }
    void setSourceText(const QString &sourcetext) { m_sourceText = sourcetext; }
    QString TransText() const { return m_transText; }
    void setTransText(const QString &transText) { m_transText = transText; }

    QString comment() const { return m_comment; }
    void setComment(const QString &comment) { m_comment = comment; }

    Type type() const { return m_type; }
    void setType(Type t) { m_type = t; }

    QString index() const { return m_index; }
    void setIndex(const QString index) { m_index = index; }

private:
    QString     m_index;
    QString     m_sourceText;
    QString     m_context;
    QString     m_md5;
    QString     m_comment;
    QString     m_transText;
    Type m_type;
};


#endif // TRANSMESSAGE_H
