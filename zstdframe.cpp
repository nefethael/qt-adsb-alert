#include "zstdframe.h"

#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#include <QDebug>

static uint32_t read32(unsigned char * buffer){
    return (uint32_t)buffer[0] << 24 |
          (uint32_t)buffer[1] << 16 |
          (uint32_t)buffer[2] << 8  |
          (uint32_t)buffer[3];
}

static void write32(unsigned char * buffer, uint32_t n){
    buffer[0] = (n >> 24) & 0xFF;
    buffer[1] = (n >> 16) & 0xFF;
    buffer[2] = (n >> 8) & 0xFF;
    buffer[3] = n & 0xFF;
}

ZSTDFrame::ZSTDFrame(QByteArray encryptedBytes) : m_encryptedBytes(encryptedBytes)
{

}

QByteArray ZSTDFrame::extract()
{
    QByteArray decrypt(m_encryptedBytes);

    /*
    uint32_t key = read32((unsigned char*)&m_encryptedBytes.data()[0]);
    write32((unsigned char*)decrypt.data(), 0x28b52ffd);

    for(auto i = 4; i < decrypt.size() - 4; i += 4){
        auto tmp = read32((unsigned char*)&m_encryptedBytes.data()[i]);
        tmp = (tmp ^ key) ^ 0x5132c744;
        write32((unsigned char*)&(decrypt.data()[i]), tmp);
    }
    */

    auto newSize = ZSTD_findDecompressedSize(decrypt.data(), decrypt.size());

    if (newSize == 0xffffffff){
        qCritical() << "Problem with decrypting";
        return QByteArray{};
    }

    QByteArray finalBytes(newSize, 0);
    ZSTD_decompress(finalBytes.data(), newSize, decrypt.data(), decrypt.size());

    return finalBytes;
}
