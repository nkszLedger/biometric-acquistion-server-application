#ifndef ENCRYPTO_H
#define ENCRYPTO_H

#include <QFile>
#include <QDebug>
#include <QObject>

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#define IVSIZE 32
#define KEYSIZE 32
#define SALTSIZE 8
#define BLOCKSIZE 256
#define PADDING RSA_PKCS1_PADDING


class Encrypto
{
public:
  /*!
   * \brief Encrypto
   */
  Encrypto();

  ~Encrypto();


  /*!
   * \brief loads public key from byte array (GET -> PUBLIC RSA KEY)
   * \param data
   * \return
   */
  RSA *getPublicKey(QByteArray &data);

  /*!
   * \brief loads public key from file (GET -> PUBLIC RSA KEY)
   * \param fileName
   * \return
   */
  RSA *getPublicKey(QString fileName);

  /*!
   * \brief loads private key from byte array (GET -> PRIVATE RSA KEY)
   * \param data
   * \return
   */
  RSA *getPrivateKey(QByteArray &data);

  /*!
   * \brief loads private key from file (GET -> PRIVATE RSA KEY)
   * \param fileName
   * \return
   */
  RSA *getPrivateKey(QString fileName);

  /*!
   * \brief encrypt a byteArray using the RSA public key
   * \param key - the public key
   * \param data - the data to encrypt
   * \return
   */
  QByteArray encryptRSA(RSA *key, QByteArray &data);

  /*!
   * \brief decrypt a byte array using the RSA private key
   * \param key - the private key
   * \param data - the data to decrypt
   * \return
   */
  QByteArray decryptRSA(RSA *key, QByteArray &data);

  /*!
   * \brief encryptAES encrypt a byte array with AES 256 CBC
   * \param passphrase
   * \param data
   * \param addRandomSalt
   * \return
   */
  QByteArray encryptAES(QByteArray passphrase, QByteArray &data, bool addRandomSalt);

  /*!
   * \brief decrypt a byte array with AES 256 CBC
   * \param passphrase
   * \param data
   * \return
   */
  QByteArray decryptAES(QByteArray passphrase, QByteArray &data);

  /*!
   * \brief generate random bytes (OPENSSL) (SALT)
   * \param size
   * \return
   */
  QByteArray randomBytes(int size);

 /*!
   * \brief frees an RSA Key from memory
   * \param key
   */
  void freeRSAKey(RSA *key);

  /*!
   * \brief readFile
   * \param fileName
   * \return
   */
  QByteArray readFile(QString fileName);

  /*!
   * \brief writeFile
   * \param fileName
   * \param data
   */
  void writeFile(QString fileName, QByteArray &data);

private:

  /*!
   * \brief initialiser
   */
  void initialiseOpenSSL();

  /*!
   * \brief finaliser
   */
  void finaliserOpenSSL();


};

#endif // ENCRYPTO_H
