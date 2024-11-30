#include <errno.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/time.h>

static const std::string deci = "0123456789";
static const std::string hexa = "0123456789ABCDEF";

char	to_upper(unsigned char c)
{
	return (std::toupper(c));
}

int	getInt(std::string str, int base, int &res)
{
	std::transform(str.begin(), str.end(), str.begin(), to_upper);
	std::string	sbase;
	const char	*p;
	std::string::size_type idx = 0;
	switch (base){
		case 10:
			sbase = deci;
			break ;
		case 16:
			sbase = hexa;
			break ;
	}
	while (str[idx] == 32 || str[idx] == 9)
		idx++;
	if (str[idx] == '+' || str[idx] == '-')
		idx++;
	if (idx == str.size())
		return (1);
	if (str.find_first_not_of(sbase, idx) != std::string::npos)
		return (1);
	p = str.c_str();
	errno = 0;
	res = strtol(p, NULL, base);
	if (errno)
		return (1);
	return (0);
}


std::string	IntToString(int x, int base)
{
	std::ostringstream ss;
	if (base == 16)
		ss << std::hex;
	ss << x;
	return (ss.str());
}

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool nocase_string_eq(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}


std::string	generate_name(const std::string* hostname)
{
	struct timeval	t;
	long long		time;
	std::ostringstream	filename;

	gettimeofday(&t, NULL);
	time = t.tv_sec * 1000000 + t.tv_usec;
	filename << "./tmp/";
	if (hostname)
		filename << *hostname;
	filename << "_";
	filename << time;
	return (filename.str());
}

static void	tokenize(const std::string& expression, std::vector<std::string>& tokenVec)
{
	int	i = 0;

	while (expression[i]) {
		if (expression[i] == '*') {
			tokenVec.push_back("*");
			i++;
		}
		else {
			int begin = i;
			while (expression[i] && expression[i] != '*')
				i++;
			tokenVec.push_back(expression.substr(begin, i - begin));
		}
	}
}

static bool	isMatch(const std::string& uri, std::vector<std::string>& tokenVec)
{
	size_t	i = 0;

	std::vector<std::string>::iterator it = tokenVec.begin();
	if (it == tokenVec.end())
		return (false);
	while (uri[i]) {
		if (*it == "*") {
			if (++it == tokenVec.end())
				return (true);
			if ((i = uri.find(*it, i)) == std::string::npos)
				return (false);
			i += it->length();
		} else {
			if (uri.compare(i, it->length(), it->c_str()) != 0)
				return (false);
			i += it->length();
		}
		it++;
		if (it == tokenVec.end()) {
			if (uri[i])
				return (false);
			return(true);
		}
	}
	if (it != tokenVec.end()) {
		if (*it++ != "*" || it != tokenVec.end())
			return (false);
	}
	return (true);
}

/// @brief Compare a token formed expression including ```*``` symbol with an uri.
/// @param location 
/// @param uri
/// @param previousMatch can be provided to check at the same time is the potential match
/// is better than an old one. Given string MUST be a valid match. Can be left to NULL.
/// @return 
bool	isUriMatch(const std::string& uriRef, const std::string& expression, const std::string* previousMatch = NULL)
{
	std::string	uri = uriRef;
	std::vector<std::string>	tokensA, tokensB;
	
	tokenize(expression, tokensA);
	if (isMatch(uri, tokensA) == false)
		return (false);
	if (previousMatch == NULL)
		return (true);
	tokenize(*previousMatch, tokensB);
	std::vector<std::string>::const_reverse_iterator	itA = tokensA.rbegin(), \
														itB = tokensB.rbegin();
	while (itA != tokensA.rend() && itB != tokensB.rend()) {
		if (*itA == "*") {
			if (*itB == "*")
				itA++, itB++;
			else
				return (false);
		} else if (*itB == "*")
			return (true);
		else {
			int	posA = uri.rfind(*itA), posB = uri.rfind(*itB);
			int	diff = (posA + itA->length()) - (posB + itB->length());
			if (diff > 0)
				return (true);
			else if (diff < 0)
				return (false);
			else if (itA->length() > itB->length())
				return (true);
			else if (itA->length() < itB->length())
				return (false);
			uri = uri.substr(0, posA);
			itA++, itB++;
		}
	}
	if (itA == tokensA.rend())
		return (false);
	return (true);
}
