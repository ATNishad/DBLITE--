#include <iostream>
#include <vector>
#include <unordered_set>

std::unordered_set<std::string> SQL_KEYWORDS= {
    "SELECT", "INSERT", "UPDATE", "DELETE", "FROM", "WHERE",
    "VALUES", "INTO", "CREATE", "DROP", "TABLE"
};

typedef enum class TKTYP{
    KEYWORD,
    IDENTIFIER,
    SYMBOL,
    STRING,
    NUMBER,
    END
}TOKENTYPE;

typedef struct{
    TOKENTYPE tk_type;
    std::string word;
}TOKEN;


class TOKENIZER{
private:
    int _pos = 0;
    std::string _query;
    std::vector<TOKEN> _token_vec;

public:
    TOKENIZER(std::string& query) : _query(query){
        tokenize();
    }

    void display(){
        for(auto& tkn : _token_vec){
            std::cout<<"Token:"<<tkn.word<<'\n';
        }
    }

    void tokenize(){
        while(_pos < _query.size()){
            if(isspace(_query[_pos])){
                _pos++;
                continue;
            }

            if(isalpha(_query[_pos])){
                std::string word;
                while(_pos < _query.size() && isalnum(_query[_pos])){
                    word += _query[_pos++];
                }

                TOKENTYPE type = (SQL_KEYWORDS.count(word) > 0) ? TOKENTYPE::KEYWORD : TOKENTYPE::IDENTIFIER;
                _token_vec.push_back({type, word});
            }

            else if(isdigit(_query[_pos])){
                std::string num;
                while(_pos < _query.size() && isdigit(_query[_pos])){
                    num += _query[_pos++];
                }
                _token_vec.push_back({TOKENTYPE::NUMBER, num});
            }

            else if(_query[_pos] == '\'' || _query[_pos] == '"'){
                char quote = _query[_pos++];
                std::string str;
                while(_pos < _query.size() && _query[_pos] != quote){
                    str += _query[_pos++];
                }
                _pos++; //to skip closing quote
                _token_vec.push_back({TOKENTYPE::STRING, str});
            }

            else{
                _token_vec.push_back({TOKENTYPE::SYMBOL, std::string(1, _query[_pos++])});
            }
        }
        _token_vec.push_back({TOKENTYPE::END, ""});
    }
};


int main(){
    std::string query;
    while(true){
        std::cout<<"dbl>";
        std::cin>>query;

        if(query == ".exit"){
            std::cout<<"Program exited";
            break;
        }

        else{
            TOKENIZER tokenizer_obj(query);
        }
        
    }
}

