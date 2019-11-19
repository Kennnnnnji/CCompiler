#ifndef TABLE_H_
#define TABLE_H_
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

enum class BaseType {
	VOID, INT, CHAR, STRING
};

struct f {
	bool returned;
	// f_ret_type ret_type;
	string name;
};

class Specifier {
public:
	bool isConst = false;
	BaseType baseType;
	Specifier();
	Specifier(bool iscon, BaseType t);
};

class SymbolC {
private:
	bool valid = false;	// if is deleted

public:
	string name;
	string rname;

	int addr;		// address
	int size = 4;		// bytes
	int dims;		// dimension
	int line;		// declared curLineNum count
	vector<int> refVec;	// refered linesInter
	int level;		// variable level
	int value;		// value of var
	bool duplicate = false;	//是否是同名变量
	bool func = false;
	bool param = false;
	bool arr = false;
	string strVal;

	Specifier spec;

	vector<SymbolC *> args;	//如果该符号对应的是函数名,那么args指向函数的输入参数符号列表
	SymbolC * next;	//指向下一个同层次的变量符号

	SymbolC();	// by default, valid = false
	SymbolC(string nam, int lev, int lin, Specifier spc);	// valid=true
	void set_spec(Specifier spc);
	Specifier *get_spec();
	bool is_ret_func();
	void set_valid(bool b);
	bool is_valid();
};

class Level {
public:
	int id;
	int outid;
	Level(int id, int outid);
};

class LevelMng {
public:
	vector<Level> levelVec;
};

class SymbolEntry {
private:
	SymbolEntry *prev = NULL, *next = NULL;
public:
	int outLevel;
	SymbolC symbolc;
	SymbolEntry();
	SymbolEntry(SymbolC symc);
	void setPrev(SymbolEntry *prev_);
	void setNext(SymbolEntry *next_);
	SymbolEntry *getPrev();
	SymbolEntry *getNext();
};

class SymTable {
public:
	unordered_multimap<string, SymbolC> sym_map;
	vector<SymbolEntry> entryVec;
	vector<f> fVec;
	bool contain(string ss);
	SymbolC *find(string nam, int level, bool outer);
	bool contain(string nam, int level, bool outer);
	SymbolC* add(SymbolC s, int out_level);
	void devalid(int level);
};
#endif
