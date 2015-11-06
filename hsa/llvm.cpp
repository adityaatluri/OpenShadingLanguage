#include"llvm/ADT/STLExtras.h"
#include<cctype>
#include<cstdio>
#include<map>
#include<string>
#include<vector>
#include<memory>
#include<iostream>
#include"llvm.h"

bool isAnArg = false;

enum Token {
tok_eof = -1,
tok_shader = -2,
tok_global = -3,
tok_local = -4,
tok_const = -5,
tok_temp = -6,
tok_float = -7,
tok_int = -8,
tok_color = -9,
tok_string = -10,
tok_read = -11,
tok_write = -12,
tok_openbraces = -13,
tok_closebraces = -14,
tok_identifier = -15,
tok_number = -16,
tok_param = -17,
tok_exit = -18,
tok_normal = -19,
tok_oparam = -20,
tok_closure = -21
};

static std::string hsail;
static std::string kernelName;
static std::string header;
static std::vector<std::string> argsType;
static std::vector<std::string> argsName;
static std::vector<std::string> argsDefault;
static std::string IdentifierStr;
static double NumVal;

static void MakeHSAKernel(){
hsail = "module &"+kernelName+":1:0:$full:$small:$default;\n\n";
hsail += "kernel &__HSA_"+kernelName+"(\n";
for(int i=0;i<argsName.size();i++){
	hsail += "\t"+argsType[i]+" "+argsName[i]+", " + argsDefault[i]+"\n";
}
hsail+=")\n{\n@__HSA_"+kernelName+"_entry:\n// BB#0: \n";
hsail+="};\n";
std::cout<< hsail<<std::endl;
}

static int gettok(){
	static int LastChar = ' ';
	while(isspace(LastChar))
		LastChar = getchar();

	if(isalpha(LastChar)){
		IdentifierStr = LastChar;
		while(isalnum(LastChar = getchar()))
			IdentifierStr += LastChar;
		std::cout<<"Just got: "<<IdentifierStr<<std::endl;

		if(IdentifierStr == "global")
			return tok_global;
		if(IdentifierStr == "local")
			return tok_local;
		if(IdentifierStr == "const")
			return tok_const;
		if(IdentifierStr == "temp")
			return tok_temp;
		if(IdentifierStr == "float")
			return tok_float;
		if(IdentifierStr == "int")
			return tok_int;
		if(IdentifierStr == "color")
			return tok_color;
		if(IdentifierStr == "string")
			return tok_string;
		if(IdentifierStr == "read")
			return tok_read;
		if(IdentifierStr == "write")
			return tok_write;
		if(IdentifierStr == "{")
			return tok_openbraces;
		if(IdentifierStr == "}")
			return tok_closebraces;
		if(IdentifierStr == "param")
			return tok_param;
		if(IdentifierStr == "shader")
			return tok_shader;
		if(IdentifierStr == "exit")
			return tok_exit;
		if(IdentifierStr == "oparam")
			return tok_oparam;
		return tok_identifier;
	}

	if(isdigit(LastChar) || LastChar == '.'){
		std::string NumStr;
		do {
			NumStr += LastChar;
			LastChar = getchar();
		} while (isdigit(LastChar) || LastChar == '.');

		NumVal = strtod(NumStr.c_str(), NULL);
		std::cout<<NumVal<<std::endl;
		return tok_number;
	}

	if(LastChar == '#'){
		do
			LastChar = getchar();
		while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if(LastChar != EOF)
			return gettok();
	}

	if(LastChar == EOF)
		return tok_eof;

	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;
}


namespace {

class ExprAST {
public:
	virtual ~ExprAST(){}
};

class NumberExprAST : public ExprAST{
	double Val;
public:
	NumberExprAST(double Val) : Val(Val) {}
};

class VariableExprAST : public ExprAST{
	std::string Name;
public:
	VariableExprAST(const std::string &Name) : Name(Name) {}
};

class GlobalExprAST : public ExprAST{
	std::string VarName;
	std::string Read1;
	std::string Read2;
	std::string Write1;
	std::string Write2;
public:
	GlobalExprAST(std::string &VarName,
			std::string &Read1,
			std::string &Read2,
			std::string &Write1,
			std::string &Write2): VarName(VarName),
						Read1(Read1),
						Read2(Read2),
						Write1(Write1),
						Write2(Write2) {}
};

static int CurTok;
static int getNextToken() {return CurTok = gettok();}

static int ParseGlobal(){
	std::cout<<IdentifierStr<<std::endl;
	getNextToken();
	return 1;
}

static int ParseConst(){
	std::cout<<IdentifierStr<<std::endl;
	getNextToken();
	return 1;
}

static int ParseTemp(){
	getNextToken();
	return 1;
}

static int ParseParam(){
	isAnArg = true;
	std::cout<<IdentifierStr<<std::endl;
	getNextToken();
	return 1;
}

static int ParseIdentifier(){
	std::cout<<"Parsing Identifier: "<<IdentifierStr<<std::endl;
	getNextToken();
	return 1;
}


static int ParseNumber(){
	std::cout<<IdentifierStr<<std::endl;
	std::cout<<"Value is: "<<NumVal<<std::endl;
	getNextToken();
	return 1;
}

static int ParseColor(){
	std::cout<<"In color: "<<IdentifierStr<<std::endl;
	if(isAnArg){
		std::cout<<"In isAnArg"<<std::endl;
		argsType.push_back("color");
		getNextToken();
		argsName.push_back(IdentifierStr);
		std::string Comm = "/*";
		for(int i=0;i<3;i++){
			getNextToken();
			Comm += std::to_string(NumVal) + " ";
		}
		Comm+="*/";
		argsDefault.push_back(Comm);
		isAnArg = false;
		std::cout<<"Exiting isAnArg"<<std::endl;
	}
	getNextToken();
	return 1;
}

static int ParseFloat(){
	if(isAnArg){
		argsType.push_back("float");
		getNextToken();
		argsName.push_back(IdentifierStr);
		getNextToken();
		argsDefault.push_back("/* " + std::to_string(NumVal) + " */");
		isAnArg = false;
	}
	getNextToken();
	return 1;
}

static int ParseShader(){
	std::string str = IdentifierStr;
	getNextToken();
	kernelName = IdentifierStr;
	getNextToken();
	return 1;
}

static void HandleGlobal(){
	if(ParseGlobal()){
		fprintf(stderr, "Parsed Global\n");
	}else{
		getNextToken();
	}
}

static void HandleConst(){
	if(ParseConst()){
		fprintf(stderr, "Parsed Const\n");
	}else{
		getNextToken();
	}
}


static void HandleTemp(){
	if(ParseTemp()){
		fprintf(stderr, "Parsed Temp\n");
	}else{
		getNextToken();
	}
}

static void HandleParam(){
	if(ParseParam()){
		fprintf(stderr, "Parsed Param\n");
	}else{
		getNextToken();
	}
}

static void HandleIdentifier(){
	if(ParseIdentifier()){
		fprintf(stderr, "Parsed Identifier\n");
	}else{
		getNextToken();
	}
}

static void HandleColor(){
	if(ParseColor()){
		fprintf(stderr, "Parsed Color\n");
	}else{
		getNextToken();
	}
}

static void HandleNumber(){
	if(ParseNumber()){
		fprintf(stderr, "Parsed Number\n");
	}else{
		getNextToken();
	}
}

static void HandleShader(){
	if(ParseShader()){
		fprintf(stderr, "Parsed Shader\n");
	}else{
		getNextToken();
	}
}

static void HandleFloat(){
	if(ParseFloat()){
		fprintf(stderr, "Parsed Float\n");
	}else{
		getNextToken();
	}
}

static void MainLoop(){
	while(1){
		fprintf(stderr, "ready> ");
		switch(CurTok){
		case tok_eof:
			return;
		case tok_global:
			HandleGlobal();
			break;
		case tok_const:
			HandleConst();
			break;
		case tok_temp:
			HandleTemp();
			break;
		case tok_param:
			HandleParam();
			break;
		case tok_identifier:
			HandleIdentifier();
			break;
		case tok_color:
			HandleColor();
			break;
		case tok_number:
			HandleNumber();
			break;
		case tok_shader:
			HandleShader();
			break;
		case tok_float:
			HandleFloat();
			break;
		case tok_exit:
			MakeHSAKernel();
			return;
		default:
//			MakeHSAKernel();
			return;
		}
	}
}

}

int main(){
	fprintf(stderr, "ready> ");
	getNextToken();
	MainLoop();
	return 0;
}

