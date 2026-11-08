#ifndef FMAOPERDICT_H
#define FMAOPERDICT_H
#include <QtCore>
#include <QMap>

class FmaOperDict
{
public:
    FmaOperDict();

    enum FmaOperProp{
    mmlNoProp = 0x00,
    mmlFence =0x01,
    mmlSeparator = 0x02,
    mmlStretchHorizontal = 0x04,
    mmlStretchVertical = 0x08,
    mmlSymmetric = 0x10,
    mmlLargeOp = 0x20,
    mmlMovableLimits = 0x40,
    mmlAccent = 0x80,
    };

    enum FormFix{
    mmlNofix,
    mmlPrefix,
    mmlInfix,
    mmlPostfix
    };

    struct Item
    {
        quint8 prop = 0;
        quint8 lspace = 0;
        quint8 rspace = 0;
    };

    struct Form :public Item{
        quint8 type = mmlNofix;

        Form(){
        }

        Form(const Item t) : Item(t){
        }

        Form(FormFix _type, const Item t) : Item(t), type(_type){
        }

        Form& operator=(const Item& t){
            prop = t.prop;
            lspace = t.lspace;
            rspace = t.rspace;
            return *this;
        }

        QString toString(){
            QString ret;
            switch(type){
            case mmlPrefix:{
                ret.append("[prefix]");
                break;
            }
            case mmlInfix:{
                ret.append("[infix]");
                break;
            }
            case mmlPostfix:{
                ret.append("[postfix]");
                break;
            }
            default:{
                ret.append("[nofix]");
                return ret;
            }
            }

            ret.append("[lspace=" + QString::number(lspace) + "]");
            ret.append("[rspace=" + QString::number(rspace) + "]");

            if(prop & mmlFence){
                ret.append("[fence]");
            }

            if(prop & mmlSeparator){
                ret.append("[separator]");
            }

            if(prop & mmlStretchHorizontal){
                ret.append("[direction:horizontal]");
            }

            if(prop & mmlStretchVertical){
                ret.append("[direction:vertical]");
            }

            if(prop & mmlSymmetric){
                ret.append("[symmetric]");
            }

            if(prop & mmlLargeOp){
                ret.append("[largeop]");
            }

            if(prop & mmlMovableLimits){
                ret.append("[movablelimits]");
            }

            if(prop & mmlAccent){
                ret.append("[accent]");
            }

            return ret;
        }
    };

    static void init();
    static FormFix findForm(const QString& f, Form& form);
    static bool hasForm(const QString& f, FormFix type);

    static QMap<QString, Item> prefixItems;
    static QMap<QString, Item> infixItems;
    static QMap<QString, Item> postfixItems;
};

#endif // FMAOPERDICT_H
