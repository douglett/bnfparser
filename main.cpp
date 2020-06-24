#include <iostream>
#include "bnf_parser.hpp"
#include "source_parser.hpp"
using namespace std;

int test1() {
	BNFParser bnf;
	SourceParser src;
	//p.parseline("test := blah+~ bloo* | blob | (boob | flob)+");
	return bnf.parsefile("dbas.bnf")
		|| src.definelang(bnf.deflist)
		|| src.parsefile("test1.bas");
}

int main() {
	printf("start\n");
	test1();
}