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

/*
 * RFC-3986: 2.1.  Percent-Encoding
 * pct-encoded = "%" HEXDIG HEXDIG
 */
static const std::string STRING_PERCENT_ENCODED_REGEX( "%[0-9a-fA-F][0-9a-fA-F]" );

/*
 * RFC-3986: 2.2.  Reserved Characters
 * reserved    = gen-delims / sub-delims
 * gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
 * sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
 *             / "*" / "+" / "," / ";" / "="
 */
static const std::string STRING_SUBSET_DELIMITERS_REGEX( "[!$&'()*+,;=]" );
static const std::string STRING_GENERAL_DELIMITERS_REGEX( "[:/?#\\][@]" );
static const std::string STRING_RESERVED_REGEX(
	"(" + STRING_GENERAL_DELIMITERS_REGEX + "|" + STRING_SUBSET_DELIMITERS_REGEX + ")" );

/*
 * RFC-3986: 2.3.  Unreserved Characters
 * unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
 */
static const std::string STRING_UNRESERVED_REGEX( "[A-Za-z0-9._~-]" );

/*
 * RFC-3986: 3.1.  Scheme
 * scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
 */
static const std::string STRING_URI_SCHEME_REGEX( "[a-zA-Z][a-zA-Z0-9+.-]" );

/*
 * RFC-3986: 3.2.1.  User Information
 * userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
 */
static const std::string STRING_URI_USER_INFORMATION_REGEX(
	"(" + STRING_UNRESERVED_REGEX +
	"|" + STRING_PERCENT_ENCODED_REGEX +
	"|" + STRING_SUBSET_DELIMITERS_REGEX + "|:)*" );

static const std::regex REGEX_URI_SCHEME( "^" + STRING_URI_SCHEME_REGEX + ":?$" );
static const std::regex REGEX_URI_USER_INFORMATION( "^" + STRING_URI_USER_INFORMATION_REGEX + "@?$" );

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

	mIsAbsolute = false;
	mIsRelative = true;

	if ( schemeString.empty() )
	{
		mIsRelative = true;
	}
	else
	{
		if ( not std::regex_match( schemeString, REGEX_URI_SCHEME ) )
		{
			throw std::invalid_argument( "Invalid scheme" );
		}

		mScheme = schemeString;
	}

	if ( userInformationString.empty() )
	{
	}
	else
	{
		if ( not std::regex_match( userInformationString, REGEX_URI_USER_INFORMATION ) )
		{
			throw std::invalid_argument( "Invalid user information" );
		}

		mUserInformation = userInformationString;
	}

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

const std::string& URI::getScheme() const
{
	return mScheme;
}

const std::string& URI::getUserInformation() const
{
	return mUserInformation;
}

const std::string& URI::getHost() const
{
	return mHost;
}

const std::string& URI::getPort() const
{
	return mPort;
}

const std::string& URI::getPath() const
{
	return mPath;
}

const std::string& URI::getQuery() const
{
	return mQuery;
}

const std::string& URI::getFragment() const
{
	return mFragment;
}

URI& URI::operator=(
	URI&& other )
{
	if ( this != &other )
	{
		_moveAssign( std::move( other ) );
	}

	return *this;
}

URI& URI::operator=(
	const URI& other )
{
	if ( this != &other )
	{
		_copyAssign( other );
	}

	return *this;
}
