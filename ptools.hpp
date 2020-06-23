#pragma once
#include <string>
#include <vector>


namespace ptools {
	typedef  std::string  string;

	// helpful parsing info
	int ucase(char c) { return c >= 'A' && c <= 'Z'; }
	int lcase(char c) { return c >= 'a' && c <= 'z'; }
	int alpha(char c) { return ucase(c) || lcase(c); }
	int numeral(char c) { return c >= '0' && c <= '9'; }
	int alphanum(char c) { return alpha(c) || numeral(c); }
	int wspace(char c) { return c == ' ' || c == '\t'; }
	int endline(char c) { return c == '\n' || c == EOF; }

	// Node struct
	struct Node {
		// data
		std::string type, val;
		std::vector<Node> kids;
		// methods
		string show(const int ind=0);
	};

	// Node functions
	string Node::show(const int ind) {
		string s = string(ind*2, ' ')+type+": "+val+"\n";
		for (auto& nn : kids)
			s += nn.show(ind+1);
		return s;
	}

	// node tools
//	string shown(const Node& n, const int ind=0) {
//		//std::string s = std::string(ind*2, ' ') + n.type + (n.val.length() ? " ["+n.val+"]" : "") + "\n";
//		string s = string(ind*2, ' ')+n.type+": "+n.val+"\n";
//		for (auto& nn : n.kids)
//			s += shown(nn, ind+1);
//		return s;
//	}

	//string showc

//	Node& pushn(Node& parent, const Node& child) {
//		parent.kids.push_back(child);
//		return parent.kids.back();
//	}
//
//	Node& backn(Node& parent) {
//		if (!parent.kids.size()) doerr("backn", "out of range");
//		return parent.kids.back();
//	}
}