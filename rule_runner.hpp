#pragma once
#include <string>
#include <map>
#include <fstream>
#include "ptools.hpp"


struct RuleRunner {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	std::map<string, Node> rulelist;
	std::fstream input;
	Node prog;

	// program entry
	int define(const Node& deflist) {
		for (auto& d : deflist.kids)
			if (d.type != "define" || d.kids.size() != 1)
				return fprintf(stderr, "define error: %s [%s]\n", d.type.c_str(), d.val.c_str()), 1;
			else if (rulelist.count(d.val))
				return fprintf(stderr, "duplicate rule: %s\n", d.val.c_str()), 1;
			else
				rulelist[d.val] = d.kids.at(0);
		return 0;
	}

	int runfile(const string& fname, const string& rulename="prog") {
		input.open(fname, std::ios::in);
		if (!input.is_open())
			return fprintf(stderr, "could not open file: %s\n", fname.c_str()), 1;
		int ok = 0;
		prog = { "prog" };
		try { ok = runrule(rulename, prog); }
		catch (Node err) { std::cerr << ptools::shownode(err); }
		input.close();
		return !ok; // returns 1 on error
	}

	// helpers
	int doerr(const string& name, const string& msg) {
		Node e = { "error", name, {
			{ "message", msg }
		}};
		throw e;
	}

	// run rules
	int runrule(const string& name, Node& res) {
		if (!rulelist.count(name)) doerr(name, "missing rule");
		const auto& rule = rulelist[name];
		std::cout << "here: " << ptools::shownode(rule);
		return 0;
	}

	int runrule(const string& name, const Node& rule, Node& res) {
		
		return 0;
	}
};