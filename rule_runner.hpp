#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "ptools.hpp"


struct RuleRunner {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	std::map<string, Node> rulelist, ruleliststr;
	std::stringstream input;
	Node prog;

	// program entry
	int define(const Node& deflist) {
		for (auto& d : deflist.kids)
			// TODO: logical structure
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

	// runtime entry
	int runfile(const string& fname, const string& rulename="prog") {
		// load file
		std::fstream fs(fname, std::ios::in);
		if (!fs.is_open())
			return fprintf(stderr, "could not open file: %s\n", fname.c_str()), 1;
		input.str(""), input.clear();
		input << fs.rdbuf();
		fs.close();
		//
		prog = { "result" };
		int errcode = 0;
		try {
			if (!dorule(rulename, prog))
				errcode = 1;
		}
		catch (Node err) {
			errcode = 1;
			std::cerr << err.show();
		}
		std::cout << prog.show();
		return errcode; // returns 1 on error
	}

	int runline(const string& s) {
		// test stuff here
		return 0;
	}

	int getlineno() {
		const int p = input.tellg();
		int lcount = 0;
		input.seekg(0);
		while (input.tellg() <= p && input.peek() != EOF)
			if (input.get() == '\n') lcount++;
		input.seekg(p);
		return lcount;
	}

	// helpers
	int doerr(const string& name, const string& msg) {
		Node e = { "error", "rule-runner", {
			{ "rule", name },
			{ "message", msg },
			{ "line", std::to_string(getlineno() + 1) },
			{ "at-char", string(1, input.peek()) }
		}};
		throw e;
		return 0;
	}

	// run rules
	int dorule(const string& name, Node& res) {
		//printf("rule: %s\n", name.c_str());
		wspace(); // clear whitespace before rule
		// user defined rules
		if (rulelist.count(name)) {
			const auto& rule = rulelist[name];
			res.push({ name });
			if (dosubrule( name, rule, res.back() )) return 1;
			return res.pop(), 0; // fail - remove from list
		}
		// string rules
		string s;
		if (dorulestr(name, s))
			return res.push({ name, s }), 1;
		//return doerr("dorule", "unexpected error"); // should never happen...
		// string rule not matched
		return 0;
	}

	int dosubrule(const string& name, const Node& rule, Node& res) {
		wspace(); // clear whitespace between sub-rules
		const int p = input.tellg(); // save fail position
		if (rule.type == "&") {
			int lock = 0;
			for (auto& r : rule.kids)
				if      (r.val == "LOCK") lock = 1;
				else if (dosubrule(name, r, res)) ;
				else if (lock) return doerr(name, "rule missing after lock: "+r.type+", "+r.val);
				else    return input.seekg(p), 0;
			return 1;
		}
		if (rule.type == "|") {
			for (auto& r : rule.kids)
				if   (dosubrule(name, r, res)) return 1;
				else input.seekg(p);
			return 0;
		}
		if (rule.type == "*") {
			while (dosubrule(name, rule.kids.at(0), res) && !eof()) ; // don't match forever
			return 1;
		}
		if (rule.type == "~") { // parse but exclude result
			Node tmp;
			return dosubrule(name, rule.kids.at(0), tmp);
		}
		if (rule.type == "()")
			return dosubrule(name, rule.kids.at(0), res);
		if (rule.type == "literal") {
			string tmp;
			if (getliteral(rule.val, tmp))
				return res.push({ rule.val, tmp }), 1;
			return 0;
		}
		if (rule.type == "rule")
			return dorule(rule.val, res);
		return doerr(name, "unknown rule type: ["+rule.type+"]");
	}

	// run string rules
	int dorulestr(const string& name, string& res) {
		using namespace ptools;
		// built-in string rules
		if (name == "ucase"   ) return ucase(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "lcase"   ) return lcase(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alpha"   ) return alpha(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "numeral" ) return numeral(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "alphanum") return alphanum(input.peek()) ? (res += input.get(), 1) : 0;
		if (name == "endl"    ) return endline(input.peek()) ? (res += unescape(input.get()), 1) : 0;
		if (name == "EOF"     ) return eof() ? (res += unescape(input.get()), 1) : 0;
		// user defined string rules
		if (ruleliststr.count(name))
			return dosubrulestr(name, ruleliststr[name], res);
		// rule not found anywhere
		return doerr(name, "missing rule");
	}

	// warning: more code reuse issues
	int dosubrulestr(const string& name, const Node& rule, string& res) {
		const int p = input.tellg();
		string tmp;
		if (rule.type == "&") {
			for (auto& r : rule.kids)
				if   (dosubrulestr(name, r, tmp)) ;
				else return input.seekg(p), 0;
			return res += tmp, 1;
		}
		if (rule.type == "|") {
			for (auto& r : rule.kids)
				if   (dosubrulestr(name, r, tmp)) return res += tmp, 1;
				else input.seekg(p), tmp = "";
			return 0;
		}
		if (rule.type == "*") {
			while (dosubrulestr(name, rule.kids.at(0), res) && !eof()) ;
			return 1;
		}
		if (rule.type == "+") {
			if (!dosubrulestr(name, rule.kids.at(0), res)) return 0;
			while (!eof() && dosubrulestr(name, rule.kids.at(0), res)) ;
			return 1;
		}
		if (rule.type == "()")
			return dosubrulestr(name, rule.kids.at(0), res);
		if (rule.type == "literal")
			return getliteral(rule.val, res);
		if (rule.type == "rule")
			return dorulestr(rule.val, res);
		return doerr(name, "unknown string-rule type: ["+rule.type+"]");
	}

	// general parsing helpers
	int wspace() {
		string s;
		while (ptools::wspace(input.peek())) s += input.get();
		return s.length() > 0;
	}

	int eof() {
		return input.peek() == EOF;
	}

	int getliteral(const string& lit, string& res) {
		const int p = input.tellg();
		const auto val = lit.substr(1, lit.size()-2);
		for (auto c : val)
			if (c != input.get())
				return input.seekg(p), 0;
		return res += val, 1;
	}

	string unescape(char c) {
		switch (c) {
			case '\n':  return "\\n";
			case  EOF:  return "\\0"; // this is wrong actually
			default:    return string(1, c);
		}
	}
};