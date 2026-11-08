#ifndef FMACLIPBOARD_H
#define FMACLIPBOARD_H
#include <QClipboard>

#include "fmatransdataobject.h"

class FmaClipboard
{
public:
    enum Format { Ole = 0, MathMl = 1, Tex = 2 };

    FmaClipboard();
    virtual ~FmaClipboard();

    static int format();
    static void setFormat(int format);

    const QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard );
    void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
    void releaseClipboard(QClipboard::Mode mode = QClipboard::Clipboard);

    static FmaClipboard instance;
private:

    FmaTransDataObject *transData = nullptr;

    void releasTransaData();

};

#endif // FMACLIPBOARD_H
