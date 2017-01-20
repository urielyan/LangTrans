#ifndef TRANSLANGUAGE_H
#define TRANSLANGUAGE_H

#include <QString>
#include <QLocale>

#include "transmessage.h"

class TransLanguage
{
public:
    TransLanguage();

public:
    QList<TransMessage>  m_transMessageList; //translate message list
    QString m_language;                                        //langugae
    QString m_languageTag;                                  //language and country code  ->
                                                                               //language: two-letter ISO 639 language code;
                                                                               //country: two- or three-letter ISO 3166 country code
};

#endif // TRANSLANGUAGE_H
