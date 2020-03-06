#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "ptools.hpp"


struct RuleParser {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	int lcount = 0;
	std::stringstream input;
	Node deflist;

	// parser entry
	int parsefile(string fname) {
		std::fstream fs(fname, std::ios::in);
		if (!fs.is_open())
			return fprintf(stderr, "error: could not open file: %s\n", fname.c_str()), 1;
		deflist = { "def-list" }, lcount = 0;
		int errcode = 0;
		try {
			string s;
			while (getline(fs, s)) {
				input = std::stringstream(s);
				lcount++;
				if (!defrule(deflist)) { errcode = 1; break; }
			}
		} catch(string err) {
			errcode = 1;
			fprintf(stderr, "error: %s\n", err.c_str());
		}
		shownode(deflist);
		return errcode;
	}

	int parseline(string ln) {
		input.str(ln), input.seekg(0);
		deflist = { "rule-list" }, lcount = 1;
		int ok = 0;
		try { ok = defrule(deflist); }
		catch (string err) { std::cerr << "error: " << err << std::endl; }
		shownode(deflist);
		return ok;
	}

	// helpers
	int doerr(string name, string msg="") {
		throw string("error in "+name+", line "+std::to_string(lcount));
		return 0;
	}

	void shownode(const Node& n, const int ind=0) {
		std::cout << string(ind*2, ' ') << n.type << (n.val.length() ? " ["+n.val+"]" : "") << std::endl;
		for (auto& nn : n.kids)
			shownode(nn, ind+1);
	}

	Node& pushn(Node& parent, const Node& child) {
		parent.kids.push_back(child);
		return parent.kids.back();
	}

	Node& backn(Node& parent) {
		if (!parent.kids.size()) doerr("backn", "out of range");
		return parent.kids.back();
	}

	// main structure parsing
	int defrule(Node& res) {
		//res.kids.push_back({ "define" });
		auto& n = pushn(res, { "define" });
		if (!identifier(n.val)) goto err;
		wspace();
		if (!getstr(":=")) goto err;
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
			auto& n = backn(res);
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
		auto& n = pushn(res, { "&" });
		while (modifier(n)) ;
		if (n.kids.size() == 0) doerr("get-and", "rule expected");
		return 1;
	}

	int modifier(Node& res) {
		string s;
		char c;
		if (!atom(res)) return 0;
		// multiples operators
		c = input.peek();
		if (c == '*' || c == '+' || c == '?')
			backn(res) = { string(1, input.get()), "", { backn(res) } };
		// exclude operator
		c = input.peek();
		if (c == '~')
			backn(res) = { string(1, input.get()), "", { backn(res) } };
		return 1;
	}

	int atom(Node& res) {
		string s;
		wspace();
		if (identifier(s)) return pushn(res, { "rule", s }), 1;
		if (brackets(res)) return 1;
		return 0;
	}

	int brackets(Node& res) {
		if (input.peek() != '(') return 0;
		input.get();
		auto& n = pushn(res, { "()" });
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
		if (!ptools::alpha(input.peek())) return 0;
		s += char(input.get());
		while (ptools::alpha(input.peek()) || ptools::numeral(input.peek()))
			s += input.get();
		return 1;
	}

	int modoperator(string& s) {
		s = "";
		char c = input.peek();
		if (c == '*' || c == '+' || c == '?' || c == '~')
			s += input.get();
		return s.length() > 0;
	}

	int wspace() {
		string s;
		while (ptools::wspace(input.peek()))
			s += char(input.get());
		return s.length() > 0;
	}

	int endline() {
		if (input.peek() == '\n' || input.peek() == EOF)
			return 1;
		return 0;
	}
};