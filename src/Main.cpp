#include "ace/Get_Opt.h"
#include <iostream>

int main(int argc, char *argv[]){
    //Specify option string so that switches b, d, f and h all expect 
    //arguments. Switches a, c, e and g expect no arguments. 
    //像b: d:这种就是希望有arguments的。a c这样的就是不希望有args的。
    // t的意思是thread数目
    // h的意思是帮助



    ACE_Get_Opt get_opt (argc, argv, "t:h"); 
    int c;

    while((c = get_opt()) != EOF){
        switch(c){
            case 't':
            break;
            case 'h':
            std::cout << "aa\n" <<
                        "bb\n";
            break;
        }
    }
}