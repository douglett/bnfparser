#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "ptools.hpp"


struct BNFParser {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	int lcount = 0;
	std::stringstream input;
	Node deflist;

	// parse a BNF definition file into a lisp-ish representation of the parsing rules
	int parsefile(string fname) {
		// load file
		std::fstream fs(fname, std::ios::in);
		if (!fs.is_open())
			return fprintf(stderr, "error: could not open file: %s\n", fname.c_str()), 1;
		input.str(""), input.seekg(0);
		input << fs.rdbuf();
		fs.close();
		// parse
		deflist = { "def-list" }, lcount = 1; // reset state
		int errcode = 0;
		try {
			while (!errcode)
				if      (input.peek() == EOF) break;
				else if (emptyline()) lcount++;
				else if (defrule(deflist)) lcount++;
				else    errcode = 1;
		} catch (Node err) {
			errcode = 1;
			std::cerr << err.show();
		}
		//std::cout << deflist.show();
		return errcode;
	}

	// parse a single line rule (useful for testing)
	int parseline(string ln) {
		input.str(ln), input.seekg(0);
		deflist = { "rule-list" }, lcount = 1;
		int errcode = 0;
		try {
			if (!defrule(deflist)) errcode = 1;
		} catch (Node err) {
			errcode = 1;
			std::cerr << err.show();
		}
		//std::cout << deflist.show();
		return errcode;
	}


private:
	// helpers
	int doerr(string name, string msg="") {
		Node n = { "error", "bnf-parser", {
			{ "rule", name },
			{ "message", msg },
			{ "line", std::to_string(lcount) },
			{ "at-char", string(1, input.peek()) }
		}};
		throw n;
		return 0;
	}

	// main structure parsing
	int emptyline() {
		wspace();
		if (input.peek() == '#') // check for line comments. parse out but exclude
			while (!ptools::endline(input.peek())) input.get();
		return endline();
	}

	int defrule(Node& res) {
		string id;
		auto& n = res.push({ "define" });
		if (!identifier(id)) goto err;
		// TODO: built-in rules warning
		wspace();
		if      (getstr(":=")) n = { "define", id };
		else if (getstr("$=")) n = { "define-str", id };
		else    goto err;
		wspace();
		if (!getor(n)) goto err;
		if (!endline()) goto err;
		return 1;
		err:
		return doerr("def-rule");
	}

	int getor(Node& res) {
		if (!getand(res)) goto err;
		if (input.peek() == '|') {
			auto& n = res.back();
			n = { "|", "", { n } };
			while (input.peek() == '|') {
				input.get();
				if (!getand(n)) goto err;
			}
		}
		return 1;
		err:
		return doerr("get-or");
	}

	int getand(Node& res) {
		auto& n = res.push({ "&" });
		while (modifier(n)) ;
		if (n.kids.size() == 0) doerr("get-and", "rule expected");
		return 1;
	}

	int modifier(Node& res) {
		string s;
		char c;
		if (!atom(res)) return 0;
		// negation operator
		c = input.peek();
		if (c == '!')
			res.back() = { string(1, input.get()), "", { res.back() } };
		// multiples operators
		c = input.peek();
		if (c == '*' || c == '+' || c == '?')
			res.back() = { string(1, input.get()), "", { res.back() } };
		// exclude operator
		c = input.peek();
		if (c == '~')
			res.back() = { string(1, input.get()), "", { res.back() } };
		return 1;
	}

	int atom(Node& res) {
		string s;
		wspace();
		// TODO: control rules
		if (identifier(s)) return res.push({ "rule", s }), 1;
		if (strlit(s)) return res.push({ "literal", s }), 1;
		if (brackets(res)) return 1;
		return 0;
	}

	int brackets(Node& res) {
		if (input.peek() != '(') return 0;
		input.get();
		auto& n = res.push({ "()" });
		if (!getor(n)) goto err;
		wspace();
		if (input.peek() != ')') goto err;
		input.get();
		return 1;
		err:
		return doerr("brackets");
	}

	// basic parsing
	int getstr(const string& s) {
		const int p = input.tellg();
		for (auto c : s)
			if (input.get() != c)
				return input.seekg(p), 0;
		return 1;
	}

	int identifier(string& s) {
		s = "";
		if (!(ptools::alpha(input.peek()) || input.peek()=='_')) return 0;
		s += char(input.get());
		while (ptools::alpha(input.peek()) || ptools::numeral(input.peek()) || input.peek()=='_')
			s += input.get();
		return 1;
	}

	int strlit(string& s) {
		s = "";
		if (input.peek() != '"') return 0;
		s += char(input.get());
		while (input.peek() != EOF) {
			if (endline()) break;
			s += input.get();
			if (s.back() == '"') return 1;
		}
		return doerr("strlit", "unexpected end of string");
	}

	// int modoperator(string& s) {
	// 	s = "";
	// 	char c = input.peek();
	// 	if (c == '*' || c == '+' || c == '?' || c == '~')
	// 		s += input.get();
	// 	return s.length() > 0;
	// }

	int wspace() {
		string s;
		while (ptools::wspace(input.peek()))
			s += char(input.get());
		return s.length() > 0;
	}

	int endline() {
		if (ptools::endline(input.peek()))
			return input.get(), 1;
		return 0;
	}
};