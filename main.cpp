#include "fmamain.h"

#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QTextCodec>
#include <QMetaObject>
#include <utility>
#include "util/lktranslator.h"
#include "util/lktheme.h"

#ifdef _WIN32
#include <QAxFactory>
#include <QAxAggregated>
#include <qt_windows.h>
#include "fmabinder.h"


extern wchar_t qAxModuleFilename[MAX_PATH];

QString axType(const QString &type, const QMetaObject *mo)
{
    if (mo) {
        int classInfoIdx = mo->indexOfClassInfo("CoClassAlias");
        if (classInfoIdx != -1) {
            const QMetaClassInfo classInfo = mo->classInfo(classInfoIdx);
            return QLatin1String(classInfo.value());
        }
    }

    QString alias(type);
    alias.remove(QLatin1String("::"));
    return alias;
}

template<class T>
class FmaAxClass : public QAxClass<T>
{
public:
    FmaAxClass(const QString &libId, const QString &appId)
        : QAxClass<T>(libId, appId)
    {}

    void registerClass(const QString &key, QSettings *settings) const override
    {
        const QChar dot(QLatin1Char('.'));
        const QChar slash(QLatin1Char('/'));
        QString file = QString::fromWCharArray(qAxModuleFilename);
        const QMetaObject *mo = QAxClass<T>::metaObject(key);
        const QString classId = QAxClass<T>::classID(key).toString().toUpper();
        const QString className = axType(key, mo);
        const QString module = QFileInfo(file).baseName();


        QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
        if (classVersion.isNull()){
            classVersion = QLatin1String("1.0");
        }

        QString classMajorVersion = classVersion.left(classVersion.indexOf(dot));
        const QString versionLessProgId = module + dot + className;
        const QString progId = versionLessProgId + dot + classMajorVersion;
        QString prefix = slash + progId;
        if(QLocale::system().language() == QLocale::Chinese){
            settings->setValue(prefix + QLatin1String("/."), QString("FMA 公式"));
        }else{
            settings->setValue(prefix + QLatin1String("/."), QLatin1String("FMA Equation"));
        }

        settings->setValue(prefix + QLatin1String("/DefaultIcon/."), file);
        settings->setValue(prefix + QLatin1String("/Protocol/StdExecute/Server/."), file);

        QString stdFile = prefix + QLatin1String("/Protocol/StdFileEditing/");
        settings->setValue(stdFile + QLatin1String("RequestDataFormats/."), QLatin1String("Native,CF_METAFILEPICT,EqnDims"));
        settings->setValue(stdFile + QLatin1String("Server/."), file);
        settings->setValue(stdFile + QLatin1String("Verb/0/."), QLatin1String("Edit"));
        settings->setValue(stdFile + QLatin1String("SetDataFormats/."), QLatin1String("Native,RTF"));

        QString shell = prefix +  QLatin1String("/Shell/");
        settings->setValue(shell + QLatin1String("open/command/."), file);

        prefix = QLatin1String("/CLSID/") + classId;
        settings->setValue(prefix + QLatin1String("/InProcHandler/."), QLatin1String("ole2.dll"));
        settings->setValue(prefix + QLatin1String("/InprocHandler32/."), QLatin1String("ole32.dll"));
        settings->setValue(prefix + QLatin1String("/LocalServer/."), file);
        if(QLocale::system().language() == QLocale::Chinese){
            settings->setValue(prefix + QLatin1String("/AuxUserType/2/."), QString("公式"));
            settings->setValue(prefix + QLatin1String("/AuxUserType/3/."), QString("FMA 公式"));
        }else{
            settings->setValue(prefix + QLatin1String("/AuxUserType/2/."), QLatin1String("Equation"));
            settings->setValue(prefix + QLatin1String("/AuxUserType/3/."), QLatin1String("FMA Equation"));
        }
        settings->setValue(prefix + QLatin1String("/DefaultIcon/."), file);
        //n = "name, menu_flag, verb_flag"
        settings->setValue(prefix + QLatin1String("/Verb/0/."), QLatin1String("&Edit,0,2"));
        settings->setValue(prefix + QLatin1String("/Verb/1/."), QLatin1String("&Open,0,2"));
        settings->remove(prefix + QLatin1String("/MiscStatus/1/")); //remove Qt active settings
        settings->setValue(prefix + QLatin1String("/MiscStatus/16/."), QString::number(1));

        QString dataFmts = prefix + QLatin1String("/DataFormats/");
        //n = format, aspect(DVASPECT 显示方式), medium(TYMED 存取媒介), flag(DATADIR 读写方向)
        settings->setValue(dataFmts + QLatin1String("GetSet/0/."), QLatin1String("Document,1,1,3"));
        settings->setValue(dataFmts + QLatin1String("GetSet/1/."), QLatin1String("Embed_Source,1,8,1"));
        settings->setValue(dataFmts + QLatin1String("GetSet/2/."), QLatin1String("1,1,1,3"));
        settings->setValue(dataFmts + QLatin1String("GetSet/3/."), QLatin1String("3,1,32,1"));

    }

    void unregisterClass(const QString &key, QSettings *settings) const override
    {
        const QChar dot(QLatin1Char('.'));
        const QChar slash(QLatin1Char('/'));
        QString file = QString::fromWCharArray(qAxModuleFilename);
        const QMetaObject *mo = QAxClass<T>::metaObject(key);
        const QString classId = QAxClass<T>::classID(key).toString().toUpper();
        const QString className = axType(key, mo);
        const QString module = QFileInfo(file).baseName();


        QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
        if (classVersion.isNull()){
            classVersion = QLatin1String("1.0");
        }

        QString classMajorVersion = classVersion.left(classVersion.indexOf(dot));
        const QString versionLessProgId = module + dot + className;
        const QString progId = versionLessProgId + dot + classMajorVersion;

        QString prefix = QLatin1String("/CLSID/") + classId;
        QString dataFmts = prefix + QLatin1String("/DataFormats/");
        settings->remove(dataFmts + QLatin1String("GetSet/0/."));
        settings->remove(dataFmts + QLatin1String("GetSet/1/."));
        settings->remove(dataFmts + QLatin1String("GetSet/2/."));
        settings->remove(dataFmts + QLatin1String("GetSet/3/."));
        settings->remove(dataFmts);

        //settings->remove(prefix + QLatin1String("/DocObject/."));
        settings->remove(prefix + QLatin1String("/MiscStatus/16/."));
        settings->remove(prefix + QLatin1String("/Verb/1/."));
        settings->remove(prefix + QLatin1String("/Verb/0/."));
        settings->remove(prefix + QLatin1String("/DefaultIcon/."));
        settings->remove(prefix + QLatin1String("/AuxUserType/3/."));
        settings->remove(prefix + QLatin1String("/AuxUserType/2/."));
        settings->remove(prefix + QLatin1String("/AuxUserType/."));
        settings->remove(prefix + QLatin1String("/InprocHandler/."));
        settings->remove(prefix + QLatin1String("/InprocHandler32/."));
        settings->remove(prefix + QLatin1String("/LocalServer/."));

        prefix = slash + progId;
        QString stdFile = prefix + QLatin1String("/Protocol/StdFileEditing/");
        settings->remove(stdFile + QLatin1String("SetDataFormats/."));
        settings->remove(stdFile + QLatin1String("Verb/0/."));
        settings->remove(stdFile + QLatin1String("Server/."));
        settings->remove(stdFile + QLatin1String("RequestDataFormats/."));
        settings->remove(stdFile);

        settings->remove(prefix + QLatin1String("/Protocol/StdExecute/Server/."));
        settings->remove(prefix + QLatin1String("/DefaultIcon/."));

        QString shell = prefix +  QLatin1String("/Shell/");
        settings->remove(shell + QLatin1String("open/command/."));
        settings->remove(shell);
    }
};

#define FMAAXCLASS(Class) \
    factory = new FmaAxClass<Class>(typeLibID().toString(), appID().toString()); \
    qRegisterMetaType<Class*>(#Class"*"); \
    keys = factory->featureList(); \
    for (const QString &key : std::as_const(keys)) { \
    factoryKeys += key; \
    factories.insert(key, factory); \
    creatable.insert(key, true); \
    }\


QAXFACTORY_BEGIN(
        "{6AE57768-5B31-4C76-9E0E-A4DB9070C622}", // type library ID
        "{24071EF2-61F0-447D-A389-35FA3ECC497A}") // application ID
FMAAXCLASS(FmaBinder)
QAXFACTORY_END()


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral("0.1"));
    LkTheme::apply(&app, QStringLiteral("light"));
    LkTranslator::install();

    if (QAxFactory::isServer()) {
        return app.exec();
    }
    else {
        QAxFactory::startServer();
        FmaBinder appobject;
        appobject.setObjectName(QStringLiteral("From Application"));
        //qWarning() << "Start fma active server.";
        QAxFactory::registerActiveObject(&appobject);
        appobject.setVisible(true, false);
        return app.exec();
    }
}
#else
#include "fmamain.h"
#include "util/lkutil.h"
#include "util/pathconfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral("0.1"));
    LkTheme::apply(&app, QStringLiteral("light"));
    LkTranslator::install();

    QString basedir_str = QCoreApplication::applicationDirPath();
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8);

    PathConfig::initPath(basedir_str + QDir::separator());
    qInstallMessageHandler(LkUtil::outputMessage);

    FmaMain m;
    m.show();
    return app.exec();
}

#endif
