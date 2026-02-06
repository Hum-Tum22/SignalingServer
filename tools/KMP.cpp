#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "KMP.h"

void getNext(const unsigned char *p,const int plen,int *next)
{
	int j,k;
	next[0]=-1;
	j=0;
	k=-1;
	while(j<plen-1)
	{
		if(k==-1||p[j]==p[k])    //匹配的情况下,p[j]==p[k]
		{
			j++;
			k++;
			next[j]=k;
		}
		else                   //p[j]!=p[k]
			k=next[k];
	}
}
int KMPMatch(const unsigned char *s,const int slen,const unsigned char *p,const int plen)
{
	int *pnext = (int *)calloc(plen,sizeof(int)); //int[plen];//[plen];
	int i,j;
	i=0;
	j=0;
	getNext(p,plen,pnext);
	while(i<slen)
	{
		if(j==-1||s[i]==p[j])
		{
			i++;
			j++;
		}
		else
		{
			j=pnext[j];       //消除了指针i的回溯
		}
		if(j==plen)
		{
			//delete []pnext;
			free(pnext);
			return i-plen;
		}
	}
	//delete []pnext;
	free(pnext);
	return -1;
}
void Next(unsigned char*T,const int &tlen, int *next){
    int i=1;
    next[1]=0;
    int j=0;
    while (i<tlen)
    {
        if (j==0||T[i-1]==T[j-1])
        {
            i++;
            j++;
            if (T[i-1]!=T[j-1])
            {
               next[i]=j;
            }
            else
            {
                next[i]=next[j];
            }
        }else{
            j=next[j];
        }
    }
}
int KMP(unsigned char * S, const int &slen, unsigned char * T, const int &tlen)
{
    int next[10];
    Next(T, tlen, next);
    int i=1;
    int j=1;
    while (i<=slen && j<=tlen) {
        if (j==0 || S[i-1]==T[j-1]) {
            i++;
            j++;
        }
        else{
            j=next[j];
        }
    }
    if (j>tlen) 
    {
        return i-tlen;
    }
    return -1;
}
