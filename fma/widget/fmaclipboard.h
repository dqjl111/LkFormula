#ifndef FMACLIPBOARD_H
#define FMACLIPBOARD_H
#include <QClipboard>

#include "fmatransdataobject.h"

class FmaClipboard
{
public:
    FmaClipboard();
    virtual ~FmaClipboard();

    const QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard );
    void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
    void releaseClipboard(QClipboard::Mode mode = QClipboard::Clipboard);

    static FmaClipboard instance;
private:

    FmaTransDataObject *transData = nullptr;

    void releasTransaData();

};

#endif // FMACLIPBOARD_H
