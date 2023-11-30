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
    Next(T, tlen, next);//鏍规嵁妯″紡涓睺,鍒濆鍖杗ext鏁扮粍
    int i=1;
    int j=1;
    while (i<=slen && j<=tlen) {
        //j==0:浠ｈ〃妯″紡涓茬殑绗竴涓瓧绗﹀氨鍜屽綋鍓嶆祴璇曠殑瀛楃涓嶇浉绛夛紱S[i-1]==T[j-1],濡傛灉瀵瑰簲浣嶇疆瀛楃鐩哥瓑锛屼袱绉嶆儏鍐典笅锛屾寚鍚戝綋鍓嶆祴璇曠殑涓や釜鎸囬拡涓嬫爣i鍜宩閮藉悜鍚庣Щ
        if (j==0 || S[i-1]==T[j-1]) {
            i++;
            j++;
        }
        else{
            j=next[j];//濡傛灉娴嬭瘯鐨勪袱涓瓧绗︿笉鐩哥瓑锛宨涓嶅姩锛宩鍙樹负褰撳墠娴嬭瘯瀛楃涓茬殑next鍊�
        }
    }
    if (j>tlen) 
    {
        //濡傛灉鏉′欢涓虹湡锛岃鏄庡尮閰嶆垚鍔�
        return i-tlen;
    }
    return -1;
}
