#pragma once
#include <string>
#include <vector>


namespace ptools {
	struct Node {
		std::string type, val;
		std::vector<Node> kids;
	};

	int ucase(char c) { return c >= 'A' && c <= 'Z'; }
	int lcase(char c) { return c >= 'a' && c <= 'z'; }
	int alpha(char c) { return ucase(c) || lcase(c); }
	int numeral(char c) { return c >= '0' && c <= '9'; }
	int alphanum(char c) { return alpha(c) || numeral(c); }
	int wspace(char c) { return c == ' ' || c == '\n'; }

	std::string shownode(const Node& n, const int ind=0) {
		std::string s = std::string(ind*2, ' ') + n.type + (n.val.length() ? " ["+n.val+"]" : "") + "\n";
		for (auto& nn : n.kids)
			s += shownode(nn, ind+1);
		return s;
	}
}