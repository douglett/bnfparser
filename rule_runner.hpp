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
		try { ok = dorulen(rulename, prog); }
		catch (Node err) { std::cerr << ptools::shown(err); }
		input.close();
		std::cout << ptools::shown(prog);
		return !ok; // returns 1 on error
	}

	// helpers
	int doerr(const string& name, const string& msg) {
		Node e = { "error", "", {
			{ "rule", name },
			{ "message", msg }
		}};
		throw e;
		return 0;
	}

	// run rules
	int dorulen(const string& name, Node& res) {
		// user defined rules
		if (rulelist.count(name)) {
			const auto& rule = rulelist[name];
			res.kids.push_back({ name });
			return dorulen(name, rule, res.kids.back());
		}
		//
		string s;
		if (dorules(name, s)) {
			res.kids.push_back({ name, s });
			return 1;
		}
		return 0;
		//return doerr(name, "missing rule");
	}

	int dorulen(const string& name, const Node& rule, Node& res) {
		if (rule.type == "&") {
			for (auto& r : rule.kids)
				if (dorulen(name, r, res)) ;
				else return 0;
			return 1;
		}
		if (rule.type == "*") {
			while (dorulen(name, rule.kids.at(0), res)) ;
			return 1;
		}
		if (rule.type == "rule")
			return dorulen(rule.val, res);
		return doerr(name, "unknown rule type: ["+rule.type+"]");
	}

	int dorules(const string& name, string& res) {
		using namespace ptools;
		// built-in rules
		if (name == "ucase"   ) return ucase(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "lcase"   ) return lcase(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alpha"   ) return alpha(input.peek()) ? (res += input.get(), 1) : 0;
//		if (name == "numeral" ) return numeral(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alphanum") return alphanum(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "endl"    ) return input.peek() == '\n' || input.peek() == EOF ? (res += input.get(), 1) : 0;
		if (name == "EOF"     ) return input.peek() == EOF ? (res += input.get(), 1) : 0;

		return doerr(name, "missing rule");
	}
};