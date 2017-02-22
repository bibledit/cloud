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
 * ParseMultipartFormData.h
 *
 *  Created on: Jul 10, 2012
 *      Author: cyril
 */

#ifndef PARSEMULTIPARTFORMDATA_H_
#define PARSEMULTIPARTFORMDATA_H_

#include "ParseWebData_local.h"

namespace ParseWebData {
namespace ParseMultipartFormData {

bool parse_data(const std::string& data,
		const string_map& content_type, WebDataMap& dataMap);


}  // namespace ParseMultipartFormData
}  // namespace ParseWebData

#endif /* PARSEMULTIPARTFORMDATA_H_ */
