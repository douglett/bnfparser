#pragma once
#include <string>
#include <map>
#include "ptools.hpp"


struct RuleRunner {
	typedef  std::string  string;
	typedef  ptools::Node  Node;
	std::map<string, Node> rules;

	int define(const Node& deflist) {
		for (auto& d : deflist.kids)
			if (d.type != "define" || d.kids.size() != 1)
				return fprintf(stderr, "define error: %s [%s]\n", d.type.c_str(), d.val.c_str()), 1;
			else if (rules.count(d.val))
				return fprintf(stderr, "duplicate rule: %s\n", d.val.c_str()), 1;
			else
				rules[d.val] = d.kids.at(0);
		return 0;
	}
};