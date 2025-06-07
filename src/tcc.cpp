#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include <map>
using namespace llvm;

// int read_var(char* s)
// {
// 	char buf[64];
// 	int val;
// 	printf("enter value for %s",s);
// 	fgets(buf,sizeof(buf),stdin);
// 	if(EOF == sscanf(buf,"%d",&val))
// 	{
// 		printf("Value %s is invalid\n",buf);
// 	}
// 	return val;
// }

// void print_val(int v)
// {
// 	printf("The result is: %d\n",v);
// }

// void print_str(char* s)
// {
// 	printf("%s",s);
// }

void read_operands(std::string s,char op,std::string& s1,std::string&s2, std::string& s3)
{
        std::size_t pos1 = s.find('=');
        std::size_t pos2 = s.find(op);
        s1 = s.substr(0, pos1);
        s2 = s.substr(s.find('=')+1,pos2-pos1-1);
        s3 = s.substr(pos2+1);
}

void generate_arith_instruction(std::string line, 
	char op,
	std::map<std::string,Value*>& VarNameToValue,
	IRBuilder<>& Builder)
{
		std::string s_strRef1,s_strRef2,s_strRef3;
		read_operands(line,op,s_strRef1,s_strRef2,s_strRef3);
		llvm::StringRef strRef1(s_strRef1);
		llvm::StringRef strRef2(s_strRef2);
		llvm::StringRef strRef3(s_strRef3);
		strRef1 = strRef1.trim(' ');
		strRef2 = strRef2.trim(' ');	
		strRef3 = strRef3.trim(' ');

		s_strRef1 = strRef1.str();
		s_strRef2 = strRef2.str();
		s_strRef3 = strRef3.str();

		if(	VarNameToValue.find(s_strRef1) != VarNameToValue.end() &&
			VarNameToValue.find(s_strRef2) != VarNameToValue.end() &&
			VarNameToValue.find(s_strRef3) != VarNameToValue.end())
		{
			Value* V1 = VarNameToValue[s_strRef1];
			Value* V2 = VarNameToValue[s_strRef2];
			Value* V3 = VarNameToValue[s_strRef3];
			
			if(op == '+')
				V1 = Builder.CreateNSWAdd(V2, V3);
			if(op == '-')
				V1 = Builder.CreateNSWSub(V2, V3);
			if(op == '*')
				V1 = Builder.CreateNSWMul(V2, V3);
			if(op == '/')
				V1 = Builder.CreateSDiv(V2, V3);

			VarNameToValue[s_strRef1] = V1;
		}
}

int main(int argc, const char **argv)
{
	if(argc != 2)
	{
		fprintf (stderr, "no input files\n");
		exit(1);
	}
	std::string i_file_name = argv[1];
	LLVMContext Ctx;
	Module* M = new Module(i_file_name,Ctx);
	IRBuilder<> Builder(M->getContext());
	Value* V;
	std::map<std::string,Value*> VarNameToValue;
	StringMap<GlobalVariable*> VarNameToGlobalName;
	std::ifstream myfile;
	std::string line;
	int InstNum;

	Type *VoidType = Type::getVoidTy(M->getContext());
	Type *Int32Type = Type::getInt32Ty(M->getContext());
	PointerType *PtrType = PointerType::getUnqual(M->getContext());
	Constant *Int32Zero = ConstantInt::get(Int32Type,0,true);

	FunctionType *MainFtype = FunctionType::get(Int32Type, {Int32Type,PtrType},false);
	Function *MainFn = Function::Create(MainFtype,GlobalValue::ExternalLinkage,"main",M);

	BasicBlock* BB = BasicBlock::Create(M->getContext(),"entry",MainFn);
	Builder.SetInsertPoint(BB);

	FunctionType *ReadFtype = FunctionType::get(Int32Type,{PtrType},false);
	Function *ReadFn = Function::Create(ReadFtype,GlobalValue::ExternalLinkage,"read_var",M);
	
	FunctionType *WriteFtype = FunctionType::get(VoidType,{Int32Type},false);
	Function *WriteFn = Function::Create(WriteFtype,GlobalValue::ExternalLinkage,"print_val",M);

	FunctionType *PrintStrType = FunctionType::get(VoidType,{PtrType},false);
	Function *PrintStrFn = Function::Create(PrintStrType,GlobalValue::ExternalLinkage,"print_str",M);

	InstNum = 0;
	
	myfile.open(i_file_name);
	while(std::getline(myfile,line))
	{
		InstNum++;
		llvm::StringRef s(line);
		if(s.starts_with("VAR "))
		{
			StringRef VarName = s.substr(4);
			Constant *StrText = ConstantDataArray::getString(M->getContext(), VarName);
      			GlobalVariable *Str = new GlobalVariable(*M, 
					StrText->getType(),
					true, 
					GlobalValue::PrivateLinkage,
          				StrText, 
					Twine(VarName).concat(".str"));
			
			VarNameToGlobalName[VarName] = Str;
		}
		else if(s.starts_with("READ "))
		{
			StringRef VarName = s.substr(5);
			if(VarNameToGlobalName.contains(VarName))
			{
			 	GlobalVariable *Str = VarNameToGlobalName[VarName];
				CallInst* Call = Builder.CreateCall(ReadFtype,ReadFn,{Str});
				std::string strVarName = VarName.str();
				VarNameToValue.insert(std::map<std::string,Value*>::value_type(strVarName, Call));
			}

		}
		else if(s.starts_with("WRITE "))
		{
			StringRef VarName = s.substr(6);
			if(VarNameToGlobalName.contains(VarName))
			{
				GlobalVariable* Str = VarNameToGlobalName[VarName];
				std::string strVarName = VarName.str();
				if(VarNameToValue.find(strVarName) != VarNameToValue.end())
				{
					Value* Val = VarNameToValue[strVarName];
					Builder.CreateCall(WriteFtype, WriteFn,{Val});
				}
			}
		}
		else if(s.starts_with("PRINT "))
		{
			StringRef VarName = s.substr(6);
			Constant *StrText = ConstantDataArray::getString(M->getContext(), VarName);
      			GlobalVariable *Str = new GlobalVariable(*M, 
					StrText->getType(),
					true, 
					GlobalValue::PrivateLinkage,
          				StrText, 
					Twine(VarName).concat(".str").concat(std::to_string(InstNum)));
			Builder.CreateCall(PrintStrType, PrintStrFn,{Str});
		}
		else if(s.contains("+"))
		{
			generate_arith_instruction(line,'+',VarNameToValue,Builder);
		}
		else if(s.contains("-"))
		{
			generate_arith_instruction(line,'-',VarNameToValue,Builder);
		}
		else if(s.contains("*"))
		{
			generate_arith_instruction(line,'*',VarNameToValue,Builder);
		}
		else if(s.contains("/"))
		{
			generate_arith_instruction(line,'/',VarNameToValue,Builder);
		}
	}
	Builder.CreateRet(Int32Zero);	
	M->print(outs(),nullptr);

	return 0;

}

