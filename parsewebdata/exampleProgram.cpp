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
 ============================================================================
 Name        : exampleProgram.c
 Author      : Cyril Margorin
 Version     :
 Copyright   : Your copyright notice
 Description : Uses shared library to print greeting
 To run the resulting executable the LD_LIBRARY_PATH must be
 set to ${project_loc}/ParseWebData/.libs
 Alternatively, libtool creates a wrapper shell script in the
 build directory of this program which can be used to run it.
 Here the script will be called exampleProgram.
 ============================================================================
 */

#include <ParseWebData.h>

#include <string.h>
#include <iostream>
#include <iomanip>


int main(int argc, char *argv[]) {
	struct ContentDataPair {
		const char* ct;
		const char* data;
	};
	ContentDataPair testData[] = {
		{ "multipart/form-data; boundary=----WebKitFormBoundaryoSPfJ4Xa1po4HxQA",
				"------WebKitFormBoundaryoSPfJ4Xa1po4HxQA\r\n"
				"Content-Disposition: form-data; name=\"file\"; filename=\"file.txt\"\r\n"
				"Content-Type: text/plain\r\n"
				"\r\n"
				"file-content\r\n"
				"\r\n"
				"------WebKitFormBoundaryoSPfJ4Xa1po4HxQA\r\n"
				"Content-Disposition: form-data; name=\"command\"\r\n"
				"\r\n"
				"cmd\r\n"
				"------WebKitFormBoundaryoSPfJ4Xa1po4HxQA--\r\n" 
		},
		{ "application/x-www-form-urlencoded",
				"file=file-content&command=cmd" 
		}, 
		{ "text/plain",
				"file=file-content\r\n"
				"command=cmd\r\n" 
		}, 
		{ NULL, NULL } 
	};

	for (ContentDataPair *data = testData; data->ct != NULL; ++data) {
		ParseWebData::WebDataMap dataMap;
		ParseWebData::parse_post_data(data->data, data->ct, dataMap);
		cout << "Data with type " << data->ct << endl;
		for (ParseWebData::WebDataMap::const_iterator iter = dataMap.begin();
				iter != dataMap.end(); ++iter) {
			cout << setw(20) << setfill('-') << '-' << endl;
			cout << (*iter).first << " : " << (*iter).second.value << endl;
		}
		cout << setw(40) << setfill('=') << '=' << endl;
	}
	return 0;
}
