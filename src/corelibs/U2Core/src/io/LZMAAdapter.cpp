/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <qendian.h>

#include "LZMAAdapter.h"

#include <U2Core/Log.h>

#include <3rdparty/7z/7zSDK.h>

#include <assert.h>

namespace U2 {

const QString LZMAAdapter::LZMA_URL_PREFIX("7z:");
const QString LZMAAdapter::LZMA_URL_SEP("!/");
const QString LZMAAdapter::LZMA_FILE_EXT(".7z");


struct LZMASupport {
    LZMASupport(): res(-1), entry_idx(UINT_MAX), outBuffer(NULL), outBufferProcessed(NULL) {}
    void destroy();

    bool init(const char* fname, const char* entry);

    ISzAlloc allocImp;
    ISzAlloc allocTempImp;

    CFileInStream archiveStream;
    CLookToRead2 lookStream;
    CSzArEx db;
    SRes res;

    UInt32 entry_idx;

    /*
    if you need cache, use these 3 variables.
    */
    UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
    Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
    size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

    size_t offset = 0;
    size_t outSizeProcessed = 0;

    Byte *outBufferProcessed = 0;
};

LZMAAdapter::LZMAAdapter(IOAdapterFactory* iof)
: IOAdapter(iof), s(NULL) {}

LZMAAdapter::~LZMAAdapter() {
    close();
}

void LZMAAdapter::close() {
    delete s;
    s = NULL;
}
#define kInputBufSize ((size_t)1 << 18)

static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

QStringList LZMAAdapter::getArchivedFileURLs() {
    QStringList res;
    if (url.getURLString().endsWith(LZMA_FILE_EXT)) {
        QStringList l = getArchivedFileNames();
        foreach (QString s, l) {
            QString su = LZMA_URL_PREFIX + url.getURLString() + LZMA_URL_SEP + s;
            res << su;
        }
    }
    else {
        res << url.getURLString();
    }
    return res;
}

QStringList LZMAAdapter::getArchivedFileNames()
{
    assert(!isOpen());
    QStringList lst;
    if (isOpen()) {
        UInt16 *temp = NULL;
        size_t tempSize = 0;

        for (UInt32 i = 0; i < s->db.NumFiles; i++)
       {
           // const CSzFileItem *f = db.Files + i;
           size_t len;
           unsigned isDir = SzArEx_IsDir(&s->db, i);
           if (isDir)
               continue;
           len = SzArEx_GetFileNameUtf16(&s->db, i, NULL);
           // len = SzArEx_GetFullNameLen(&db, i);

           if (len > tempSize)
           {
               SzFree(NULL, temp);
               tempSize = len;
               temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
               if (!temp)
               {
                   s->res = SZ_ERROR_MEM;
                   break;
               }
           }

           SzArEx_GetFileNameUtf16(&s->db, i, temp);
           QString name = QString::fromUtf16(temp);
           lst << name;

       }
        SzFree(NULL, temp);
    }
    return lst;
}

bool LZMAAdapter::isOpen() const {
    return s && s->res == SZ_OK;
}

bool LZMAAdapter::open(const GUrl& url, IOAdapterMode m ) {
    assert(!isOpen());
    close();
    if (m == IOAdapterMode_Read) {
        this->url = url;
        s = new LZMASupport;
        return s->init(url.getURLString().toLocal8Bit().constData(), NULL);
//        if (url.getURLString().startsWith(LZMA_URL_PREFIX)) {

//        }
//        const char* sep = strchr(filename, '!');
//        if (sep && sep[1] == '/') {
//            string aname(filename, sep - filename);
//            return init(aname.c_str(), sep + 2);
//        }
//        else {
//            cerr << "Malformed archive entry url: " << filename << "\n";
//            return false;
//        }
    }
    else {
        return false;
    }
}

qint64 LZMAAdapter::readBlock(char* data, qint64 size)
{
    if (!isOpen()) {
        assert(0 && "not ready to read");
        return false;
    }
//    // first use data put back to buffer if any
//    int cached = 0;
//    if (rewinded != 0) {
//        assert(rewinded > 0 && rewinded <= buf->length());
//        cached = buf->read(data, size, buf->length() - rewinded);
//        if (cached == size) {
//            rewinded -= size;
//            return size;
//        }
//        assert(cached < size);
//        rewinded = 0;
//    }
//    size = z->uncompress(data + cached, size - cached);
//    if (size == -1) {
//        return -1;
//    }
//    buf->append(data + cached, size);

//    return size + cached;
    return -1;
}

qint64 LZMAAdapter::writeBlock(const char* data, qint64 size) {
    if (!isOpen()) {
        assert(0 && "not ready to write");
        return false;
    }
    //qint64 l = z->compress(data, size);
    return 0;
}

bool LZMAAdapter::skip(qint64 nBytes) {
    if (!isOpen()) {
        assert(0 && "not ready to seek");
        return false;
    }
//    assert(buf);
//    nBytes -= rewinded;
//    if (nBytes <= 0) {
//        if (-nBytes <= buf->length()) {
//            rewinded = -nBytes;
//            return true;
//        }
//        return false;
//    }
//    rewinded = 0;
//    char* tmp = new char[nBytes];
//    qint64 skipped = readBlock(tmp, nBytes);
//    delete[] tmp;

//    return skipped == nBytes;
    return false;
}

qint64 LZMAAdapter::left() const
{
    return 0;
}

int LZMAAdapter::getProgress() const
{
    return 0;
}

qint64 LZMAAdapter::bytesRead() const
{
    return 0;
}

GUrl LZMAAdapter::getURL() const {
    return url;
}

QString LZMAAdapter::errorString() const{
    return s ? QString::number(s->res) : QString();
}

//static int strcmp16(const char *p, const UInt16 *name)
//{
//    CBuf buf;
//    int res = 1;
//    Buf_Init(&buf);
//    if (Utf16_To_Char(&buf, name) == SZ_OK) {
//        //fprintf(stderr, "Found entry: %s", (const char *)buf.data);
//        res = strcmp(p, (const char *)buf.data);
//        Buf_Free(&buf, &g_Alloc);
//    }
//    return res;
//}

bool LZMASupport::init(const char* fname, const char* entry) {
    allocImp = g_Alloc;
    allocTempImp = g_Alloc;

    if (InFile_Open(&archiveStream.file, fname))
    {
        fprintf(stderr, "can not open 7z file: %s\n", fname);
        return false;
    }

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf = NULL;

    res = SZ_OK;

    {
        lookStream.buf = (Byte*)ISzAlloc_Alloc(&allocImp, kInputBufSize);
        if (!lookStream.buf)
            res = SZ_ERROR_MEM;
        else
        {
            lookStream.bufSize = kInputBufSize;
            lookStream.realStream = &archiveStream.vt;
            LookToRead2_Init(&lookStream);
        }
    }

    CrcGenerateTable();

    SzArEx_Init(&db);

    if (res == SZ_OK)
    {
        UInt16 *temp = NULL;
        size_t tempSize = 0;

        res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);

        if (entry) {

//             for (UInt32 i = 0; i < db.NumFiles; i++)
//            {
//                // const CSzFileItem *f = db.Files + i;
//                size_t len;
//                unsigned isDir = SzArEx_IsDir(&db, i);
//                if (isDir)
//                    continue;
//                len = SzArEx_GetFileNameUtf16(&db, i, NULL);
//                // len = SzArEx_GetFullNameLen(&db, i);

//                if (len > tempSize)
//                {
//                    SzFree(NULL, temp);
//                    tempSize = len;
//                    temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
//                    if (!temp)
//                    {
//                        res = SZ_ERROR_MEM;
//                        break;
//                    }
//                }

//                SzArEx_GetFileNameUtf16(&db, i, temp);
//                if (0 == strcmp16(entry, temp)) {
//                    entry_idx = i;
//                    break;
//                }

//                /*
//                if (SzArEx_GetFullNameUtf16_Back(&db, i, temp + len) != temp)
//                {
//                res = SZ_ERROR_FAIL;
//                break;
//                }
//                */
//            }
             SzFree(NULL, temp);
        }
    }

    if (res == SZ_OK)
    {
//		if (entry_idx == UINT32_MAX) {
//			fprintf(stderr, "Bad target configuration, 7z archive entry not found: %s", entry);
//			res = SZ_ERROR_DATA;
//		}
//		else
        {
            return true;
        }
    }

    if (res == SZ_ERROR_UNSUPPORTED)
        fprintf(stderr, "decoder doesn't support this archive\n");
    else if (res == SZ_ERROR_MEM)
        fprintf(stderr, "can not allocate memory\n");
    else if (res == SZ_ERROR_CRC)
        fprintf(stderr, "CRC error\n");
    else
    {
        char s[32];
        //UInt64ToStr(res, s, 0);
        //fprintf(stderr, "Error decoding 7z archive: %s\n", s);
    }
    return false;
}

void LZMASupport::destroy() {
    ISzAlloc_Free(&allocImp, outBuffer);
    SzArEx_Free(&db, &allocImp);
    ISzAlloc_Free(&allocImp, lookStream.buf);
    File_Close(&archiveStream.file);
}

};//namespace
