/*
 * Copyright (c) 2014 Fredy Wijaya
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fstream>
#include <iostream>
#include "PropertiesParser.h"
#include "PropertiesUtils.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>

namespace cppproperties {

PropertiesParser::PropertiesParser() {
}

PropertiesParser::~PropertiesParser() {
}

Properties PropertiesParser::Read(const QString& fileName) {
    Properties properties;
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {;
        qWarning() <<  "PropertiesParser::Read(" + fileName + "): Unable to open file for reading.";
    }

    {
        size_t linenr = 0;
        QString line;
        QTextStream is(&file);
        while(!is.atEnd()){
            line = is.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("#")) {
                // ignore it
            } else if (PropertiesUtils::IsProperty(line)) {
                QPair<QString, QString> prop = PropertiesUtils::ParseProperty(line);
                properties.AddProperty(prop.first, prop.second);
            } else {
                qWarning() << "PropertiesParser::Read(" + fileName + "): Invalid line "
                           << linenr <<  ": " << line.data();
            }
            ++linenr;
        }
    }
    file.close();
    return properties;
}

void PropertiesParser::Write(const QString& fileName, const Properties& props) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() <<  "PropertiesParser::Write(" + fileName + "): Unable to open file for writing.";
    }

    {
        QTextStream os(&file);
        const QMap<QString, QString>& propMap = props.GetPropertyMap() ;
        QMap<QString, QString>::const_iterator iter  = propMap.begin();
        for(;iter != propMap.end(); ++iter){
            os << iter.key() << " = " << iter.value() << "\n";
        }
    }
    file.close();
}

} /* namespace cppproperties */
