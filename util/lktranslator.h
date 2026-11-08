#ifndef LKTRANSLATOR_H
#define LKTRANSLATOR_H

#include <QString>

class QObject;

namespace LkTranslator {

// Installs the translator according to the saved language or the system locale.
void install();

// QObjects that need to refresh dynamic strings on a language change.
void addLanguageObserver(QObject *object);

// "zh_CN" loads the Chinese translation, "en" restores the source strings.
// An empty language means "use the saved / system language".
bool setLanguage(const QString &language = QString());

QString currentLanguage();

}

#endif // LKTRANSLATOR_H
