#pragma once

#include <string>
using std::string;

#include <vector>
using std::vector;

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");
