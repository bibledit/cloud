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
/*
 * ParseWebData_local.h
 *
 *  Created on: Jul 10, 2012
 *      Author: cyril
 */

#ifndef ParseWebData_H
#define ParseWebData_H

#include <string>
#include <map>

/*!
 * Parse Web Data namespace
 */
namespace ParseWebData {

/*!
 * Map string -> string
 */
typedef std::map<std::string, std::string> string_map;

/*!
 * Passed variable structure
 */
struct WebData {
	WebData() {
	}
	WebData(const std::string& _value) :
			value(_value) {
	}
	/*!
	 * Value of variable
	 */
	std::string value;
	/*!
	 * Additional attributes of variable
	 */
	string_map attributes;
};

/*!
 * Map Variable name to variable structure
 */
typedef std::map<std::string, WebData> WebDataMap;

/*!
 * Parse data from POST request
 * @param[in]  postData POST request to be parsed
 * @param[in]  contentType contents of Content-Type header
 * @param[out] postDataMap map of Variable->(Value+attribs)
 * @return success of parsing
 */
bool parse_post_data(const std::string& postData,
		const std::string& contentType, WebDataMap& postDataMap);

/*!
 * Parse data from GET request (query string)
 * @param[in]  getData x
 * @param[out] postDataMap x
 * @return success of parsing
 */
bool parse_get_data(const std::string& getData, WebDataMap& postDataMap);

} // namespace ParseWebData

#endif
