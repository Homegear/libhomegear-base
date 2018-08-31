/* Copyright 2013-2017 Sathya Laufer
 *
 * libhomegear-base is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libhomegear-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libhomegear-base.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#include "GZip.h"

#include <array>

namespace BaseLib
{

template<typename DataOut, typename DataIn> DataOut GZip::compress(const DataIn& data, int32_t compressionLevel)
{
    z_stream zStream{};
    zStream.zalloc = Z_NULL;
    zStream.zfree = Z_NULL;
    zStream.opaque = Z_NULL;
    if (deflateInit2(&zStream, compressionLevel, Z_DEFLATED, 0x1F, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        throw GZipException("Error initializing GZip stream.");
    }

    zStream.next_in = (unsigned char*)data.data();
    zStream.avail_in = (unsigned int)data.size();

    DataOut compressedData;
    compressedData.reserve(data.size());
    std::array<uint8_t, 16384> compressedChunk{};

    do
    {
        zStream.avail_out = compressedChunk.size();
        zStream.next_out = compressedChunk.data();
        if(deflate(&zStream, Z_FINISH) == Z_STREAM_ERROR)
        {
            deflateEnd(&zStream);
            throw GZipException("Error during compression.");
        }
        compressedData.insert(compressedData.end(), compressedChunk.begin(), compressedChunk.begin() + (compressedChunk.size() - zStream.avail_out));
    } while(zStream.avail_out == 0);

    if(deflateEnd(&zStream) != Z_OK) throw GZipException("Error during compression finalization.");

    return compressedData;
}

template<typename DataOut, typename DataIn> DataOut GZip::uncompress(const DataIn& data)
{
    z_stream zStream{};
    zStream.zalloc = Z_NULL;
    zStream.zfree = Z_NULL;
    zStream.opaque = Z_NULL;
    zStream.avail_in = 0;
    zStream.next_in = Z_NULL;
    if (inflateInit2(&zStream, 16 + MAX_WBITS) != Z_OK)
    {
        throw GZipException("Error initializing GZip stream.");
    }

    zStream.avail_in = data.size();
    zStream.next_in = (unsigned char*)data.data();

    DataOut uncompressedData;
    uncompressedData.reserve(data.size() * 2);
    std::array<uint8_t, 16384> uncompressedChunk{};
    int result = 0;

    do
    {
        zStream.avail_out = uncompressedChunk.size();
        zStream.next_out = uncompressedChunk.data();
        result = inflate(&zStream, Z_NO_FLUSH);
        switch (result) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&zStream);
                throw GZipException("Error during uncompression.");
        }
        uncompressedData.insert(uncompressedData.end(), uncompressedChunk.begin(), uncompressedChunk.begin() + (uncompressedChunk.size() - zStream.avail_out));
    } while (zStream.avail_out == 0);

    if (inflateEnd(&zStream) != Z_OK) throw GZipException("Error during uncompression finalization.");

    return uncompressedData;
}

}