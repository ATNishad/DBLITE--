#include <iostream>
#include <vector>
#include<limits>
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


typedef struct{
    std::string table_name;
    std::string values;
}S_INSERT_QUERY;


class TOKENIZER{

    private:
    size_t _pos = 0;
    std::string _query;
    std::vector<TOKEN> _token_vec;

    public:
    TOKENIZER(std::string& query) : _query(query){
        tokenize();
    }

    std::vector<TOKEN> get_token_vec(){
        return _token_vec;
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

class PARSER{
    private:
    size_t _pos=0;
    S_INSERT_QUERY insert_query;
    std::vector<TOKEN> _t_v;

    public:
    PARSER(std::vector<TOKEN> temp ) : _t_v(temp){
        parser_fn();
    } 
    
    void parser_fn(){

        if(_t_v[_pos].word == "INSERT" && _t_v[_pos].tk_type == TOKENTYPE::KEYWORD ){
            insert_parse();
        }
        else if(_t_v[_pos].word == "SELECT" && _t_v[_pos].tk_type == TOKENTYPE::KEYWORD ){
            select_parse();
        }
        else if(_t_v[_pos].word == "UPDATE" && _t_v[_pos].tk_type == TOKENTYPE::KEYWORD ){
            update_parse();
        }
        else if(_t_v[_pos].word == "DELETE" && _t_v[_pos].tk_type == TOKENTYPE::KEYWORD ){
            delete_parse();
        }

    }

    void insert_parse(){
        if(_t_v[_pos].word != "INSERT"){
            throw std::runtime_error("SYNTAX ERROR : expected 'INSERT' ");
        }
        _pos++;

        if(_t_v[_pos].word != "INTO"){
            throw std::runtime_error("SYNTAX ERROR : expected 'INTO' ");
        }
        _pos++;

        if(_t_v[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::runtime_error("SYNTAX ERROR : expected table name ");

        }
        insert_query.table_name = _t_v[_pos].word;
        _pos++;

        if(_t_v[_pos].word != "VALUES"){
            throw std::runtime_error("SYNTAX ERROR : expected 'VALUES' ");
        }
        _pos++;

        if(_t_v[_pos].word != "("){
            throw std::runtime_error("SYNTAX ERROR : expected '(' ");
        }
        _pos++;

        std::string values;
        while(_pos < _t_v.size() && _t_v[_pos].word != ")"){
            values += _t_v[_pos].word;
            _pos++;
        }

        if(_t_v[_pos].word != ")"){
            throw std::runtime_error("SYNTAX ERROR : expected ')' ");
        }
        _pos++;

        if(_t_v[_pos].word != ";"){
            throw std::runtime_error("SYNTAX ERROR : expected ';' ");
        }
        _pos++;

        insert_query.values = values;        
    }
     
    void select_parse(){
    }

    void update_parse(){

    }

    void delete_parse(){

    }
};


class QUERY_ENGINE{
    private:
    std::string query;

    public:
    QUERY_ENGINE(std::string temp) : query(temp){
        TOKENIZER tk_obj(query);
        PARSER pars_obj(tk_obj.get_token_vec());
    }
};

int main(){
    std::string query;
    while(true){
        std::cout<<"dbl>";
        std::cout.flush();
        std::getline(std::cin,query);

        if(query == ".exit"){
            std::cout<<"Program exited";
            break;
        }

        else{
            try{
            QUERY_ENGINE QE(query);
            }
            catch(const std::exception& e){
                std::cout<<"Error:"<<e.what()<<"\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                query="";
                continue;

            }
        }
        
    }
}

