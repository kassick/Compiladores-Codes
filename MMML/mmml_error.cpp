/****************************************************************************
 *        Filename: "MMML/mmml_error.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 16:40:03 2017"
 *         Updated: "2017-10-10 23:23:31 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/error.H"
#include <iostream>

namespace mmml {

using namespace std;

// Static storage for out_stream and err_stream
ostream* Report::out_stream = &cout ;
ostream* Report::err_stream = &cerr ;
int Report::nerrors = 0;
int Report::nwarns = 0;
}
