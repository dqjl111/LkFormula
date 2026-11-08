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
#include "PropertiesUtils.h"
#include <QString>
namespace cppproperties {
namespace PropertiesUtils {

bool IsProperty(const QString& str) {
    QString::size_type s = str.indexOf("=");
    if (s <0) {
        return false;
    }
    QString key = str.mid(0, s).trimmed();
    // key can't be empty
    if (key == "") {
        return false;
    }
    return true;
}

QPair<QString, QString> ParseProperty(const QString& str) {
    QString::size_type s = str.indexOf("=");
    QString key = str.mid(0, s).trimmed();
    QString value = str.mid(s+1).trimmed();

    QString::size_type e = value.indexOf("#");
    if(e>=0){
        value = value.mid(0, e).trimmed();
    }
    return QPair<QString, QString>(key, value);
}


} // namespace PropertiesUtils
} // namespace cppproperties

