#include<iostream>


int main(){
    std::string query;
    while(true){
        std::cout<<"dbl>";
        std::cin>>query;

        if(query == ".exit"){
            std::cout<<"Program exited";
            break;
        }
    }
}

