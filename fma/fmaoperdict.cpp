#include "fmaoperdict.h"
#include "prop/PropertiesParser.h"
#include <QDebug>

QMap<QString, FmaOperDict::Item> FmaOperDict::prefixItems;
QMap<QString, FmaOperDict::Item> FmaOperDict::infixItems;
QMap<QString, FmaOperDict::Item> FmaOperDict::postfixItems;

using namespace cppproperties;

FmaOperDict::FmaOperDict()
{

}

void FmaOperDict::init()
{
    PropertiesParser parser;
    Properties prop = parser.Read(":/mathfont.properties");
    const QMap<QString, QString>& props = prop.GetPropertyMap() ;
    QMap<QString, QString>::const_iterator iter  = props.begin();
    QStringList keyItems, valueItems, t;
    QString tLiteral;
    for(;iter != props.end(); ++iter){
        keyItems = iter.key().split(".");
        if(keyItems.size() >= 3){
            QString itemKey;
            t = keyItems[1].split("\\u");
            for(const QString& d: t){
                if(d.isEmpty()){
                    continue;
                }
                bool ok;
                unsigned short c = d.toUShort(&ok, 16 );
                if ( !ok )
                {
                    qWarning() << "decode fma prop bad literal: \"" + d + "\"";
                }
                itemKey += QChar( c );
            }

            if(!itemKey.isEmpty()){
                valueItems = iter.value().split(" ");
                Item item;
                for(QString& v: valueItems){
                    v = v.trimmed();
                    if(v.startsWith("lspace")){
                        t = v.split(":");
                        if(t.size()>=2){
                            item.lspace = t[1].toUInt();
                        }
                    }else if(v.startsWith("rspace")){
                        t = v.split(":");
                        if(t.size()>=2){
                            item.rspace = t[1].toUInt();
                        }
                    }else if(v.startsWith("fence")){
                        item.prop |= mmlFence;
                    }else if(v.startsWith("separator")){
                        item.prop |= mmlSeparator;
                    }else if(v.startsWith("direction")){
                        if(iter.value().contains("stretchy")){
                            t = v.split(":");
                            if(t.size()>=2){
                                if(t[1] == "horizontal"){
                                    item.prop |= mmlStretchHorizontal;
                                }else if(t[1] == "vertical"){
                                    item.prop |= mmlStretchVertical;
                                }
                            }
                        }
                    }else if(v.startsWith("symmetric")){
                        item.prop |= mmlSymmetric;
                    }else if(v.startsWith("largeop")){
                        item.prop |= mmlLargeOp;
                    }else if(v.startsWith("movablelimits")){
                        item.prop |= mmlMovableLimits;
                    }else if(v.startsWith("accent")){
                        item.prop |= mmlAccent;
                    }
                }

                QString form = keyItems[2].trimmed();

                if(form == "prefix"){
                    prefixItems[itemKey] = item;
                }else if(form == "infix"){
                    infixItems[itemKey] = item;
                }else if(form == "postfix"){
                    postfixItems[itemKey] = item;
                }
            }
        }
    }
}

FmaOperDict::FormFix FmaOperDict::findForm(const QString &f, Form &form)
{
    switch(form.type){
    case FmaOperDict::mmlNofix:{
        QMap<QString, Item>::iterator iter = infixItems.find(f);
        if(iter != infixItems.end()){
            form.type = FmaOperDict::mmlInfix;
            form = *iter;
            return FmaOperDict::mmlInfix;
        }

        iter = prefixItems.find(f);
        if(iter != prefixItems.end()){
            form.type = FmaOperDict::mmlPrefix;
            form = *iter;
            return FmaOperDict::mmlPrefix;
        }

        iter = postfixItems.find(f);
        if(iter != postfixItems.end()){
            form.type = FmaOperDict::mmlPostfix;
            form = *iter;
            return FmaOperDict::mmlPostfix;
        }
        break;
    }
    case FmaOperDict::mmlPrefix:{
        QMap<QString, Item>::iterator iter = prefixItems.find(f);
        if(iter != prefixItems.end()){
            form.type = FmaOperDict::mmlPrefix;
            form = *iter;
            return FmaOperDict::mmlPrefix;
        }
        break;
    }
    case FmaOperDict::mmlInfix:{
        QMap<QString, Item>::iterator iter = infixItems.find(f);
        if(iter != infixItems.end()){
            form.type = FmaOperDict::mmlInfix;
            form = *iter;
            return FmaOperDict::mmlInfix;
        }
        break;
    }
    case FmaOperDict::mmlPostfix:{
        QMap<QString, Item>::iterator iter = postfixItems.find(f);
        if(iter != postfixItems.end()){
            form.type = FmaOperDict::mmlPostfix;
            form = *iter;
            return FmaOperDict::mmlPostfix;
        }
        break;
    }
    }
    return FmaOperDict::mmlNofix;
}

bool FmaOperDict::hasForm(const QString &f, FormFix type)
{
    switch(type){
    case FmaOperDict::mmlNofix:{
        QMap<QString, Item>::iterator iter = infixItems.find(f);
        if(iter != infixItems.end()){
            return true;
        }

        iter = prefixItems.find(f);
        if(iter != prefixItems.end()){
            return true;
        }

        iter = postfixItems.find(f);
        if(iter != postfixItems.end()){
            return true;
        }
        break;
    }
    case FmaOperDict::mmlPrefix:{
        QMap<QString, Item>::iterator iter = prefixItems.find(f);
        if(iter != prefixItems.end()){
            return true;
        }
        break;
    }
    case FmaOperDict::mmlInfix:{
        QMap<QString, Item>::iterator iter = infixItems.find(f);
        if(iter != infixItems.end()){
            return true;
        }
        break;
    }
    case FmaOperDict::mmlPostfix:{
        QMap<QString, Item>::iterator iter = postfixItems.find(f);
        if(iter != postfixItems.end()){
            return true;
        }
        break;
    }
    }
    return FmaOperDict::mmlNofix;
}

