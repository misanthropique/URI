/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <regex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "URI.hpp"

/**
 * RFC-3986 in Appendix B. Parsing a URI Reference with a Regular Expression
 * defines a regular expression for parsing a 5 part URI, however the authority
 * is parsed as a complete unit instead of its constituents. So the expression has
 * been expanded upon to parse a 7 part URI.
 */
static const std::regex REGEX_URI_PARSE( "^(([^:/?#]+):)?(//(([^@]+)@)?([^/?#:]*)(:([^/?#]+))?)?([^?#]*)(\\?([^#]*))?(#(.*))?$" );

void URI::_copyAssign(
	const URI& other )
{
	mScheme = other.mScheme;
	mUserInformation = other.mUserInformation;
	mHost = other.mHost;
	mPort = other.mPort;
	mPath = other.mPath;
	mQuery = other.mQuery;
	mFragment = other.mFragment;
}

void URI::_initialize(
	const std::string& schemeString,
	const std::string& userInformationString,
	const std::string& hostString,
	const std::string& portString,
	const std::string& pathString,
	const std::string& queryString,
	const std::string& fragmentString )
{
	mScheme.clear();
	mUserInformation.clear();
	mHost.clear();
	mPort.clear();
	mPath.clear();
	mQuery.clear();
	mFragment.clear();

	mScheme = schemeString;
	mUserInformation = userInformationString;
	mHost = hostString;
	mPort = portString;
	mPath = pathString;
	mQuery = queryString;
	mFragment = fragmentString;
}

void URI::_moveAssign(
	URI&& other )
{
	mScheme = std::move( other.mScheme );
	mUserInformation = std::move( other.mUserInformation );
	mHost = std::move( other.mHost );
	mPort = std::move( other.mPort );
	mPath = std::move( other.mPath );
	mQuery = std::move( other.mQuery );
	mFragment = std::move( other.mFragment );
}

std::tuple<
	std::string, // Scheme
	std::string, // User Information
	std::string, // Host
	std::string, // Port
	std::string, // Path
	std::string, // Query
	std::string  // Fragment
> URI::_parseUriString(
	const std::string& uriString )
{
	std::smatch uriGroupMatch;
	std::string scheme, userInformation, host, port, path, query, fragment;

	if ( std::regex_search( uriString, uriGroupMatch, REGEX_URI_PARSE ) )
	{
		scheme = uriGroupMatch[ 2 ];
		userInformation = uriGroupMatch[ 5 ];
		host = uriGroupMatch[ 6 ];
		port = uriGroupMatch[ 8 ];
		path = uriGroupMatch[ 9 ];
		query = uriGroupMatch[ 11 ];
		fragment = uriGroupMatch[ 13 ];
	}
	else
	{
		throw std::invalid_argument( "Failed to parse a URI from the given string" );
	}

	return { scheme, userInformation, host, port, path, query, fragment };
}

URI::URI()
{
	_initialize( "", "", "", "", "", "", "" );
}

URI::URI(
	URI&& other )
{
	_moveAssign( std::move( other ) );
}

URI::URI(
	const URI& other )
{
	_copyAssign( other );
}

URI::URI(
	const std::string& uriString )
{
	auto uriComponents = _parseUriString( uriString );
	std::apply( &URI::_initialize, std::tuple_cat( std::forward_as_tuple( *this ), uriComponents ) );
}
