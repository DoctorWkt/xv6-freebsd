/* tic tac toe */
#include <stdio.h>
#define P printf
enum {X='X',O='O',N='\0',C='C'};
static char ck(char b[9]) {
    int i, w[] = {0,1,2,3,4,5,6,7,8,0,3,6,1,4,7,2,5,8,0,4,8,2,4,6};
    for (i=0;i<24;i+=3) {
        if (b[w[i+0]]==X&&b[w[i+1]]==X&&b[w[i+2]]==X) return X;
        if (b[w[i+0]]==O&&b[w[i+1]]==O&&b[w[i+2]]==O) return O;
    } for (i=0;i<9;i++) if(b[i]==N) return N; return C;
}
#define S(i)(b[i]==X?"\033[1;32mX\033[m":b[i]==O?"\033[1;34mO\033[m":".")
#define B P("\n %s│%s│%s  0 1 2\n ─┼─┼─\n %s│%s│%s  3 4 5\n" \
" ─┼─┼─\n %s│%s│%s  6 7 8\n", S(0),S(1),S(2),S(3),S(4),S(5),S(6),S(7),S(8));
int main() { char b[9]={N},*res,l[32],p=X,w=N;int m,t=1;B;
    do { while(1){ P("\nTurn #%d. %c, your move (0-8): ",t,p);fflush(stdout);
            res=fgets(l,sizeof l,stdin);
            if(!(res==l&&(sscanf(l,"%d",&m)!=1||m<0||m>8||b[m]!=N)))break;
        };t++;b[m]=p;B;p=(p==X)?O:X;w=ck(b);
        if(w==C){P("\ndraw.\n");}else if(w!=N){P("\n%c won.\n",w);}
    } while(w==N); fflush(stdout); return 0;
}
