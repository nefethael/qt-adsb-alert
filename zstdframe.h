#ifndef ZSTDFRAME_H
#define ZSTDFRAME_H

#include <QByteArray>

class ZSTDFrame
{
public:
    ZSTDFrame(QByteArray encryptedBytes);

    QByteArray extract();
private:
    QByteArray m_encryptedBytes;
};

#endif // ZSTDFRAME_H
