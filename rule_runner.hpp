#pragma once
#include <string>
#include <map>
#include <fstream>
#include "ptools.hpp"


struct RuleRunner {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	std::map<string, Node> rulelist, ruleliststr;
	std::fstream input;
	Node prog;

	// program entry
	int define(const Node& deflist) {
		for (auto& d : deflist.kids)
			if (!(d.type == "define" || d.type == "define-str") || d.kids.size() != 1)
				return fprintf(stderr, "define error: %s [%s]\n", d.type.c_str(), d.val.c_str()), 1;
			else if (rulelist.count(d.val))
				return fprintf(stderr, "duplicate rule: %s\n", d.val.c_str()), 1;
			else if (d.type == "define")
				rulelist[d.val] = d.kids.at(0);
			else if (d.type == "define-str")
				ruleliststr[d.val] = d.kids.at(0);
		return 0;
	}

	int runfile(const string& fname, const string& rulename="prog") {
		input.open(fname, std::ios::in);
		if (!input.is_open())
			return fprintf(stderr, "could not open file: %s\n", fname.c_str()), 1;
		int ok = 0;
		prog = { "prog" };
		try { ok = dorule(rulename, prog); }
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

	// TODO: should be reusable
	Node& pushn(Node& parent, const Node& child) {
		parent.kids.push_back(child);
		return parent.kids.back();
	}
	Node& backn(Node& parent) {
		if (!parent.kids.size()) doerr("backn", "out of range");
		return parent.kids.back();
	}

	// run rules
	int dorule(const string& name, Node& res) {
		// user defined rules
		if (rulelist.count(name)) {
			const auto& rule = rulelist[name];
			pushn(res, { name });
			return dorule(name, rule, backn(res));
		}
		// string rules
		string s;
		if (dorulestr(name, s))
			return pushn(res, { name, s }), 1;
		return 0;
	}

	int dorule(const string& name, const Node& rule, Node& res) {
		const int p = input.tellg();
		if (rule.type == "&") {
			for (auto& r : rule.kids)
				if (dorule(name, r, res)) ;
				else return input.seekg(p), 0;
			return 1;
		}
		if (rule.type == "*") {
			while (dorule(name, rule.kids.at(0), res)) ;
			return 1;
		}
		if (rule.type == "rule")
			return dorule(rule.val, res);
		return doerr(name, "unknown rule type: ["+rule.type+"]");
	}

	int dorulestr(const string& name, string& res) {
		using namespace ptools;
		// built-in string rules
//		if (name == "ucase"   ) return ucase(input.peek()) ? (res += input.get(), 1) : 0;
//		if (name == "lcase"   ) return lcase(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alpha"   ) return alpha(input.peek()) ? (res += input.get(), 1) : 0;
//		if (name == "numeral" ) return numeral(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alphanum") return alphanum(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "endl"    ) return input.peek() == '\n' || input.peek() == EOF ? (res += input.get(), 1) : 0;
//		if (name == "EOF"     ) return input.peek() == EOF ? (res += input.get(), 1) : 0;

		// user defined string rules
		if (ruleliststr.count(name)) {
			return dorulestr(name, ruleliststr[name], res);
		}
		return doerr(name, "missing rule");
	}

	// warning: more code reuse issues
	int dorulestr(const string& name, const Node& rule, string& res) {
		const int p = input.tellg();
		if (rule.type == "&") {
			for (auto& r : rule.kids)
				if (dorulestr(name, r, res)) ;
				else return input.seekg(p), 0;
			return 1;
		}
		if (rule.type == "*") {
			while (dorulestr(name, rule.kids.at(0), res)) ;
			return 1;
		}
		if (rule.type == "rule")
			return dorulestr(rule.val, res);
		return doerr(name, "unknown string-rule type: ["+rule.type+"]");
	}
};