/*
 * Copyright 2013-2020 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "util/cmdline/Parser.h"

#include <sstream>

#include <boost/algorithm/string/predicate.hpp>

#include "util/String.h"

namespace util::cmdline {

namespace {

enum OptionType {
	PositionalArguments,
	LongOption,
	ShortOption
};

} // anonymous namespace

void parse(interpreter<std::string> & cli, int argc, char ** argv) {
	
	// Create a copy of the arguments that we can edit
	std::vector<std::string> args;
	typedef std::vector<std::string>::iterator iterator;
	if(argc) {
		std::copy(argv + 1, argv + argc, std::inserter(args, args.end()));
	}
	
	// Parse tokens one by one
	interpreter<std::string>::type_cast_t tc;
	iterator p = args.begin();
	const iterator end = args.end();
	while(p != end) {
		
		// Parse the option starting athe the current token
		std::string option = *p;
		try {
			
			const iterator original_p = p;
			
			iterator optend = p + 1; //!< end for optional arguments
			
			OptionType type;
			
			if(boost::algorithm::starts_with(option, "--")) {
				
				//! Handle long options
				type = LongOption;
				
				// Long options can have an argument appended as --option=arg
				std::string::size_type sep = option.find('=');
				if(sep != std::string::npos) {
					// Re-insert remaining part of the token as an argument
					*p = option.substr(sep + 1);
					option = option.substr(0, sep);
				} else {
					// Token fully consumed as an option
					++p;
				}
				
				// Special option -- forces the remaining tokens to be treated as positional args
				if(option == "--") {
					optend = end;
					type = PositionalArguments;
				}
				
			} else if(boost::algorithm::starts_with(option, "-")) {
				
				//! Handle short options
				type = ShortOption;
				
				// Short options can contain multiple options and/or a final argument
				if(option.length() > 2) {
					// Re-insert remaining part of the token as an argument
					*p = option.substr(2);
					option.resize(2);
				} else {
					// Token fully consumed as an option
					++p;
				}
				
			} else {
				
				//! Handle positional arguments (without an option)
				type = PositionalArguments;
				option = "--";
				
			}
			
			// Scan ahead to find the next token starting with a dash (-> could be an option)
			// This is used to determine how many tokens to consume for optional arguments
			for(; optend != end; ++optend) {
				if(boost::algorithm::starts_with(*optend, "-")) {
					// Could be the next option, do not consume as optional arguments
					break;
				}
			}
			
			// Parse the option and consume as many arguments as needed
			cli.invoke(option, p, optend, end, tc);
			
			// There was an explicit argument, but it was not used by the option!
			if(p == original_p) {
				switch(type) {
					
					case LongOption: {
						// Argument given for long option that doesn't take arguments
						throw error(error::invalid_arg_count, "too many arguments");
					}
					
					case ShortOption: {
						// Short option didn't take any arguments
						// Re-try the rest of the token as more short options
						p->insert(p->begin(), '-');
						break;
					}
					
					case PositionalArguments: {
						// Unused positional arguments
						throw error(error::invalid_arg_count, "too many positional arguments");
					}
					
				}
			}
			
		} catch(error & e) {
			
			std::ostringstream oss;
			oss << "Error parsing command-line";
			
			if(option == "--") {
				if(p != end) {
					oss << " argument \"" << util::escapeString(*p) << "\"";
				}
				oss << ": ";
				if(e.code() == error::cmd_not_found) {
					oss << "positional arguments not supported";
				} else {
					oss << e.what();
				}
			} else {
				oss << " option " << option << ": " << e.what();
				if(p != end && e.code() == error::invalid_arg_count) {
					oss << ": \"" << util::escapeString(*p) << "\"";
				}
			}
			
			throw error(e.code(), oss.str());
			
		} catch(...) {
			
			std::ostringstream oss;
			oss << "Error parsing command-line";
			
			if(option != "--") {
				oss << " option " << option;
			}
			
			oss << ": invalid value";
			if(p != end) {
				oss << " \"" << util::escapeString(*p) << "\"";
			}
			
			throw error(error::invalid_value, oss.str());
			
		}
		
	}
	
}

} // namespace util::cmdline
