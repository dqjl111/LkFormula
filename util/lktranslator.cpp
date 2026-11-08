#include "lktranslator.h"

#include <QCoreApplication>
#include <QEvent>
#include <QList>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <QTranslator>

namespace {

const char *kOrganization = "LkFormula";
const char *kApplication = "LkFormula";
const char *kLanguageKey = "Language";

QList<QPointer<QObject>> observers;

QTranslator *translator()
{
    static QTranslator *t = new QTranslator(QCoreApplication::instance());
    return t;
}

QString qmPath(const QString &language)
{
    return QStringLiteral(":/translations/LkFormula_%1.qm").arg(language);
}

QSettings appSettings()
{
    return QSettings(QLatin1String(kOrganization), QLatin1String(kApplication));
}

}

namespace LkTranslator {

void addLanguageObserver(QObject *object)
{
    if (object)
        observers.append(QPointer<QObject>(object));
}

QString currentLanguage()
{
    const QString saved = appSettings().value(QLatin1String(kLanguageKey)).toString();
    if (!saved.isEmpty())
        return saved;

    return QLocale::system().language() == QLocale::Chinese
            ? QStringLiteral("zh_CN") : QStringLiteral("en");
}

bool setLanguage(const QString &language)
{
    QTranslator *t = translator();
    QCoreApplication::removeTranslator(t);

    const QString lang = language.isEmpty() ? currentLanguage() : language;
    if (lang != QLatin1String("en")) {
        if (!t->load(qmPath(lang)))
            return false;
        QCoreApplication::installTranslator(t);
    }

    QSettings settings = appSettings();
    settings.setValue(QLatin1String(kLanguageKey), lang);

    QEvent event(QEvent::LanguageChange);
    for (auto it = observers.begin(); it != observers.end(); ) {
        if (it->isNull()) {
            it = observers.erase(it);
        } else {
            QCoreApplication::sendEvent(it->data(), &event);
            ++it;
        }
    }
    return true;
}

void install()
{
    setLanguage(currentLanguage());
}

}
