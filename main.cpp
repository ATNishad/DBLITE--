#include <iostream>
#include <vector>
#include<limits>
#include <unordered_set>
#include<unordered_map>
#include<functional>

std::unordered_set<std::string> SQL_KEYWORD_SET= {
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

struct BASE_QUERY_S{
    std::string tablename;
    virtual ~BASE_QUERY_S() {} 
};

struct S_CREATE_QUERY : public BASE_QUERY_S{
    std::vector<std::pair<std::string,std::string>> column_name_type;
};

struct S_INSERT_QUERY : public BASE_QUERY_S{
    std::vector<std::string> values;
};

struct S_SELECT_QUERY : public BASE_QUERY_S{
    std::vector<std::string> columns;
    std::string condition;
};

struct S_UPDATE_QUERY : public BASE_QUERY_S{
    std::unordered_map<std::string , std::string> updates;
    std::string condition;
};

struct S_DELETE_QUERY : public BASE_QUERY_S{
    std::string condition;
};


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

                TOKENTYPE type = (SQL_KEYWORD_SET.count(word) > 0) ? TOKENTYPE::KEYWORD : TOKENTYPE::IDENTIFIER;
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
    std::vector<TOKEN> _token_vec;

    public:
    PARSER(std::vector<TOKEN> temp ) : _token_vec(temp){
        parser_fn();
    } 
    
    void parser_fn(){
        std::unordered_map<std::string, std::function<void()>>handler_umap = {
            {"INSERT", std::bind(&PARSER::insert_parse,this)},
            {"SELECT", std::bind(&PARSER::select_parse,this)},
            {"UPDATE", std::bind(&PARSER::update_parse,this)},
            {"DELETE", std::bind(&PARSER::delete_parse,this)},
        };

        auto itr = handler_umap.find(_token_vec[_pos].word);
        if(itr != handler_umap.end()){
            itr->second();
        }
        else{
            throw std::invalid_argument("Unkown query");
        }

    }

    void create_parse(){
        if(_token_vec[_pos].word != "CREATE"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'CREATE' ");
        }
        _pos++;

        if(_token_vec[_pos].word != "TABLE"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'TABLE' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("SYNTAX ARGUMENT : expected tablename ");
        }
        S_CREATE_QUERY create_query;
        create_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != "("){
            throw std::invalid_argument("SYNTAX ERROR : expected '(' ");
        }
        _pos++;

        while(_pos< _token_vec.size() && _token_vec[_pos].word != ")" ){
    
            //parse col1 TYPE, col2 TYPE
        }

        

    }

    void insert_parse(){
        if(_token_vec[_pos].word != "INSERT"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'INSERT' ");
        }
        _pos++;
        S_INSERT_QUERY insert_query;    //query object here

        if(_token_vec[_pos].word != "INTO"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'INTO' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("SYNTAX ERROR : expected table name ");

        }
        insert_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != "VALUES"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'VALUES' ");
        }
        _pos++;

        if(_token_vec[_pos].word != "("){
            throw std::invalid_argument("SYNTAX ERROR : expected '(' ");
        }
        _pos++;

        while(_pos < _token_vec.size() && _token_vec[_pos].word != ")"){
            
            if(_token_vec[_pos].tk_type == TOKENTYPE::STRING ||
            _token_vec[_pos].tk_type == TOKENTYPE::NUMBER || 
            _token_vec[_pos].tk_type == TOKENTYPE::IDENTIFIER)
            {
            insert_query.values.push_back(_token_vec[_pos].word);  
            _pos++;      
            }

            else if( _token_vec[_pos].word == ","){
                _pos++;
            }

            else{
                throw std::invalid_argument("SYNTAX ERROR : unexpected token in values");
            }
        
        }

        if(_token_vec[_pos].word != ")"){
            throw std::invalid_argument("SYNTAX ERROR : expected ')' ");
        }
        _pos++;

        if(_token_vec[_pos].word != ";"){
            throw std::invalid_argument("SYNTAX ERROR : expected ';' ");
        }
        _pos++;
        std::cout<<"INSERT PARSE SUCCESSFUL";
    }
     
    void select_parse(){
        if(_token_vec[_pos].word != "SELECT"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'SELECT' ");
        }
        S_SELECT_QUERY select_query;   //query object here
        _pos++;

        if(_token_vec[_pos].word == "*"){
            select_query.columns.push_back("*");
            _pos++;
        }
        else{
            while(_token_vec[_pos].tk_type == TOKENTYPE::IDENTIFIER){
                select_query.columns.push_back(_token_vec[_pos].word);
                _pos++;

                if(_token_vec[_pos].word == ","){
                    _pos++;
                    continue;
                }
                else{
                    break;
                }
            }
        }
        
        if(_token_vec[_pos].word != "FROM"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'FROM' ");
        }
        _pos++;
        
        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("SYNTAX ERROR : expected table name ");
        }
        select_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word == "WHERE"){
            _pos++;

            while(_pos < _token_vec.size() && _token_vec[_pos].word != ";"){
                if(!select_query.condition.empty()){
                    select_query.condition += " "; 
                select_query.condition += _token_vec[_pos].word;
                _pos++;
            }
        }
        }

        if(_token_vec[_pos].word != ";"){
            throw std::invalid_argument("SYNTAX ERROR : expected ';' ");
        }
        _pos++;
        std::cout<<"SELECT PARSE SUCCESSFUL";

    }

    void update_parse(){
        if(_token_vec[_pos].word != "UPDATE"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'UPDATE' ");
        }

        _pos++;
        S_UPDATE_QUERY update_query;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("SYNTAX ERROR : expected tablename");
        }
        update_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != "SET"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'SET' ");
        }
        _pos++;

        while(_pos < _token_vec.size() && _token_vec[_pos].word != "WHERE" && _token_vec[_pos].word != ";"){
            if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
                throw std::invalid_argument("SYNTAX ERROR : expected column name");
            }

            std::string column = _token_vec[_pos].word;
            _pos++;

            if(_token_vec[_pos].word != "="){
                throw std::invalid_argument("SYNTAX ERROR : expected =");
            }
            _pos++;

            if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER &&
                 _token_vec[_pos].tk_type != TOKENTYPE::STRING &&
                 _token_vec[_pos].tk_type != TOKENTYPE::NUMBER ){
                     throw std::invalid_argument("SYNTAX ERROR : expected value after = ");
                    }
                    update_query.updates[column] = _token_vec[_pos].word;
                    _pos++;
                                    
            if(_token_vec[_pos].word == ","){
                _pos++;
                continue;
            }

            else if(_token_vec[_pos].word == "WHERE" || _token_vec[_pos].word == ";"){
                break;
            }
            else{
                throw std::invalid_argument("SYNTAX ERROR : expected token in SET");
            }
        }

        if(_token_vec[_pos].word == "WHERE"){
            _pos++;
            while(_pos< _token_vec.size() && _token_vec[_pos].word != ";"){
                update_query.condition += _token_vec[_pos].word + " ";
                _pos++;
            }
            update_query.condition = update_query.condition.substr(0,update_query.condition.size()-1);
        }

        if(_token_vec[_pos].word != ";"){
            throw std::invalid_argument("SYNTAX ERROR : expected ';' at end");
        }
        _pos++;
        std::cout<<"UPDATE PARSE SUCCESSFUL";

    }

    void delete_parse(){
        if(_token_vec[_pos].word != "DELETE"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'DELETE' ");
        }
        _pos++;
        S_DELETE_QUERY delete_query;

        if(_token_vec[_pos].word != "FROM"){
            throw std::invalid_argument("SYNTAX ERROR : expected 'FROM' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER ){
            throw std::invalid_argument("SYNTAX ERROR : expected table name");
        }
        delete_query.tablename = _token_vec[_pos].word;
        _pos++;

        
        if (_token_vec[_pos].word == "WHERE"){
            _pos++;
            while(_token_vec[_pos].word != ";" && _pos< _token_vec.size()){
                delete_query.condition += _token_vec[_pos].word;
                delete_query.condition += " ";
                _pos++;
            }
        }
        else if(_token_vec[_pos].word == ";"){
            _pos++;
        }
        std::cout<<"DELETE query parsed \n";
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

