#include "translator.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamReader>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "xlsxdocument.h"
#include "xlsxformat.h"

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)
#define STRING(s) static QString str##s(QLatin1String(STRINGIFY(s)))

class TSReader : public QXmlStreamReader
{
public:
    TSReader(QIODevice &dev)
      : QXmlStreamReader(&dev)
    {}

    // the "real thing"
    bool update(TransLanguage *pTransLanguage);
    bool release(TransLanguage &transLanguage);

private:
    bool elementStarts(const QString &str) const
    {
        return isStartElement() && name() == str;
    }

    bool isWhiteSpace() const
    {
        return isCharacters() && text().toString().trimmed().isEmpty();
    }

    QString readContents();
};

static QString byteValue(QString value)
{
    int base = 10;
    if (value.startsWith(QLatin1String("x"))) {
        base = 16;
        value.remove(0, 1);
    }
    int n = value.toUInt(0, base);
    return (n != 0) ? QString(QChar(n)) : QString();
}

QString TSReader::readContents()
{
    STRING(byte);
    STRING(value);

    QString result;
    while (!atEnd()) {
        readNext();
        if (isEndElement()) {
            break;
        } else if (isCharacters()) {
            result += text();
        } else if (elementStarts(strbyte)) {
            // <byte value="...">
            result += byteValue(attributes().value(strvalue).toString());
            readNext();
            if (!isEndElement()) {
                qDebug("Error1");
                break;
            }
        } else {
            qDebug("Error2");
            break;
        }
    }
    return result;
}

bool TSReader::update(TransLanguage *pTransLanguage)
{
    QString sourceContext, sourceText, sourceTrans;

    STRING(byte);
    STRING(catalog);
    STRING(comment);
    STRING(context);
    STRING(defaultcodec);
    STRING(dependencies);
    STRING(dependency);
    STRING(extracomment);
    STRING(filename);
    STRING(id);
    STRING(language);
    STRING(line);
    STRING(location);
    STRING(message);
    STRING(name);
    STRING(numerus);
    STRING(numerusform);
    STRING(obsolete);
    STRING(oldcomment);
    STRING(oldsource);
    STRING(source);
    STRING(sourcelanguage);
    STRING(translation);
    STRING(translatorcomment);
    STRING(TS);
    STRING(type);
    STRING(unfinished);
    STRING(userdata);
    STRING(value);
    STRING(vanished);

    static const QString strextrans(QLatin1String("extra-"));

    while (!atEnd()) {
        readNext();
        if (isStartDocument()) {
            // <!DOCTYPE TS>
        } else if (isEndDocument()) {
            // <!DOCTYPE TS>
        } else if (isDTD()) {
            // <!DOCTYPE TS>
        } else if (elementStarts(strTS)) {
            // <TS>
            while (!atEnd()) {
                readNext();
                if (isEndElement()) {
                    // </TS> found, finish local loop
                    break;
                } else if (isWhiteSpace()) {
                    // ignore these, just whitesp{ace
                } else if (elementStarts(strdefaultcodec)) {
                    // <defaultcodec>
                    readElementText();
                    // </defaultcodec>
                } else if (isStartElement()
                        && name().toString().startsWith(strextrans)) {
                    // <extra-...>
                    readElementText();
                    // </extra-...>
                } else if (elementStarts(strdependencies)) {
                    /*
                     * <dependencies>
                     *   <dependency catalog="qtsystems_no"/>
                     *   <dependency catalog="qtbase_no"/>
                     * </dependencies>
                     **/
                    QStringList dependencies;
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </dependencies> found, finish local loop
                            break;
                        } else if (elementStarts(strdependency)) {
                            // <dependency>
                            QXmlStreamAttributes atts = attributes();
                            dependencies.append(atts.value(strcatalog).toString());
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </dependency> found, finish local loop
                                    break;
                                }
                            }
                        }
                    }
                } else if (elementStarts(strcontext)) {
                    // <context>
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </context> found, finish local loop
                            break;
                        } else if (isWhiteSpace()) {
                            // ignore these, just whitespace
                        } else if (elementStarts(strname)) {
                            // <name>
                            sourceContext = readElementText();
                            // </name>
                        } else if (elementStarts(strmessage)) {
                            // <message>
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </message> found, finish local loop
                                    TransMessage message;
                                    TransMessage *pCurMessage = new TransMessage;
                                    bool find=false;
                                    pCurMessage->setIndex(QString::number(pTransLanguage->m_transMessageList.length()));
                                    pCurMessage->setSourceText(sourceText);
                                    pCurMessage->setContext(sourceContext);
                                    pCurMessage->setmd5();
                                    foreach (message, pTransLanguage->m_transMessageList) {
                                        if (message.md5() == pCurMessage->md5()) {
                                            find = true;
                                            break;
                                        }
                                    }

                                    if (!find) {
                                        pTransLanguage->m_transMessageList.append(*pCurMessage);
                                    } else
                                        delete pCurMessage;

                                    break;
                                } else if (isWhiteSpace()) {
                                    // ignore these, just whitespace
                                } else if (elementStarts(strsource)) {
                                    // <source>...</source>
                                    sourceText = readElementText();
                                } else if (elementStarts(stroldsource)) {
                                    // <oldsource>...</oldsource>
                                    readElementText();
                                } else if (elementStarts(stroldcomment)) {
                                    // <oldcomment>...</oldcomment>
                                    readElementText();
                                } else if (elementStarts(strextracomment)) {
                                    // <extracomment>...</extracomment>
                                    readElementText();
                                } else if (elementStarts(strtranslatorcomment)) {
                                    // <translatorcomment>...</translatorcomment>
                                    readElementText();
                                } else if (elementStarts(strlocation)) {
                                    // <location/>
//                                    QXmlStreamAttributes atts = attributes();
//                                    QString fileName = atts.value(strfilename).toString();
//                                    QString lin = atts.value(strline).toString();
                                    readContents();
//                                    qDebug()<<QString("Location fileName: %1, Line:%2").arg(fileName).arg(lin);
                                } else if (elementStarts(strcomment)) {
                                    // <comment>...</comment>
                                    readElementText();
                                } else if (elementStarts(struserdata)) {
                                    // <userdata>...</userdata>
                                    readElementText();
                                } else if (elementStarts(strtranslation)) {
                                    // <translation>
//                                    QXmlStreamAttributes atts = attributes();
//                                    QStringRef type = atts.value(strtype);
//                                    if (type == strunfinished)
//                                        qDebug("translation ---unfinished");
//                                    else if (type == strvanished)
//                                       qDebug("translation ---vanished");
//                                    else if (type == strobsolete)
//                                        qDebug("translation ---obsolete");
                                    sourceTrans = readElementText();
//                                    qDebug()<<"Context: "<<sourceContext<<"Source: "<<sourceText<<"Translation: "<<sourceTrans;
                                    // </translation>
                                } else if (isStartElement()
                                        && name().toString().startsWith(strextrans)) {
                                    // <extra-...>
                                    readElementText();
                                    // </extra-...>
                                } else {
                                    qDebug("Error--Unknow token within <message>..</message>");
                                }
                            }
                            // </message>
                        } else {
                            qDebug("Error--Unknow token within <context>..</context>");
                        }
                    }
                    // </context>
                } else {
                    qDebug("Error--Unknow token within <TS>..</TS>");
                }
            } // </TS>
        } else if (isWhiteSpace()) {
            // ignore these, just whitespace
        } else {
            qDebug("Error--Unknow token within this Document");
        }
    }

    if (hasError()) {
        qDebug("Error--Doc read Error, please check....");
        return false;
    }

    return true;
}

bool TSReader::release(TransLanguage &transLanguage)
{
    QString sourceContext, sourceText, sourceTrans;

    STRING(byte);
    STRING(catalog);
    STRING(comment);
    STRING(context);
    STRING(defaultcodec);
    STRING(dependencies);
    STRING(dependency);
    STRING(extracomment);
    STRING(filename);
    STRING(id);
    STRING(language);
    STRING(line);
    STRING(location);
    STRING(message);
    STRING(name);
    STRING(numerus);
    STRING(numerusform);
    STRING(obsolete);
    STRING(oldcomment);
    STRING(oldsource);
    STRING(source);
    STRING(sourcelanguage);
    STRING(translation);
    STRING(translatorcomment);
    STRING(TS);
    STRING(type);
    STRING(unfinished);
    STRING(userdata);
    STRING(value);
    STRING(vanished);

    static const QString strextrans(QLatin1String("extra-"));

    QFile tsFile(transLanguage.m_languageTag + QString(".ts"));
    if (!tsFile.open(QFile::WriteOnly | QFile::Truncate)) {
        qDebug("Error---Dest TS file open failed");
        return false;
    }

    QTextStream t(&tsFile);
    t.setCodec(QTextCodec::codecForName("UTF-8"));

    t << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n";
    t << "<TS version=\"2.1\"";
    t << " language=\"" << transLanguage.m_languageTag << "\"";
    t << ">\n";

    while (!atEnd()) {
        readNext();
        if (isStartDocument()) {
            // <!DOCTYPE TS>
        } else if (isEndDocument()) {
            // <!DOCTYPE TS>
        } else if (isDTD()) {
            // <!DOCTYPE TS>
        } else if (elementStarts(strTS)) {
            // <TS>
            while (!atEnd()) {
                readNext();
                if (isEndElement()) {
                    // </TS> found, finish local loop
                    t << "</TS>\n";
                    break;
                } else if (isWhiteSpace()) {
                    // ignore these, just whitesp{ace
                } else if (elementStarts(strdefaultcodec)) {
                    // <defaultcodec>
                    readElementText();
                    // </defaultcodec>
                } else if (isStartElement()
                        && name().toString().startsWith(strextrans)) {
                    // <extra-...>
                    readElementText();
                    // </extra-...>
                } else if (elementStarts(strdependencies)) {
                    /*
                     * <dependencies>
                     *   <dependency catalog="qtsystems_no"/>
                     *   <dependency catalog="qtbase_no"/>
                     * </dependencies>
                     **/
                    QStringList dependencies;
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </dependencies> found, finish local loop
                            break;
                        } else if (elementStarts(strdependency)) {
                            // <dependency>
                            QXmlStreamAttributes atts = attributes();
                            dependencies.append(atts.value(strcatalog).toString());
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </dependency> found, finish local loop
                                    break;
                                }
                            }
                        }
                    }
                } else if (elementStarts(strcontext)) {
                    // <context>
                    t << "<context>\n";
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </context> found, finish local loop
                            t << "</context>\n";
                            break;
                        } else if (isWhiteSpace()) {
                            // ignore these, just whitespace
                        } else if (elementStarts(strname)) {
                            // <name>
                            t << "    <name>";
                            sourceContext = readElementText();
                            t << sourceContext<<"</name>\n";
                            // </name>
                        } else if (elementStarts(strmessage)) {
                            // <message>
                            t <<  "    <message>\n";
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </message> found, finish local loop
                                    t << "    </message>\n";
                                    break;
                                } else if (isWhiteSpace()) {
                                    // ignore these, just whitespace
                                } else if (elementStarts(strsource)) {
                                    // <source>...</source>
                                    sourceText = readElementText();
                                    t << "        <source>"
                                      << sourceText
                                      << "</source>\n";
                                } else if (elementStarts(stroldsource)) {
                                    // <oldsource>...</oldsource>
                                    readElementText();
                                } else if (elementStarts(stroldcomment)) {
                                    // <oldcomment>...</oldcomment>
                                    readElementText();
                                } else if (elementStarts(strextracomment)) {
                                    // <extracomment>...</extracomment>
                                    readElementText();
                                } else if (elementStarts(strtranslatorcomment)) {
                                    // <translatorcomment>...</translatorcomment>
                                    readElementText();
                                } else if (elementStarts(strlocation)) {
                                    // <location/>
                                    QXmlStreamAttributes atts = attributes();
                                    QString fileName = atts.value(strfilename).toString();
                                    QString lin = atts.value(strline).toString();
                                    readContents();
//                                    qDebug()<<QString("Location fileName: %1, Line:%2").arg(fileName).arg(lin);
                                    t << "        <location";
                                    if (!fileName.isEmpty())
                                        t << " filename=\"" << fileName << "\"";
                                    if (!lin.isEmpty())
                                        t << " line=\"" << lin << "\"";
                                    t << "/>\n";
                                } else if (elementStarts(strcomment)) {
                                    // <comment>...</comment>
                                    readElementText();
                                } else if (elementStarts(struserdata)) {
                                    // <userdata>...</userdata>
                                    readElementText();
                                } else if (elementStarts(strtranslation)) {
                                    // <translation>
                                    bool transFlag = false;
                                    QXmlStreamAttributes atts = attributes();
                                    QStringRef type = atts.value(strtype);
                                    TransMessage curMessage;

                                    t << "        <translation";
                                    if (type == strvanished) {
                                        t << " type=\"vanished\">";
                                    } else if (type == strobsolete) {
                                        t << " type=\"obsolete\">";
                                    } else { //"unfinished or empty"
                                        t << ">";
                                        transFlag = true;
                                    }
                                    sourceTrans = readElementText();

                                    if (transFlag){
                                        curMessage.setContext(sourceContext);
                                        curMessage.setSourceText(sourceText);
                                        curMessage.setTransText(QString());
                                        curMessage.setmd5();

                                        //search current message in message list
                                        TransMessage tmpMessage;
                                        foreach (tmpMessage, transLanguage.m_transMessageList) {
                                            if (tmpMessage.md5() == curMessage.md5()){
                                                curMessage.setTransText(tmpMessage.TransText());
                                                break;
                                            }
                                        }
                                    }
                                    sourceTrans = curMessage.TransText();

                                    if (!sourceTrans.isEmpty())
                                        t << sourceTrans;
                                    t << "</translation>\n";
                                    // </translation>
                                } else if (isStartElement()
                                        && name().toString().startsWith(strextrans)) {
                                    // <extra-...>
                                    readElementText();
                                    // </extra-...>
                                } else {
                                    qDebug("Error--Unknow token within <message>..</message>");
                                }
                            }
                            // </message>
                        } else {
                            qDebug("Error--Unknow token within <context>..</context>");
                        }
                    }
                    // </context>
                } else {
                    qDebug("Error--Unknow token within <TS>..</TS>");
                }
            } // </TS>
        } else if (isWhiteSpace()) {
            // ignore these, just whitespace
        } else {
            qDebug("Error--Unknow token within this Document");
        }
    }

    if (hasError()) {
        qDebug("Error--Doc read Error, please check....");
        return false;
    }

    return true;
}

void Translator::registerTransWorker(const Translator::TransWorker &worker)
{
    QList<Translator::TransWorker> &workers = registeredTransWorkers();

    workers.append(worker);
}

QList<Translator::TransWorker> &Translator::registeredTransWorkers()
{
    static QList<Translator::TransWorker> theWorkers;
    return theWorkers;
}

bool Translator::execTranslator()
{
    QList<Translator::TransWorker> &workers = Translator::registeredTransWorkers();

    if (workers.isEmpty()){
        qDebug()<<"No work to handle...";
        return false;
    }

    TransWorker work;
    foreach (work, workers){
        switch (work.mode)
        {
        case TransWorker::TransUpdate:
            updateTranslator(work);
            break;
        case TransWorker::TransRelease:
            releaseTranslator(work);
            break;
        default:
            qDebug()<<"Wrong operation mode!";
            break;
        }
    }

    return false;
}

/**
 * Add new text field from *.ts file to the *.xlsx file
 */
bool Translator::updateTranslator(const TransWorker &work)
{
    QFile *pFile = new QFile(work.fileInOut.first);

     if(!pFile || !pFile->open(QFile::ReadWrite | QFile::Text)){
        qDebug("Error----XML File open failed!");
         return false;
     }

    TSReader    tsReader(*pFile);
    QXlsx::Document xlsx(work.fileInOut.second);
    TransLanguage   *pTransLanguage = new TransLanguage;

    //read from *.xlsx
    QString str;
    for (int row=TRANS_START_ROW; str.compare(QString("END"), Qt::CaseInsensitive) != 0; ++row){
        str = xlsx.read(row, TRANS_COLU_INDEX).toString();

        if (str.compare(QString("END"), Qt::CaseInsensitive) == 0)
            break;

        if (str.isEmpty() || str.isNull()){
            qDebug("Language Translation contain row of blank, please check...");
             break;
        }

        TransMessage *pTransMessage = new TransMessage;
        pTransMessage->setIndex(xlsx.read(row, TRANS_COLU_INDEX).toString());
        pTransMessage->setSourceText(xlsx.read(row, TRANS_COLU_SOURCE).toString());
        pTransMessage->setContext(xlsx.read(row, TRANS_COLU_CONTEXT).toString());
        pTransMessage->setComment(xlsx.read(row, TRANS_COLU_COMMENT).toString());
        pTransMessage->setmd5();
        pTransLanguage->m_transMessageList.append(*pTransMessage);
    }

    tsReader.update(pTransLanguage);

    TransMessage *pEndMessage = new TransMessage;
    pEndMessage->setIndex(QString("END"));
    pTransLanguage->m_transMessageList.append(*pEndMessage);

    //update to *.xlsx
    int row = TRANS_START_ROW;
    foreach (TransMessage message, pTransLanguage->m_transMessageList) {
        if (message.index() == QString("END")) {
            xlsx.write(row, TRANS_COLU_INDEX, message.index());
            break;
        } else {
            xlsx.write(row, TRANS_COLU_INDEX, message.index());
            xlsx.write(row, TRANS_COLU_SOURCE, message.sourceText());
            xlsx.write(row, TRANS_COLU_CONTEXT, message.context());
            xlsx.write(row, TRANS_COLU_COMMENT, message.comment());

            ++row;
        }
    }
    QXlsx::Format headerFormat;
    headerFormat.setBorderColor(QColor(60, 60, 60));
    headerFormat.setBorderStyle(QXlsx::Format::BorderThin);
    headerFormat.setFontBold(true);
    headerFormat.setPatternBackgroundColor(QColor(0,180,60));
    headerFormat.setPatternForegroundColor(QColor(255,255,255));
    headerFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    xlsx.write(1,1, QString("Version:%1 Date:%2").arg(QString(SW_VERSION)).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd")),headerFormat);
    xlsx.mergeCells(QXlsx::CellRange(1,1,1,4));
    xlsx.write(2,1, QString("Index"),headerFormat);
    xlsx.write(2,2, QString("Source"),headerFormat);
    xlsx.write(2,3, QString("Context"),headerFormat);
    xlsx.write(2,4, QString("Comment"),headerFormat);
    xlsx.setRowFormat(1,2,headerFormat);

    xlsx.save();

    delete pTransLanguage;

    return true;
}

/**
 * Read all translation from *.xlsx and generate "language".ts for each language
 */
bool Translator::releaseTranslator(const TransWorker &work)
{
    QFile *pFile = new QFile(work.fileInOut.first);

    //read translation language from *.xlsx
    registerTransLanguage(work);

    //update language *.ts
    TransLanguage language;
    foreach (language, Translator::registeredTransLanguages()) {
        if (!pFile || !pFile->open(QIODevice::ReadWrite | QFile::Text)){
            qDebug("Error----TS File open failed");
            return false;
        }

        TSReader    tsReader(*pFile);

        tsReader.release(language);

        pFile->close();
    }

    return true;
}

void Translator::registerTransLanguage(const TransWorker &work)
{
    int IndexStart, IndexEnd;
    QXlsx::Document xlsx(work.fileInOut.second);

    IndexStart = work.range.first;
    IndexEnd = work.range.second;

    //language list append && trans message list append from *.excel
    for (int langIndex=IndexStart; langIndex<=IndexEnd; ++langIndex){
        QString str;
        TransLanguage   *pTransLanguage = new TransLanguage;
        pTransLanguage->m_language = xlsx.read(1, langIndex).toString();
        pTransLanguage->m_languageTag = xlsx.read(2, langIndex).toString();

        for (int row=TRANS_START_ROW; str.compare(QString("END"), Qt::CaseInsensitive) != 0; ++row){
            str = xlsx.read(row, TRANS_COLU_INDEX).toString();

            if(str.compare(QString("END"), Qt::CaseInsensitive) == 0)
                break;

            if (str.isEmpty() || str.isNull()){
                qDebug("Language Translation contain row of blank, please check...");
                 break;
            }

            TransMessage *pTransMessage = new TransMessage;
            pTransMessage->setIndex(xlsx.read(row, TRANS_COLU_INDEX).toString());
            pTransMessage->setSourceText(xlsx.read(row, TRANS_COLU_SOURCE).toString());
            pTransMessage->setContext(xlsx.read(row, TRANS_COLU_CONTEXT).toString());
            pTransMessage->setComment(xlsx.read(row, TRANS_COLU_COMMENT).toString());
            pTransMessage->setmd5();
            pTransMessage->setTransText(xlsx.read(row, langIndex).toString());
            pTransLanguage->m_transMessageList.append(*pTransMessage);
        }

        Translator::registeredTransLanguages().append(*pTransLanguage);
    }
}

QList<TransLanguage> &Translator::registeredTransLanguages()
{
    static QList<TransLanguage> languages;
    return languages;
}
