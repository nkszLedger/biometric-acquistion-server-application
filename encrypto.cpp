#include "encrypto.h"

Encrypto::Encrypto()
{
  // initialise OpenSSl
  initialiseOpenSSL();
}

Encrypto::~Encrypto()
{
  // free memory and delete instance
  finaliserOpenSSL();
}

RSA *Encrypto::getPublicKey(QByteArray &data)
{
  // load data into char array
  const char* public_key_str = data.constData();

  // static cast into BIO
  BIO *bio = BIO_new_mem_buf((void *)public_key_str, -1);

  // set flags for BIO
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  // read BIO key into RSA Object
  RSA* rsa_pub_key = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);

  if(!rsa_pub_key)
  {
    // - Error
    qDebug() << "Could not load public key" << ERR_error_string(ERR_get_error(), NULL);
  }

  // free object
  BIO_free(bio);

  // return RSA Public Key
  return rsa_pub_key;
}

RSA *Encrypto::getPublicKey(QString fileName)
{
  QByteArray data = readFile(fileName);

  return getPublicKey(data);
}

RSA *Encrypto::getPrivateKey(QByteArray &data)
{
  const char* private_key_str = data.constData();
  BIO *bio = BIO_new_mem_buf((void *)private_key_str, -1);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  RSA* rsa_private_key = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
  if(!rsa_private_key)
  {
    qDebug() << "Could not load private key" << ERR_error_string(ERR_get_error(), NULL);
  }

  // free object
  BIO_free(bio);

  // return RSA Public Key
  return rsa_private_key;
}

RSA *Encrypto::getPrivateKey(QString fileName)
{
  QByteArray data = readFile(fileName);

  return getPrivateKey(data);
}

QByteArray Encrypto::encryptRSA(RSA *key, QByteArray &data)
{
  QByteArray buffer;
  int data_size = data.length();

  //
  const unsigned char* str = (const unsigned char*)data.constData();

  // get length of key
  int rsa_len = RSA_size(key);

  // allocate memory based on rsa key length
  unsigned char* ed = (unsigned char*)malloc(rsa_len);

  // encrypt using public key
  int result_len = RSA_public_encrypt(data_size, (const unsigned char*)str, ed, key, PADDING);

  if(result_len == -1)
  {
    qDebug() << "Could not encrypt";
    return buffer;
  }

  buffer = QByteArray(reinterpret_cast<char*>(ed), result_len);

  return buffer;
}

QByteArray Encrypto::decryptRSA(RSA *key, QByteArray &data)
{
  QByteArray buffer;
  const unsigned char* encrypted_data = (const unsigned char*)data.constData();

  //int data_size = data.length();
  //const unsigned char* str = (const unsigned char*)data.constData();

  // get length of key
  int rsa_len = RSA_size(key);

  // allocate memory based on rsa key length
  unsigned char* ed = (unsigned char*)malloc(rsa_len);

  // encrypt using public key
  int result_len = RSA_private_decrypt(rsa_len, encrypted_data, ed, key, PADDING);

  if(result_len == -1)
  {
    qDebug() << "Could not decrypt";
    return buffer;
  }

  //buffer = QByteArray(reinterpret_cast<char*>(ed), result_len);
  buffer = QByteArray::fromRawData((const char*)ed, result_len);
  return buffer;
}

QByteArray Encrypto::encryptAES(QByteArray passphrase, QByteArray &data, bool addRandomSalt)
{
  // openSSL command default is ONE
  int rounds = 1;

  QByteArray m_salt;
  m_salt.clear();
  // generate SALT
  if(addRandomSalt)
    m_salt = randomBytes(SALTSIZE);
  else
    m_salt.append("??K39??u");

  unsigned char key[KEYSIZE];
  unsigned char init_vector[IVSIZE];

  //
  const unsigned char* salt     = (const unsigned char*)m_salt.constData();
  const unsigned char* password = (const unsigned char*)passphrase.constData();

  // converts passphrase into secure key
  int i = EVP_BytesToKey(EVP_aes_256_cbc(),\
                         EVP_sha1(),\
                         salt,\
                         password,\
                         passphrase.length(),\
                         rounds,\
                         key,\
                         init_vector);

  if(i != KEYSIZE)
  {
    // - error
    return QByteArray();
  }

  // cipher contect
  EVP_CIPHER_CTX en;
  // init cipher
  EVP_CIPHER_CTX_init(&en);

  if(!EVP_EncryptInit_ex(&en, EVP_aes_256_cbc(), NULL, key, init_vector))
  {
    // - error
    return QByteArray();
  }

  //
  char *input = data.data();
  char *out;
  int len = data.size();

  int f_len = 0;
  int c_len = len + AES_BLOCK_SIZE;

  unsigned char* cipher_text = (unsigned char*)malloc(c_len);

  if(!EVP_EncryptInit_ex(&en, NULL, NULL, NULL, NULL))
  {
    // - error
    return QByteArray();
  }

  // May have to repeat this for large files
  // #cipher chaining

  // encrypt data
  if(!EVP_EncryptUpdate(&en,\
                        cipher_text,\
                        &c_len,\
                        (unsigned char*)input,\
                        len))
  {
    // - error
    return QByteArray();
  }

  if(!EVP_EncryptFinal(&en,\
                       cipher_text+c_len,\
                       &f_len))
  {
    // - error
    return QByteArray();
  }

  len = c_len + f_len;
  out = (char*)cipher_text;
  EVP_CIPHER_CTX_cipher(&en);



  QByteArray encrypted = QByteArray(reinterpret_cast<char*>(cipher_text), len);
  QByteArray finished;
  finished.append("Salted__");
  finished.append(m_salt);
  finished.append(encrypted);

  free(cipher_text);

  return finished;
}

QByteArray Encrypto::decryptAES(QByteArray passphrase, QByteArray &data)
{
  QByteArray m_salt;

  if(QString(data.mid(0,8)) == "Salted__")
  {
    // - load salt from the data
    m_salt = data.mid(8, 8);
    data = data.mid(16);
    qDebug() << "SALT: " << m_salt;
  }
  else
  {
    qDebug() << "FAILED to get SALT";
    m_salt = randomBytes(SALTSIZE);
  }

  // openSSL command default is ONE
  int rounds = 1;
  unsigned char key[KEYSIZE];
  unsigned char init_vector[IVSIZE];

  //
  const unsigned char* salt     = (const unsigned char*)m_salt.constData();
  const unsigned char* password = (const unsigned char*)passphrase.constData();

  // converts passphrase into secure key
  int i = EVP_BytesToKey(EVP_aes_256_cbc(),\
                         EVP_sha1(),\
                         salt,\
                         password,\
                         passphrase.length(),\
                         rounds,\
                         key,\
                         init_vector);

  if(i != KEYSIZE)
  {
    // - error
    return QByteArray();
  }

  // cipher contect
  EVP_CIPHER_CTX decryption;
  // init cipher
  EVP_CIPHER_CTX_init(&decryption);

  if(!EVP_DecryptInit_ex(&decryption, EVP_aes_256_cbc(), NULL, key, init_vector))
  {
    // - error
    return QByteArray();
  }

  //
  char *input = data.data();
  int len = data.size();

  int p_len = len;
  int f_len = 0;

  unsigned char* plain_text = (unsigned char*)malloc(p_len + AES_BLOCK_SIZE);

  // May have to repeat this for large files
  // #cipher chaining

  // encrypt data
  if(!EVP_DecryptUpdate(&decryption,\
                        plain_text,\
                        &p_len,\
                        (unsigned char*)input,\
                        len))
  {
    // - error
    return QByteArray();
  }

  if(!EVP_DecryptFinal_ex(&decryption,\
                          plain_text+p_len,\
                          &f_len))
  {
    // - error
    return QByteArray();
  }

  len = p_len + f_len;
  EVP_CIPHER_CTX_cleanup(&decryption);


  QByteArray decrypted = QByteArray(reinterpret_cast<char*>(plain_text), len);

  free(plain_text);

  return decrypted;
}

QByteArray Encrypto::randomBytes(int size)
{
  // declare array of size
  unsigned char arr[size];

  // call OPENSSL random generator
  RAND_bytes(arr, size);

  // reinterpret back to ByteArray
  QByteArray buffer = QByteArray(reinterpret_cast<char*>(arr), size);

  // return
  return buffer;
}

void Encrypto::freeRSAKey(RSA *key)
{
  // free objects memory
  RSA_free(key);
}

void Encrypto::initialiseOpenSSL()
{
  // loading Err Crypto if - fails
  ERR_load_CRYPTO_strings();
  // loading algorithms
  OpenSSL_add_all_algorithms();
  // loading SSL
  OPENSSL_config(NULL);
}

void Encrypto::finaliserOpenSSL()
{
  // Envelopes clean up
  EVP_cleanup();
  // free strings
  ERR_free_strings();
}

QByteArray Encrypto::readFile(QString fileName)
{
  QByteArray data;
  QFile file(fileName);
  if(!file.open(QFile::ReadOnly))
  {
    qDebug() << "Could not reading file";

    return data;
  }

  data = file.readAll();
  file.close();
  return data;
}

void Encrypto::writeFile(QString fileName, QByteArray &data)
{
  QFile file(fileName);
  if(!file.open(QFile::WriteOnly))
  {
    qDebug() << "Could not write file";

    return;
  }

  file.write(data);
  file.close();
}
