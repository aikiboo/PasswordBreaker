/*
Christophe Devine 
c.devine@cr0.net
http://www.cr0.net:8040/code/crypto/
*/
#ifndef _MD5_H
#define _MD5_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#define MD5_HASHBYTES 16

typedef struct
{
    uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];
}
md5_context;

void calcul_md5( unsigned char *donnees, unsigned int taille, unsigned char *hash);
void md5_starts( md5_context *ctx );
void md5_update( md5_context *ctx, uint8 *input, uint32 length );
void md5_finish( md5_context *ctx, uint8 digest[16] );

#endif /* md5.h */

