#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

namespace ptools {
	int ucase(char c) { return c >= 'A' && c <= 'Z'; }
	int lcase(char c) { return c >= 'a' && c <= 'z'; }
	int alpha(char c) { return ucase(c) || lcase(c); }
	int numeral(char c) { return c >= '0' && c <= '9'; }
	int alphanum(char c) { return alpha(c) || numeral(c); }
	int wspace(char c) { return c == ' ' || c == '\n'; }
}

struct Node {
	std::string type, val;
	std::vector<Node> kids;
};

struct RuleParser {
	typedef  std::string  string;
	std::stringstream input;

	int parseline(string ln) {
		input.str(ln), input.seekg(0);
		Node n = { "rule-list" };
		int ok = 0;
		try { ok = defrule(n); }
		catch (string err) { cerr << "error: " << err << endl; }
		shownode(n);
		return ok;
	}

	int doerr(string name) {
		throw string("error in "+name+", line 1");
		return 0;
	}

	void shownode(const Node& n, const int ind=0) {
		cout <<	string(ind*2, ' ') << n.type << (n.val.length() ? " ["+n.val+"]" : "") << endl;
		for (auto& nn : n.kids)
			shownode(nn, ind+1);
	}

	Node& pushn(Node& parent, const Node& child) {
		parent.kids.push_back(child);
		return parent.kids.back();
	}

	int defrule(Node& res) {
		res.kids.push_back({ "define" });
		auto& n = res.kids.back();
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
			auto& n = res.kids.back();
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
//		string s;
//		wspace();
//		while (identifier(s))
//			pushn(n, { "rule", s }), wspace();
		while (modifier(n)) ;
		return 1;
	}

	int modifier(Node& res) {
		string s;
		if (!atom(res)) return 0;
		while (modoperator(s)) {
			auto& n = res.kids.back();
			n = { s, "", { n } };
		}
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
		pushn(res, { "()" });
		if (!getor(res.kids.back())) goto err;
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


int main() {
	printf("start\n");
	RuleParser p;
	p.parseline("test := blah+~ bloo* | blob | (boob | flob)+");
}