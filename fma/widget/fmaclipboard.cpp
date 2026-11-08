#include "fmaclipboard.h"
#include <QApplication>
#include <QMimeData>
#include <QThread>
#include <QSettings>
#include "fma/fmamime.h"

#include <wtsapi32.h>

FmaClipboard FmaClipboard::instance;

int FmaClipboard::format()
{
    QSettings settings(QStringLiteral("LkFormula"), QStringLiteral("LkFormula"));
    return settings.value(QStringLiteral("ClipboardFormat"), 0).toInt();
}

void FmaClipboard::setFormat(int format)
{
    QSettings settings(QStringLiteral("LkFormula"), QStringLiteral("LkFormula"));
    settings.setValue(QStringLiteral("ClipboardFormat"), format);
}

FmaClipboard::FmaClipboard()
{

}

FmaClipboard::~FmaClipboard()
{
    releasTransaData();
}

const QMimeData *FmaClipboard::mimeData(QClipboard::Mode mode)
{

#ifdef _WIN32
    if (mode != QClipboard::Clipboard){
        return nullptr;
    }

    if(transData &&  OleIsCurrentClipboard(transData) == S_OK){
        return transData->mimeData();
    }else{
        return QApplication::clipboard()->mimeData(mode);
    }
#else
    return QApplication::clipboard()->mimeData(mode);
#endif

}

void FmaClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
    if(data){
    #ifdef _WIN32
        if(data->hasFormat(FmaMime::fmaCbor)){
            if (mode != QClipboard::Clipboard){
                return;
            }

            // 同时带文本（MathML/TeX）时交给 Qt 剪贴板：外部应用可读文本，
            // 程序内仍可通过 fmaCbor 粘贴。
            if(data->hasFormat(FmaMime::fmaMml) || !data->text().isEmpty()){
                releasTransaData();
                QApplication::clipboard()->setMimeData(data, mode);
                return;
            }

            if(transData == nullptr){
                transData = new FmaTransDataObject(data);
            }else if(transData->mimeData() != data){
                transData->resetData(data);
            }

            HRESULT hr = OleSetClipboard(transData);
            if (hr != S_OK) {
                releasTransaData();
            }
        }else{
            releasTransaData();
            QApplication::clipboard()->setMimeData(data, mode);
        }
    #else
        QApplication::clipboard()->setMimeData(data, mode);
    #endif
    }
}

void FmaClipboard::releaseClipboard(QClipboard::Mode mode)
{
    if(QApplication::instance() && QApplication::clipboard()
            && QApplication::clipboard()->ownsClipboard()){
        QApplication::clipboard()->clear(mode);
    }
    releasTransaData();
}

void FmaClipboard::releasTransaData()
{
#ifdef _WIN32
    if (transData) {
        if(OleIsCurrentClipboard(transData) == S_OK){
            OleFlushClipboard();
        }

        transData->Release();
        transData = nullptr;
    }
#endif
}
