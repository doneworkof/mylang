#include <iostream>
#include <vector>
#include <string>
#include <functional>

using namespace std;

#define NUMBER 0
#define STRING 1

#define DEFAULT 0
#define PROTECTED 1
#define STATIC 2

#define CommandFunc function<void(vector<Variable>, AdditArgTable)>

string var_forbidden_symb = "!'#$%&\"()*+,-./:;<=>?@[\\]^`{|}~ \n";
vector<float> storage_number;
vector<string> storage_string;


int stvar_counter = -1;

string generate_name_of_stvar() {
    stvar_counter++;
    return "%static" + to_string(stvar_counter) + "%";
}


class Variable {
private:
    string name;
    int type;
    int ptr;
    int subtype;

public:
    Variable() {
        name = "%NULLVAR%";
        type = -1;
        ptr = -1;
        subtype = -1;
    }
    Variable(string name, int type, int ptr, int subtype=DEFAULT) {
        this->name = subtype == STATIC? generate_name_of_stvar() : name;
        this->type = type;
        this->ptr = ptr;
        this->subtype = subtype;
    }

    string to_string() {
        if (type == NUMBER)
            return std::to_string(storage_number[ptr]);
        return storage_string[ptr];
    }

    int get_subtype() {
        return subtype;
    }

    int get_ptr() {
        return ptr;
    }

    int get_type() {
        return type;
    }

    string get_name() {
        return name;
    }

    void set_ptr(int new_ptr) {
        ptr = new_ptr;
    }

    bool compare(Variable other) {
        return other.get_name() == name && other.get_type() == type &&
            other.get_ptr() == ptr && other.get_subtype() == subtype;
    }

};

vector<Variable> storage_var;
Variable NULLVAR;


bool is_nullvar(Variable var) {
    return var.compare(NULLVAR);
}

class AdditArg {
private:
    Variable var;
    string name;

public:
    AdditArg(string name, Variable var) {
        this->name = name;
        this->var = var;
    }
    string get_name() {
        return name;
    }
    Variable get_var() {
        return var;
    }
};

class AdditArgTable {
private:
    vector<AdditArg> args;
public:
    AdditArgTable(vector<AdditArg> args) {
        this->args = args;
    }
    Variable get_arg(string name) {
        for (int i = 0; i < args.size(); i++)
            if (args[i].get_name() == name) return args[i].get_var();
        return NULLVAR;
    }
    string string_or(string name, string def = "") {
        Variable var = get_arg(name);
        if (!is_nullvar(var)) return var.to_string();
        return def;
    }
    float number_or(string name, float def = 0) {
        Variable var = get_arg(name);
        if (!is_nullvar(var) || var.get_type() != NUMBER)
            return storage_number[var.get_ptr()];
        return def;
    }
};



class Command {
private:
    CommandFunc func;
    string name;
public:
    Command(string name, CommandFunc func) {
        this->name = name;
        this->func = func;
    }
    string get_name() {
        return name;
    }
    void call(vector<Variable> vars, AdditArgTable AAT) {
        func(vars, AAT);
    }
};

vector<Command> storage_commands;


void error(string message = "unknown error") {
    cout << "Error in UrineLang.exe occured!!" << endl;
    cout << "Message: " << message << endl;
    system("pause");
    exit(0);
}

void assert(bool cond, string message = "assert failure") {
    if (cond) error(message);
}

Variable get_var_by_name(string name) {
    for (int i = 0; i < storage_var.size(); i++)
        if (storage_var[i].get_name() == name) return storage_var[i];
    error("We didn't find your variable.");
    return NULLVAR;
}

Variable get_var_by_type_and_ptr(int type, int ptr) {
    for (int i = 0; i < storage_var.size(); i++)
        if (storage_var[i].get_type() == type && storage_var[i].get_ptr() == ptr)
            return storage_var[i];
    error("We didn't find your variable.");
    return NULLVAR;
}


template<typename T>
bool _shift_ptr(vector<T> & vec, int ptr) {
    bool state = ptr + 1 != vec.size();;
    vec[ptr] = vec[vec.size() - 1];
    vec.erase(vec.end());
    return state;
}

bool free_storage(int type, int ptr) {
    if (type == NUMBER)
        return _shift_ptr<float>(storage_number, ptr);
    return _shift_ptr<string>(storage_string, ptr);
}

int get_index_of_var(string name) {
    for (int i = 0; i < storage_var.size(); i++)
        if (storage_var[i].get_name() == name) return i;
    return -1;
}

bool delete_var(string name) {
    Variable var = get_var_by_name(name);
    if (var.get_subtype() == PROTECTED) return false;
    bool replaced = free_storage(var.get_type(), var.get_ptr());

    if (replaced) {
        int last_ptr = var.get_type() == NUMBER ?
            storage_number.size() - 1 : storage_string.size() - 1;
        Variable var2 = get_var_by_type_and_ptr(var.get_type(), last_ptr);
        var2.set_ptr(var.get_ptr());
    }

    int index = get_index_of_var(var.get_name());
    storage_var.erase(storage_var.begin() + index);

    return true;
}

bool is_name_free(string name) {
    for (int i = 0; i < storage_var.size(); i++)
        if (storage_var[i].get_name() == name) return false;
    return true;
}

bool contains_any_of(string str, string set) {
    for (int i = 0; i < set.length(); i++)
        if (str.find(set[i]) < str.length()) return true;
    return false;
}

bool check_name(string name) {
    return !isdigit(name[0]) && is_name_free(name) && !contains_any_of(name, var_forbidden_symb);
}

Variable _creation_success(string name, int type, int ptr, int subtype) {
    Variable var(name, type, ptr, subtype);
    storage_var.push_back(var);
    return storage_var[storage_var.size() - 1];
}

Variable create_number(string name, float value, int subtype = DEFAULT) {
    if (!check_name(name))
        return NULLVAR;

    int ptr = storage_number.size();
    storage_number.push_back(value);

    return _creation_success(name, NUMBER, ptr, subtype);
}

Variable create_string(string name, string value, int subtype = DEFAULT) {
    if (!check_name(name))
        return NULLVAR;

    int ptr = storage_string.size();
    storage_string.push_back(value);

    return _creation_success(name, STRING, ptr, subtype);
}

string var_to_string(string name) {
    Variable var = get_var_by_name(name);
    return var.to_string();
}

bool strweight(string str) {
    for (int i = 0; i < str.length(); i++)
        if (str[i] != '\n' || str[i] != ' ') return true;
    return false;
}

vector<string> advanced_split(string str, char ch = ' ') {
    vector<string> pieces;
    bool quotes = false;
    string current = "";
    str += ch;

    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ch && !quotes && strweight(current)) {
            pieces.push_back(current);
            current = "";
            continue;
        }
        if (str[i] == '"')
            quotes = !quotes;
        current += str[i];
    }

    return pieces;
}

string strip(string str, char ch) {
    int start_idx = -1;
    int end_idx = -1;
    int n = str.length();

    for (int i = 0; i < n; i++) {
        if (start_idx == -1 && str[i] != ch)
            start_idx = i;
        if (end_idx == -1 && str[n - i - 1] != ch)
            end_idx = n - i - 1;
    }

    return str.substr(start_idx, end_idx - start_idx + 1);
}

Variable parse_variable(string str) {
    str = strip(str, ' ');

    if (isdigit(str[0])) {
        float val = stof(str);
        return create_number("", val, STATIC);
    }
    else if (contains_any_of(str, "\"")) {
        str = strip(str, '"');
        return create_string("", str, STATIC);
    }
    return get_var_by_name(str);
}

bool create_command(string name, CommandFunc func) {
    Command command(name, func);
    storage_commands.push_back(command);
    return true;
}


void compile(string str) {
    str = strip(str, ' ');
    vector<string> splitted = advanced_split(str);
    string command_name = splitted[0];
    vector<Variable> vars;
    vector<AdditArg> addit_args;
    
    for (int i = 1; i < splitted.size(); i++) {
        if (contains_any_of(splitted[i], "=")) {
            vector<string> arg_spl = advanced_split(splitted[i], '=');
            assert(arg_spl.size() != 2);
            Variable var = parse_variable(arg_spl[1]);
            addit_args.push_back(AdditArg(arg_spl[0], var));
        }
        else {
            Variable var = parse_variable(splitted[i]);
            vars.push_back(var);
        }
    }

    AdditArgTable AAT(addit_args);

    for (int i = 0; i < storage_commands.size(); i++)
        if (storage_commands[i].get_name() == command_name) {
            storage_commands[i].call(vars, AAT);
            break;
        }
}


void print(vector<Variable> vars, AdditArgTable AAT) {
    string output = "";

    string ending = AAT.string_or("ending", "\n");
    string sep = AAT.string_or("sep", " ");

    for (int i = 0; i < vars.size(); i++)
        output += vars[i].to_string()
        + (i + 1 == vars.size() ? "" : sep);

    output += ending;
    cout << output;
}

void var(vector<Variable> vars, AdditArgTable AAT) {
    if (vars.size() != 2) error("Incorrect count of arguments");
    else if (vars[0].get_type() != STRING) error("First value must be the name.");

    int type = vars[1].get_type();

    if (type == STRING) {
        string val = vars[1].to_string();
        if (is_nullvar(create_string(
            vars[0].to_string(), val
        ))) error("Cannot create variable.");
    }
    else {
        float val = storage_number[vars[1].get_ptr()];
        if (is_nullvar(create_number(
            vars[0].to_string(), val
        ))) error("Cannot create variable.");
    }
}


int main()
{
    create_string("space", " ", PROTECTED);
    create_string("newline", "\n", PROTECTED);
    create_number("pi", 3.14159265359, PROTECTED);

    create_command("print", print);
    create_command("var", var);

    compile("var \"abc\" 5");
    compile("print abc abc abc sep=newline");

    return 0;
}
