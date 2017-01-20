#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QString>
#include <QPair>

#include "translanguage.h"

#define TRANS_LANG_ROW  2
#define TRANS_START_ROW 3
#define TRANS_COLU_INDEX 1
#define TRANS_COLU_SOURCE   2
#define TRANS_COLU_CONTEXT  3
#define TRANS_COLU_COMMENT 4

#define LANGUAGE_STRAT_INDEX    5
#define LANGUAGE_NUMS   50

#define SW_VERSION  "1.0"

class Translator
{
public:
    Translator(){}

    struct TransWorker {
        TransWorker() {}
        enum workMode { TransUpdate, TransRelease } mode;   //"update", "release"
        QPair<int, int> range;                                                           //index range of language
        QPair<QString, QString> fileInOut;                                      //filename of input && output
    };
    static void registerTransWorker(const Translator::TransWorker &worker);
    static QList<Translator::TransWorker> &registeredTransWorkers();
    bool execTranslator();
    bool updateTranslator(const TransWorker &work);
    bool releaseTranslator(const TransWorker &work);

private:
    void registerTransLanguage(const TransWorker &work);
    static QList<TransLanguage> &registeredTransLanguages();

private:
};

#endif // TRANSLATOR_H
