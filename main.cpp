#include <iostream>
#include "rule_parser.hpp"
#include "rule_runner.hpp"
using namespace std;

int test1() {
	RuleParser p;
	RuleRunner r;
	//p.parseline("test := blah+~ bloo* | blob | (boob | flob)+");
	return p.parsefile("dbas.bnf")
		|| r.define(p.deflist)
		|| r.runfile("test1.bas");
}

int main() {
	printf("start\n");
	test1();
}