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

#pragma once
#include <U2Core/IOAdapter.h>

namespace U2 {

struct LZMASupport;

class U2CORE_EXPORT LZMAAdapter: public IOAdapter {
    Q_OBJECT
public:
    static const QString LZMA_URL_PREFIX;
    static const QString LZMA_URL_SEP;
    static const QString LZMA_FILE_EXT;

    LZMAAdapter(IOAdapterFactory* iof);
    ~LZMAAdapter();

    virtual bool open(const GUrl& url, IOAdapterMode m_ );

    virtual bool isOpen() const;

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);

    virtual qint64 left() const;

    virtual int getProgress() const;

    virtual qint64 bytesRead() const;

    virtual GUrl getURL() const;

    virtual QString errorString() const;

    QStringList getArchivedFileNames();
    QStringList getArchivedFileURLs();

private:
    LZMASupport *s;
    GUrl url;
};

}//namespace

