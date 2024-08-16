#include <stdio.h>
int x = 5;

int fun(int x){
    x = 2;

    return x;
}
//cmt
int main(){
    int y=55;
    // printf("%d\n", fun(x));
    // printf("%d\n", x);
    {
        //printf("%d",y);
        for(int i = 1; i < 2; i++){
            printf("%f",i);
        }

    }
    //new
    return 0;
}