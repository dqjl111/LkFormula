#ifndef LKTHEME_H
#define LKTHEME_H

#include <QString>

class QApplication;

// Global UI theme: Fusion style + palette + QSS (dark by default).
namespace LkTheme {

void apply(QApplication *app, const QString &style = QStringLiteral("light"));

}

#endif // LKTHEME_H
