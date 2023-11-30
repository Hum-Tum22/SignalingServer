#ifndef KMP_H
#define KMP_H
#ifdef __cplusplus
extern "C"
{
#endif
	int KMPMatch(const  unsigned char *s, const int slen, const unsigned char *p, const int plen);
	int KMP(unsigned char *S, const int &slen, unsigned char *T, const int &tlen);

#ifdef __cplusplus
}
#endif
#endif