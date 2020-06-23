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
		Node& push(const Node& child);
		Node  pop();
		Node& back();
	};

	// Node function definitions
	string Node::show(const int ind) {
		string s = string(ind*2, ' ')+type+": "+val+"\n";
		for (auto& nn : kids)
			s += nn.show(ind+1);
		return s;
	}
	Node& Node::push(const Node& child) {
		kids.push_back(child);
		return kids.back();
	}
	Node  Node::pop() {
		auto n = kids.back(); // range check here
		return kids.pop_back(), n;
	}
	Node& Node::back() {
		if (!kids.size()) fprintf(stderr, "Node: back() from empty node list"), exit(1);
		return kids.back();
	}
}