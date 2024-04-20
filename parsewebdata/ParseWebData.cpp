/*
 The MIT License (MIT)
 Copyright (c) 2012 Cyril Margorin, Imperx Inc. and other contributors
 https://code.google.com/p/parsewebdata/
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Winline"


#include "ParseWebData.h"
#include "ParseWebData_local.h"

#include "ParseMultipartFormData.h"

#include <algorithm>
#include <sstream>
#include <iterator>

namespace ParseWebData {

typedef bool (*ParserFunc)(const std::string& data, const string_map& content_type, WebDataMap& dataMap);

static bool parse_default(const std::string& data, const string_map& content_type, WebDataMap& dataMap) {
  if (data.empty()) {};
  if (content_type.empty()) {};
  if (sizeof (dataMap)) {};
	return false;
}

bool parse_url_encoded_data(const std::string& data, const string_map& content_type, WebDataMap& dataMap)
{
  if (content_type.empty()) {};
	string_map values = map_pairs(data, "&", "=");
	for(string_map::const_iterator iter = values.begin(); iter != values.end(); ++iter)
	{
		dataMap.insert(std::make_pair((*iter).first, WebData((*iter).second)));
	}
	return true;
}

bool parse_plain_text_data(const std::string& data, const string_map& content_type, WebDataMap& dataMap)
{
  if (content_type.empty()) {};
	string_map values = map_pairs(data, "\r\n", "=");
	for(string_map::const_iterator iter = values.begin(); iter != values.end(); ++iter)
	{
		dataMap.insert(std::make_pair((*iter).first, WebData((*iter).second)));
	}
	return true;
}

typedef std::map<std::string, ParserFunc> ParserFuncsMap;
static const ParserFuncsMap& get_parsers_map() {
	static ParserFuncsMap parsers_map;
	if (parsers_map.empty()) {
		parsers_map.insert(
				std::make_pair("multipart/form-data",
						&ParseMultipartFormData::parse_data));
		parsers_map.insert(
				std::make_pair("application/x-www-form-urlencoded",
						&parse_url_encoded_data));
		parsers_map.insert(
				std::make_pair("text/plain",
						&parse_plain_text_data));
	}

	return parsers_map;
}

static ParserFunc get_parser(const std::string& contentType) {
	ParserFuncsMap::const_iterator iter = get_parsers_map().find(contentType);
	if (get_parsers_map().end() == iter)
		return &parse_default;
	else
		return (*iter).second;

}

bool parse_post_data(const std::string & postData,
		const std::string & contentType, WebDataMap & postDataMap) {
	// 1. parse contentType
	string_map contentTypeMap = map_pairs(
			std::string("content-type=") + contentType, "; ", "=");
	return (*get_parser(contentTypeMap["content-type"]))(postData,
			contentTypeMap, postDataMap);
}

bool parse_get_data(const std::string & getData, WebDataMap & getDataMap) {
	return parse_url_encoded_data(getData, string_map(), getDataMap);
}

namespace map_pairs_helper {
class ParsePairsFunc {
public:
	ParsePairsFunc(const std::string& _pairDelim, string_map& _result) :
			pairDelim(_pairDelim), result(_result) {
	}
	void operator ()(const std::string& value) {
		string_list list = split(value, pairDelim);
		std::pair<std::string, std::string> pair;
		string_list::const_iterator iter = list.begin();
		if (iter != list.end()) {
			pair.first = *iter;
			iter++;
			if (iter != list.end()) {
				pair.second = *iter;
			}
		}

		result.insert(pair);
	}

private:
	const std::string& pairDelim;
	string_map& result;
};
} // namespace map_pairs_helper

string_map map_pairs(const std::string& s, const std::string& elemDelim,
		const std::string& pairDelim) {
	using  namespace map_pairs_helper;

	string_list words;
	string_map result;

	words = split(s, elemDelim, false);
	std::for_each(words.begin(), words.end(),
			ParsePairsFunc(pairDelim, result));
	return result;
}

string_list split(const std::string & s, const std::string & delim,
		bool keep_empty) {
	string_list result;
	std::string::const_iterator substart = s.begin(), subend = substart;
	while (true) {
		subend = std::search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			result.push_back(temp);
		}
		if (subend == s.end())
			break;
		substart = subend + delim.size();
	}

	return result;
}

}
// namespace ParseWebData
