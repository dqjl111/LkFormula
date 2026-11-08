#include "fmawmf.h"
#include <QDebug>
#include <QMetaEnum>
#include <QIODevice>

// Max Comment Size (max number bytes allowed in an AppsMFCC comment) ...
//   for file types that support only a 16-bit comment size (e.g. WMF)
#define MAX_APPSMFCC_16_CMT_LEN       0x00007FFE
//   for file types that support a 32-bit comment size (e.g. EMF)
#define MAX_APPSMFCC_32_CMT_LEN       0x7FFFFFFE

quint16 FmaWmfRecord::paramU16(quint32 index)
{
    if((index*2+1) < quint32(parameters.size())){
        quint16 sp1;
        quint16 sp2;

        index <<= 1;
        sp1 = parameters[index];
        index++;
        sp2 = parameters[index];

        return ((sp2 << 8) | sp1);
    }else{
        qWarning() << "Invalid param index";
        return 0;
    }
}

QString FmaWmfRecord::metaToString(quint16 meta)
{
    QMetaEnum metaEnm = QMetaEnum::fromType<Meta>();
    return QString(metaEnm.valueToKey(meta)).toLower();
}

/////////////
void FmaWmfExtTextout::fromRecord(std::shared_ptr<FmaWmfRecord> record)
{

    if(record->meta == FmaWmfRecord::EXTTEXTOUT){
        y = record->paramU16(0);
        x = record->paramU16(1);

        quint16 length = record->paramU16(2);
        boxInfo = record->paramU16(3);
        if(boxInfo){
            left = record->paramU16(4);
            top = record->paramU16(5);

            right = record->paramU16(6);
            bottom = record->paramU16(7);

            text = record->parameters.mid(8*2, length);
        }else{
            text = record->parameters.mid(4*2, length);
        }
    }
}

//////////////////////////////////////////////////////

QString FmaWmfEscape::typeToString(quint16 type)
{
    QMetaEnum metaEnm = QMetaEnum::fromType<Type>();
    return QString(metaEnm.valueToKey(type)).toLower();
}

void FmaWmfEscape::fromRecord(std::shared_ptr<FmaWmfRecord> record)
{
    type = record->paramU16(0);
    byteCount = record->paramU16(1);
}

/////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------
// The total size of the chunk (header and data) must be <= Max comment size.

// Max Comment Size (max number bytes allowed in an AppsMFCC comment) ...
//   for file types that support only a 16-bit comment size (e.g. WMF)
//#define MAX_APPSMFCC_16_CMT_LEN       0x00007FFE
//   for file types that support a 32-bit comment size (e.g. EMF)
//#define MAX_APPSMFCC_32_CMT_LEN       0x7FFFFFFE

void FmaWmfEscapeEnhancedMetaFileRecord::fromRecord(std::shared_ptr<FmaWmfRecord> record)
{
    headType = UNKNOWN_HEAD;
    QDataStream is(record->parameters);
    is.setByteOrder(QDataStream::LittleEndian);
    quint16 type, byteCount;
    is >> type >> byteCount;

    if(type != FmaWmfEscape::META_ESCAPE_ENHANCED_METAFILE){
        return;
    }

    QByteArray temp = record->parameters.mid(4, 8);
    if(temp.startsWith("AppsMFCC")){
        if(record->parameters.size() < AppsMFCCHead::streamSize()+4 ){
            return;
        }

        headType = APPSMFCC_HEAD;
        headData.resize(sizeof(AppsMFCCHead));
        AppsMFCCHead* head = (AppsMFCCHead*)headData.data();

        is.readRawData(head->id, sizeof(head->id));
        is >> head->version;
        is >> head->totalBytes;
        is >> head->dataBytes;

        qint8 signByte;

        while(!is.atEnd()){
            is >> signByte;
            appsMfccSignature.append(signByte);
            if(signByte == '\0'){
                break;
            }
        }

        if(!is.atEnd()){
            int readSize = AppsMFCCHead::streamSize() + 4 + appsMfccSignature.size();

            if(record->parameters.size() >= int(readSize + head->dataBytes)){
                metafileData.resize(head->dataBytes);
                if(head->dataBytes){
                    is.readRawData(metafileData.data(), head->dataBytes);
                }
            }else{
                qWarning() << "Invalid EscapeEnhancedMetaFileRecord AppsMFCCHead size!";
            }
        }
    }
    else if(temp.startsWith("MathType")){
        if(record->parameters.size() < MTEFHead::streamSize() +4){
            return;
        }

        headType = MTEF_HEAD;
        headData.resize(sizeof(MTEFHead));
        MTEFHead* head = (MTEFHead*)headData.data();

        is.readRawData(head->id, sizeof(head->id));
        is >> head->magic;
        is >> head->dataBytes;

        if(!is.atEnd()){
            int readSize = MTEFHead::streamSize() + 4;

            if(record->parameters.size() >= int(readSize + head->dataBytes)){
                metafileData.resize(head->dataBytes);
                if(head->dataBytes){
                    is.readRawData(metafileData.data(), head->dataBytes);
                }
            }else{
                qWarning() << "Invalid EscapeEnhancedMetaFileRecord MTEFHead size!";
            }
        }
    }
    else{
        quint32 commentIdentifier;
        is >> commentIdentifier;

        if(commentIdentifier == emfEscapeKey){

            headType = EMF_HEAD;
            headData.resize(sizeof(EmfHead));
            EmfHead* head = (EmfHead*)headData.data();
            head->commentIdentifier = commentIdentifier;

            is >> head->commentType
                >> head->version
                >> head->checksum
                >> head->flags
                >> head->commentRecordCount
                >> head->currentRecordSize
                >> head->remainingBytes
                >> head->enhancedMetafileDataSize;

            if(!is.atEnd()){
                int readSize = EmfHead::streamSize() + 4;

                if(record->parameters.size() >= int(readSize + head->enhancedMetafileDataSize)){
                    metafileData.resize(head->enhancedMetafileDataSize);
                    if(head->enhancedMetafileDataSize){
                        is.readRawData(metafileData.data(), head->enhancedMetafileDataSize);
                    }
                }else{
                    qWarning() << "Invalid EscapeEnhancedMetaFileRecord EmfHead size!";
                }
            }
        }
    }
}

FmaWmfEscapeEnhancedMetaFileRecord::EmfHead *FmaWmfEscapeEnhancedMetaFileRecord::emfHead()
{
    if(headType == EMF_HEAD){
        return (EmfHead*)headData.data();
    }
    return nullptr;
}

FmaWmfEscapeEnhancedMetaFileRecord::AppsMFCCHead *FmaWmfEscapeEnhancedMetaFileRecord::appsMfccHead()
{
    if(headType == APPSMFCC_HEAD){
        return (AppsMFCCHead*)headData.data();
    }
    return nullptr;
}

FmaWmfEscapeEnhancedMetaFileRecord::MTEFHead *FmaWmfEscapeEnhancedMetaFileRecord::mtefHead()
{
    if(headType == MTEF_HEAD){
        return (MTEFHead*)headData.data();
    }
    return nullptr;
}

QList<std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> >
FmaWmfEscapeEnhancedMetaFileRecord::buildFromMml(QString mmlData)
{
    if(!mmlData.startsWith("<?xml")){
        mmlData.prepend("<?xml version=\"1.0\"?>");
        //mmlData.prepend("<?xml version=\"1.0\"?><!-- MathType@Translator@5@5@MathML2 (Clipboard).tdl@MathML 2.0 (Clipboard)@ -->");
        //mmlData.append("<!-- MathType@End@5@5@ -->");
    }

    QList<std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> > ret;
    QByteArray data = mmlData.toUtf8();

    if(!data.isEmpty()){
        int i=0;
        bool needBreak = false;
        while(true){
            std::shared_ptr<FmaWmfEscapeEnhancedMetaFileRecord> mmlRecord =
                    std::make_shared<FmaWmfEscapeEnhancedMetaFileRecord>();

            mmlRecord->headType = APPSMFCC_HEAD;
            mmlRecord->headData.resize(sizeof(AppsMFCCHead));
            AppsMFCCHead* head = (AppsMFCCHead*)mmlRecord->headData.data();
            memcpy(head->id, "AppsMFCC",8);
            head->version = 1;
            head->totalBytes = data.size();

            if((i+1)*MAX_APPSMFCC_16_CMT_LEN < data.size() ){
                mmlRecord->metafileData = data.mid(i*MAX_APPSMFCC_16_CMT_LEN, MAX_APPSMFCC_16_CMT_LEN);
            }else{
                mmlRecord->metafileData = data.mid(i*MAX_APPSMFCC_16_CMT_LEN);
                needBreak = true;
            }
            mmlRecord->appsMfccSignature = "Design Science, Inc.";
            mmlRecord->appsMfccSignature.append(char(0));
            head->dataBytes = mmlRecord->metafileData.size();
            ret.append(mmlRecord);
            if(needBreak){
                break;
            }
        }
    }

    return ret;
}

QByteArray FmaWmfEscapeEnhancedMetaFileRecord::toParameters()
{
    QByteArray ret;
    QDataStream os(&ret, QIODevice::WriteOnly);
    os.setByteOrder(QDataStream::LittleEndian);

    os << quint16(FmaWmfEscape::META_ESCAPE_ENHANCED_METAFILE);

    switch(headType){
    case EMF_HEAD:{
        if(headData.size() != sizeof(EmfHead)){
            os << quint16(0);
            break;
        }

        EmfHead* head = (EmfHead*)headData.data();
        if(int(head->enhancedMetafileDataSize) != metafileData.size()){
            head->enhancedMetafileDataSize = metafileData.size();
        }
        os << quint16(EmfHead::streamSize() + head->enhancedMetafileDataSize);
        os << head->commentIdentifier << head->commentType
           << head->version << head->checksum << head->flags
           << head->commentRecordCount << head->currentRecordSize
           << head->remainingBytes << head->enhancedMetafileDataSize;

        os.writeRawData(metafileData.data(), head->enhancedMetafileDataSize);
        break;
    }
    case APPSMFCC_HEAD:{
        if(headData.size() != sizeof(AppsMFCCHead)){
            os << quint16(0);
            break;
        }

        AppsMFCCHead* head = (AppsMFCCHead*)headData.data();
        if(int(head->dataBytes) != metafileData.size()){
            head->dataBytes = metafileData.size();
        }
        os << quint16(AppsMFCCHead::streamSize() + head->dataBytes + appsMfccSignature.size());
        os.writeRawData(head->id,8);
        os << head->version << head->totalBytes << head->dataBytes;
        os.writeRawData(appsMfccSignature.data(), appsMfccSignature.size());
        os.writeRawData(metafileData.data(), head->dataBytes);
        break;
    }
    case MTEF_HEAD:{
        if(headData.size() != sizeof(MTEFHead)){
            os << quint16(0);
            break;
        }

        MTEFHead* head = (MTEFHead*)headData.data();
        if(int(head->dataBytes) != metafileData.size()){
            head->dataBytes = metafileData.size();
        }
        os << quint16(MTEFHead::streamSize() + head->dataBytes);
        os.writeRawData(head->id,8);
        os << head->magic << head->dataBytes;
        os.writeRawData(metafileData.data(), head->dataBytes);
        break;
    }
    default:{
        os << quint16(0);
        break;
    }
    }
    if(ret.size()%2){
        ret.append(char(0));
    }
    return ret;
}


//////////////////////////////////////////////////////
FmaWmfHelper::FmaWmfHelper()
{

}

bool FmaWmfHelper::readWmf(QDataStream& wmfStream)
{
    records.clear();
    wmfStream.setByteOrder(QDataStream::LittleEndian);

    if(readHead(wmfStream)){
        readRecords(wmfStream);
        return true;
    }
    return false;
}

bool FmaWmfHelper::saveWmf(QDataStream &wmfStream,bool withPlaceHeader)
{
    wmfStream.setByteOrder(QDataStream::LittleEndian);
    if(writeHead(wmfStream, withPlaceHeader)){
        writeRecords(wmfStream);
        return true;
    }
    return false;
}

QByteArray FmaWmfHelper::fetchMmlData()
{
    qint32 currentRecordSize = 0;
    QByteArray sign;
    QByteArray ret;
    for(std::shared_ptr<FmaWmfRecord> record: records){
        if(record->meta == FmaWmfRecord::ESCAPE){
            FmaWmfEscape escape;
            escape.fromRecord(record);

            if(escape.type == FmaWmfEscape::META_ESCAPE_ENHANCED_METAFILE){
                FmaWmfEscapeEnhancedMetaFileRecord metaRecord;
                metaRecord.fromRecord(record);

                FmaWmfEscapeEnhancedMetaFileRecord::AppsMFCCHead* mfccHead = metaRecord.appsMfccHead();

                if(mfccHead != nullptr){
                    if(metaRecord.metafileData.startsWith("<?xml")){
                        if(mfccHead->totalBytes <= mfccHead->dataBytes){
                            return metaRecord.metafileData;
                        }else{
                            sign = metaRecord.appsMfccSignature;
                            currentRecordSize = mfccHead->totalBytes;
                            ret.append(metaRecord.metafileData);
                        }
                    }else{
                        if(currentRecordSize >0){
                            if(metaRecord.appsMfccSignature == sign){
                                ret.append(metaRecord.metafileData);
                                if(ret.size() >= currentRecordSize){
                                    return ret;
                                }
                            }
                        }
                    }

                }
            }else{

            }
        }
    }

    return ret;
}


void FmaWmfHelper::printRecords()
{
    qWarning() << "key:" << placeHead.key << ", handle:" << placeHead.handle;
    qWarning() << "left:" << placeHead.left << ", top:" << placeHead.top;
    qWarning() << "right:" << placeHead.right << ", bottom:" << placeHead.bottom;
    qWarning() << "inch:" << placeHead.inch << ", reserved:" << placeHead.reserved;
    qWarning() << "checksum:" << placeHead.checksum;

    qWarning() << "fileType:" << wmfHead.fileType << ", headerSize:" << wmfHead.headerSize;
    qWarning() << "version:" << wmfHead.version << ", fileSize:" << wmfHead.fileSize;
    qWarning() << "numOfObjects:" << wmfHead.numOfObjects << ", maxRecordSize:" << wmfHead.maxRecordSize;
    qWarning() << "numOfParams:" << wmfHead.numOfParams;

    QString info;
    for(std::shared_ptr<FmaWmfRecord> record: records){
        info = QString::number(record->meta)
                + ":" + FmaWmfRecord::metaToString(record->meta)
                + ":" + QString::number(record->parameters.size());

        if(record->meta == FmaWmfRecord::EXTTEXTOUT){
            qWarning() << info;

            QDataStream is(record->parameters);
            is.setByteOrder(QDataStream::LittleEndian);
            quint16 y, x, len, fwOpt;
            is >> y >> x >> len >> fwOpt;
            qWarning() << "y: " << y << ", x: " << x << ", len:" << len;
            qWarning() << "fwOpt: " << fwOpt;
            if(fwOpt & quint16(FmaWmfTextOpt::ETO_OPAQUE)
                    || fwOpt & quint16(FmaWmfTextOpt::ETO_CLIPPED)){
                quint16 left, top, right, bottom;
                is >> left >> top >> right >> bottom;
                qWarning() << "left: " << left << ", top: " << top
                           << ", right:" << right << ", bottom: " << bottom;
            }
            QByteArray strData(int(len), char(0));
            is.readRawData(strData.data(), len);
            qWarning()  << QString::fromUtf8(strData);
        }else{
            qWarning() << info << ":" + record->parameters;
        }
    }
}

void FmaWmfHelper::buildHeaderAsRecord(QSize size, quint16 objNum, quint32 maxRecord, qint16 logicalPerInch)
{
    placeHead.key = wmfKey;
    placeHead.handle = 0;
    placeHead.left = 0;
    placeHead.top = 0;
    placeHead.right = size.width();
    placeHead.bottom = size.height();
    placeHead.inch = logicalPerInch;
    placeHead.reserved = 0;
    placeHead.checksum = quint16(wmfKey&0xFFFF)
            ^ quint16(wmfKey >> 16)
            ^ placeHead.right
            ^ placeHead.bottom
            ^ placeHead.inch;

    wmfHead.fileType = 1;
    wmfHead.headerSize = 9;
    wmfHead.version = 768; //0x300
    wmfHead.fileSize = metaSize();
    wmfHead.numOfObjects = objNum;
    wmfHead.maxRecordSize = maxRecord;
    wmfHead.numOfParams = 0;
}

bool FmaWmfHelper::readHead(QDataStream &wmfStream)
{
    memset(((char*)&placeHead),0, sizeof(placeHead));
    memset(((char*)&wmfHead),0, sizeof(wmfHead));

    wmfStream >> placeHead.key;
    if(placeHead.key == wmfKey){
        wmfStream >> placeHead.handle
                >> placeHead.left
                >> placeHead.top
                >> placeHead.right
                >> placeHead.bottom
                >> placeHead.inch
                >> placeHead.reserved
                >> placeHead.checksum;

        wmfStream >> wmfHead.fileType;
        wmfStream >> wmfHead.headerSize;
    }else{
        wmfHead.fileType = quint16(placeHead.key & 0xFFFF);
        wmfHead.headerSize = quint16(placeHead.key >> 16);
    }

    if(wmfStream.atEnd()){
        return false;
    }

    if (wmfHead.headerSize == 9)
    {
        wmfStream >> wmfHead.version
                >> wmfHead.fileSize
                >> wmfHead.numOfObjects
                >> wmfHead.maxRecordSize
                >> wmfHead.numOfParams;
    }

    return !wmfStream.atEnd();
}

void FmaWmfHelper::readRecords(QDataStream &wmfStream)
{
    while(!wmfStream.atEnd()){
        std::shared_ptr<FmaWmfRecord> record = std::make_shared<FmaWmfRecord>();
        wmfStream >> record->size;
        wmfStream >> record->meta;

        if ((record->size == 3) && (record->meta == 0))
        {
            records.append(record);
            break; /* Probably final record ?? */
        }

        if(((record->size - 3) > wmfHead.maxRecordSize) || (record->size < 3)){
            qWarning() << "Bad wmf record size: " << record->size;
            break;
        }

        int paramLen = (record->size - 3) * 2;
        record->parameters.resize(paramLen);
        if(wmfStream.readRawData(record->parameters.data(), paramLen) == paramLen){
            records.append(record);
        }
    }
}

bool FmaWmfHelper::writeHead(QDataStream &wmfStream, bool withPlaceHeader)
{
    if(placeHead.key == wmfKey && withPlaceHeader){
        wmfStream << placeHead.key
                << placeHead.handle
                << placeHead.left
                << placeHead.top
                << placeHead.right
                << placeHead.bottom
                << placeHead.inch
                << placeHead.reserved
                << placeHead.checksum;

        wmfStream << wmfHead.fileType
                << wmfHead.headerSize
                << wmfHead.version
                << wmfHead.fileSize
                << wmfHead.numOfObjects
                << wmfHead.maxRecordSize
                << wmfHead.numOfParams;
    }else{
        wmfStream << wmfHead.fileType
                << wmfHead.headerSize
                << wmfHead.version
                << wmfHead.fileSize
                << wmfHead.numOfObjects
                << wmfHead.maxRecordSize
                << wmfHead.numOfParams;
    }
    return true;
}

void FmaWmfHelper::writeRecords(QDataStream &wmfStream)
{
    for(std::shared_ptr<FmaWmfRecord> record : records){
        int paramLen = (record->size - 3) * 2;
        if(paramLen != record->parameters.size()){
            paramLen = record->parameters.size();
            record->size = paramLen/2 + 3;
        }

        wmfStream << record->size;
        wmfStream << record->meta;

        wmfStream.writeRawData(record->parameters.data(), paramLen);
    }
}

quint32 FmaWmfHelper::metaSize()
{
    quint32 bytes = 18;
    for(std::shared_ptr<FmaWmfRecord> record : records){
        bytes += 6 + record->parameters.size();
    }
    return bytes/2;
}






