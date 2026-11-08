#ifndef FMAMMLTABLE_H
#define FMAMMLTABLE_H

#include <QHash>

class FmaMmlTable
{
public:
    FmaMmlTable();

    static QString entities(QString text, uint &prefix_lines);
    static QList<QString> searchNameByValue(const QString &value);
    static QString searchValueByName(const QString& name);
    static QString searchCodeByName(const QString& name);
    static QString searchByText(const QString& name);

    static QString mmlDecodeEntityValue( QString literal );

    static void print(QString dest);
    static QStringList littleGreekLetters();  //希腊小写
    static QStringList bigGreekLetters(); //希腊大写
    static QStringList compareLetters(); //比较运算
    static QStringList mathsetLetters(); //集合运算
    static QStringList conditionLetters(); //条件运算
    static QStringList arrowLetters(); //箭头
    static QStringList extraAddLetters();
    static QStringList extraNoteLetters();

    static QString texOverLiteral(const QString& key);
    static QString letterToTex(const QString& letter);
private:
    static QHash<QString, QString> valueLookup;
    static QMultiHash<QString, QString> nameLookup;
    static QHash<QString, QString> texOverLookup;
    static QHash<QString, QString> texLetterLookup;

    static QHash<QString, QString> initValueLookup();
    static QMultiHash<QString, QString> initNameLookup();
    static QHash<QString, QString> initTexOverLookup();
    static QHash<QString, QString> initTexLetterLookup();

};

#endif // FMAMMLTABLE_H
