/* Language Translater
 *
 * This tool is used to automatic update and generate translation through a template file(*.xlsx)
 *
 * This application works in two mode:
 *      -update:
 *              generate or update the dest *.xlsx File, according the context in *.ts.
 *      -release:
 *              according the translations in *.xlsx and the context in *.ts, generate "language".ts for each language.
 */

#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include "xlsxdocument.h"
#include "translator.h"

#define DEFAULT_FILENAME_XLSX   "f260_language.xlsx"
#define DEFAULT_FILENAME_TS "f260_trans.ts"

class Translator;

int main(int argc, char *argv[])
{
    int startIndex,endIndex;
    bool startValidFlag, endValidFlag;
    Translator *pTranslator = new Translator;

    QCoreApplication a(argc, argv);

    //parameter parsing
    if (QCoreApplication::arguments().count() == 3 &&
             QCoreApplication::arguments().at(1).compare(QString("-update"), Qt::CaseInsensitive) == 0){
        QString tsFileName = QCoreApplication::arguments().at(2);
        QFile tsFile(tsFileName);
        if (!tsFile.exists()) {
            qDebug()<<QString("File(%1) not exists, please check...").arg(tsFileName);
            goto USAGE;
        } else {
            Translator::TransWorker *pWorker = new Translator::TransWorker;
            pWorker->mode = Translator::TransWorker::TransUpdate;
            pWorker->fileInOut.first = QCoreApplication::arguments().at(2);
            pWorker->fileInOut.second = QString(DEFAULT_FILENAME_XLSX);
            Translator::registerTransWorker(*pWorker);
        }
    } else if (QCoreApplication::arguments().count() == 4 &&
               QCoreApplication::arguments().at(1).compare(QString("-release"), Qt::CaseInsensitive) == 0){
        startIndex = QCoreApplication::arguments().at(2).toInt(&startValidFlag);
        endIndex = QCoreApplication::arguments().at(3).toInt(&endValidFlag);

        if (startValidFlag && endValidFlag && (startIndex>=LANGUAGE_STRAT_INDEX) && \
            (startIndex<=endIndex) && (endIndex<=LANGUAGE_NUMS)){
            Translator::TransWorker *pWorker = new Translator::TransWorker;
            pWorker->mode = Translator::TransWorker::TransRelease;
            pWorker->range = qMakePair(startIndex, endIndex);
            pWorker->fileInOut.first = QString(DEFAULT_FILENAME_TS);
            pWorker->fileInOut.second = QString(DEFAULT_FILENAME_XLSX);

            Translator::registerTransWorker(*pWorker);
        } else {
            qDebug("Wrong Parameter! Please check...");
            goto USAGE;
        }

    } else {
        goto USAGE;
    }

    return pTranslator->execTranslator();

//    return a.exec();

USAGE:
    qWarning("Usage:\
\n\t1. LangTrans [-update path/to/lang_file(*.ts)]\
\n\t\tGenerate or Add new text field to Excel file;\
\n\t2. LangTrans [-release indexStart(min: 5) indexEnd(max: 54)]\
\n\t\tUpdate translation to the language(indexStart-indexEnd) files(*.ts) ");

    return -1;
}
