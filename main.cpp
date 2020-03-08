#include <iostream>
#include "rule_parser.hpp"
#include "rule_runner.hpp"
using namespace std;

int main() {
	printf("start\n");
	RuleParser p;
	//p.parseline("test := blah+~ bloo* | blob | (boob | flob)+");
	p.parsefile("dbas.bnf");
//	RuleRunner r;
//	r.define(p.deflist);
//	r.runfile("test1.bas", "line");
}