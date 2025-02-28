#include <iostream>
#include <vector>
#include<limits>
#include <unordered_set>
#include<unordered_map>
#include<functional>
#include<memory>

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
    std::vector<std::pair<std::string,std::string>> column_name_type {};
};

struct S_DROP_QUERY : public BASE_QUERY_S{
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
    std::unique_ptr<BASE_QUERY_S> query_ptr;

    public:
    PARSER(std::vector<TOKEN> temp ) : _token_vec(temp){
        parser_fn();
    } 

    void parser_fn(){
        std::unordered_map<std::string, std::function<void()>>handler_umap = {
            {"CREATE", std::bind(&PARSER::create_parse,this)},
            {"DROP"  , std::bind(&PARSER::drop_parse,this)},
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

    std::unique_ptr<BASE_QUERY_S> get_parsed_query(){
        return std::move(query_ptr);
    }

    void create_parse(){
        if(_token_vec[_pos].word != "CREATE"){
            throw std::invalid_argument("syntax error : expected 'CREATE' ");
        }
        _pos++;

        if(_token_vec[_pos].word != "TABLE"){
            throw std::invalid_argument("syntax error : expected 'TABLE' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("SYNTAX ARGUMENT : expected tablename ");
        }
        auto create_query = std::make_unique<S_CREATE_QUERY>();        //query object here!!!
        create_query->tablename = _token_vec[_pos].word;
        _pos++;
        

        if(_token_vec[_pos].word != "("){
            throw std::invalid_argument("syntax error : expected '(' ");
        }
        _pos++;

        while(_pos< _token_vec.size() && _token_vec[_pos].word != ")" ){
            

            if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
                throw std::invalid_argument("syntax error : expected column name ");
            }
            std::string col = _token_vec[_pos].word;
            _pos++;

            if(_token_vec[_pos].word != ":"){
                throw std::invalid_argument("syntax error : expected ':' ");
            }
            _pos++;

            if(_token_vec[_pos].word != "STRING" && _token_vec[_pos].word != "NUMBER" ){

                throw std::invalid_argument("syntax error : expected column type ");
                }
                

            std::string type = _token_vec[_pos].word;
            _pos++; 

            create_query->column_name_type.push_back({col,type});

            if(_token_vec[_pos].word == ","){
                _pos++;
            }
            else if(_token_vec[_pos].word != ")" ){
                throw std::invalid_argument("syntax error : expected ') or ,' ");
            }

        }
        if(_token_vec[_pos].word != ")"){
            throw std::invalid_argument("syntax error : expected ')' ");
        }
        _pos++;

        if(_pos >= _token_vec.size() || _token_vec[_pos].word != ";"){
            throw std::invalid_argument("syntax error : expected ';' ");
        }

        _pos++;
        query_ptr = std::move(create_query);
        std::cout<<"Creat query parsed\n";
    }

    void drop_parse(){
        if(_token_vec[_pos].word != "DROP"){
            throw std::invalid_argument("syntax error : expected 'DROP' ");
        }
        _pos++;

        if(_token_vec[_pos].word != "TABLE"){
            throw std::invalid_argument("syntax error : expected 'TABLE' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("syntax error : expected tablename");
        }
        auto drop_query = std::make_unique<S_DROP_QUERY>();
        drop_query->tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != ";"){
            throw std::invalid_argument("syntax error : expected ;");
        }
        _pos++;

        query_ptr = std::move(drop_query);
        std::cout<<"Drop query parsed\n";
        }
        
    void insert_parse(){
        if(_token_vec[_pos].word != "INSERT"){
            throw std::invalid_argument("syntax error : expected 'INSERT' ");
        }
        _pos++;
        S_INSERT_QUERY insert_query;    //query object here!!!

        if(_token_vec[_pos].word != "INTO"){
            throw std::invalid_argument("syntax error : expected 'INTO' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("syntax error : expected table name ");

        }
        insert_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != "VALUES"){
            throw std::invalid_argument("syntax error : expected 'VALUES' ");
        }
        _pos++;

        if(_token_vec[_pos].word != "("){
            throw std::invalid_argument("syntax error : expected '(' ");
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
                throw std::invalid_argument("syntax error : unexpected token in values");
            }
        
        }

        if(_token_vec[_pos].word != ")"){
            throw std::invalid_argument("syntax error : expected ')' ");
        }
        _pos++;

        if(_token_vec[_pos].word != ";"){
            throw std::invalid_argument("syntax error : expected ';' ");
        }
        _pos++;
        std::cout<<"INSERT PARSE SUCCESSFUL";
    }
     
    void select_parse(){
        if(_token_vec[_pos].word != "SELECT"){
            throw std::invalid_argument("syntax error : expected 'SELECT' ");
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
            throw std::invalid_argument("syntax error : expected 'FROM' ");
        }
        _pos++;
        
        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("syntax error : expected table name ");
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
            throw std::invalid_argument("syntax error : expected ';' ");
        }
        _pos++;
        std::cout<<"SELECT PARSE SUCCESSFUL";

    }

    void update_parse(){
        if(_token_vec[_pos].word != "UPDATE"){
            throw std::invalid_argument("syntax error : expected 'UPDATE' ");
        }

        _pos++;
        S_UPDATE_QUERY update_query;    //query object here!!!

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
            throw std::invalid_argument("syntax error : expected tablename");
        }
        update_query.tablename = _token_vec[_pos].word;
        _pos++;

        if(_token_vec[_pos].word != "SET"){
            throw std::invalid_argument("syntax error : expected 'SET' ");
        }
        _pos++;

        while(_pos < _token_vec.size() && _token_vec[_pos].word != "WHERE" && _token_vec[_pos].word != ";"){
            if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER){
                throw std::invalid_argument("syntax error : expected column name");
            }

            std::string column = _token_vec[_pos].word;
            _pos++;

            if(_token_vec[_pos].word != "="){
                throw std::invalid_argument("syntax error : expected =");
            }
            _pos++;

            if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER &&
                 _token_vec[_pos].tk_type != TOKENTYPE::STRING &&
                 _token_vec[_pos].tk_type != TOKENTYPE::NUMBER ){
                     throw std::invalid_argument("syntax error : expected value after = ");
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
                throw std::invalid_argument("syntax error : expected token in SET");
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
            throw std::invalid_argument("syntax error : expected ';' at end");
        }
        _pos++;
        std::cout<<"UPDATE PARSE SUCCESSFUL";

    }

    void delete_parse(){
        if(_token_vec[_pos].word != "DELETE"){
            throw std::invalid_argument("syntax error : expected 'DELETE' ");
        }
        _pos++;
        S_DELETE_QUERY delete_query;        //query object here!!!

        if(_token_vec[_pos].word != "FROM"){
            throw std::invalid_argument("syntax error : expected 'FROM' ");
        }
        _pos++;

        if(_token_vec[_pos].tk_type != TOKENTYPE::IDENTIFIER ){
            throw std::invalid_argument("syntax error : expected table name");
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


class DATABASE{
    private:
    struct TABLE{
        std::vector<std::string> columns;
        std::vector<std::unordered_map<std::string,std::string>> rows;
    };

    std::unordered_map<std::string,TABLE> tables;

    public:

    void execute_create(const S_CREATE_QUERY& create_query){
        if(tables.find(create_query.tablename)!= tables.end()){
            std::cout<<"Error : Table "<<create_query.tablename<<" already exists \n";
            return;
        }

        TABLE newtable;
        for(const auto& col_type : create_query.column_name_type){
            newtable.columns.push_back(col_type.first);
        }

        newtable.rows = {};

        tables[create_query.tablename] = newtable;
        std::cout<<"Table "<<create_query.tablename<<" has been created successfully \n";
    }

    void execute_drop(const S_DROP_QUERY& drop_query){
        if(tables.find(drop_query.tablename) == tables.end()){
            std::cout<<"Table "<<drop_query.tablename<<" doesnt exist \n";        
            return;    
        }

        else{
            tables.erase(tables.find(drop_query.tablename));
            std::cout<<"Table "<<drop_query.tablename<<" has been dropped successfully \n";
        }

    }

    void execute_insert(){}

    void execute_update(){}

    void execute_select(){}

    void execute_delete(){}

};

class QUERY_ENGINE{
    private:
    std::string query;
    DATABASE &db;


    public:
    QUERY_ENGINE(std::string temp, DATABASE& dtemp) : query(temp) , db(dtemp){
        TOKENIZER tk_obj(query);
        PARSER pars_obj(tk_obj.get_token_vec());
        auto parsed_query = pars_obj.get_parsed_query();
        if(parsed_query){
            if(S_CREATE_QUERY* create_query = dynamic_cast<S_CREATE_QUERY*>(parsed_query.get())){
                db.execute_create(*create_query);
            }
            if(S_DROP_QUERY* drop_query = dynamic_cast<S_DROP_QUERY*>(parsed_query.get())){
                db.execute_drop(*drop_query);
            }
        }
    }
};

int main(){
    std::string query;
    DATABASE db;

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
            QUERY_ENGINE QE(query,db);
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

