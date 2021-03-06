/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#include <algorithm>
#include <cctype>
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

static const std::string STRING_RESERVED_CHARACTERS( ":/?#[]@!$&'()*+,;=" );

/*
 * RFC-3986: 2.3.  Unreserved Characters
 * unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
 */
static const std::string STRING_UNRESERVED_REGEX( "[A-Za-z0-9._~-]" );

static const std::string STRING_UNRESERVED_CHARACTERS(
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
	"abcdefghijklmnopqrstuvwxyz" \
	"0123456789-._~" );

/*
 * RFC-3986: 3.1.  Scheme
 * scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
 */
static const std::string STRING_URI_SCHEME_REGEX( "[a-zA-Z][a-zA-Z0-9+.-]*" );

/*
 * RFC-3986: 3.2.1.  User Information
 * userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
 */
static const std::string STRING_URI_USER_INFORMATION_REGEX(
	"(" + STRING_UNRESERVED_REGEX +
	"|" + STRING_PERCENT_ENCODED_REGEX +
	"|" + STRING_SUBSET_DELIMITERS_REGEX + "|:)*" );

/*
 * RFC-3986: 3.2.2.  Host
 * host        = IP-literal / IPv4address / reg-name
 * IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
 * IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
 * IPv6address =                            6( h16 ":" ) ls32
 *             /                       "::" 5( h16 ":" ) ls32
 *             / [               h16 ] "::" 4( h16 ":" ) ls32
 *             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
 *             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
 *             / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
 *             / [ *4( h16 ":" ) h16 ] "::"              ls32
 *             / [ *5( h16 ":" ) h16 ] "::"              h16
 *             / [ *6( h16 ":" ) h16 ] "::"
 * ls32        = ( h16 ":" h16 ) / IPv4address
 *             ; least-significant 32 bits of address
 * h16         = 1*4HEXDIG
 *             ; 16 bits of address represented in hexadecimal
 * IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
 * dec-octet   = DIGIT                 ; 0-9
 *             / %x31-39 DIGIT         ; 10-99
 *             / "1" 2DIGIT            ; 100-199
 *             / "2" %x30-34 DIGIT     ; 200-249
 *             / "25" %x30-35          ; 250-255
 * reg-name    = *( unreserved / pct-encoded / sub-delims )
 */
static const std::string STRING_REGISTERED_NAME_REGEX(
	"(" + STRING_UNRESERVED_REGEX +
	"|" + STRING_PERCENT_ENCODED_REGEX +
	"|" + STRING_SUBSET_DELIMITERS_REGEX + ")*" );
static const std::string STRING_DECIMAL_OCTET_REGEX(
	"([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])" );
static const std::string STRING_IPV4_ADDRESS_REGEX(
	STRING_DECIMAL_OCTET_REGEX + "." + STRING_DECIMAL_OCTET_REGEX +
	"." + STRING_DECIMAL_OCTET_REGEX + "." + STRING_DECIMAL_OCTET_REGEX );
static const std::string STRING_HEXADECIMAL_HEXTET_REGEX( "[0-9A-Fa-f]{4}" );
static const std::string STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX(
	"((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":" + STRING_HEXADECIMAL_HEXTET_REGEX + ")|" + STRING_IPV4_ADDRESS_REGEX + ")" );
static const std::string STRING_IPV6_ADDRESS_REGEX(
	"((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){6}" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|::(" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){5}" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|(" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::(" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){4}" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,1}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::(" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){3}" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,2}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::(" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){2}" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,3}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::" + STRING_HEXADECIMAL_HEXTET_REGEX + ":" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,4}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::" + STRING_IPV6_ADDRESS_LEAST_SIGNIFICANT_32_BITS_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,5}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::" + STRING_HEXADECIMAL_HEXTET_REGEX +
	"|((" + STRING_HEXADECIMAL_HEXTET_REGEX + ":){0,6}" + STRING_HEXADECIMAL_HEXTET_REGEX + ")?::)" );
static const std::string STRING_IPVFUTURE_REGEX(
	"v[0-9A-Fa-f]+\\.(" + STRING_UNRESERVED_REGEX + "|" + STRING_SUBSET_DELIMITERS_REGEX + "|:)+" );
static const std::string STRING_IPLITERAL_REGEX(
	"\\[(" + STRING_IPV6_ADDRESS_REGEX + "|" + STRING_IPVFUTURE_REGEX + ")\\]");
static const std::string STRING_URI_HOST_REGEX(
	"(" + STRING_IPLITERAL_REGEX +
	"|" + STRING_IPV4_ADDRESS_REGEX +
	"|" + STRING_REGISTERED_NAME_REGEX + ")" );

/*
 * RFC-3986: 3.2.3.  Port
 * port        = *DIGIT
 * Although the regex defined in RFC-3986 §3.2.3 allows any string
 * of digits, the only ports that are valid are 1-65535, so the regex defined
 * here shall only allow 1-65535. The string of zero is a separate case
 * that signals to the OS that the system should use the default port
 * defined for the scheme.
 */
static const std::string STRING_URI_PORT_REGEX(
	"([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])" );
//        1-9999,         10000-59999,  60000-64999,   65000-65499,    65500-65529,  65530-65535

/*
 * RFC-3986: 3.3.  Path
 * path          = path-abempty    ; begins with "/" or is empty
 *               / path-absolute   ; begins with "/" but not "//"
 *               / path-noscheme   ; begins with a non-colon segment
 *               / path-rootless   ; begins with a segment
 *               / path-empty      ; zero characters
 * path-abempty  = *( "/" segment )
 * path-absolute = "/" [ segment-nz *( "/" segment ) ]
 * path-noscheme = segment-nz-nc *( "/" segment )
 * path-rootless = segment-nz *( "/" segment )
 * path-empty    = 0<pchar>
 * segment       = *pchar
 * segment-nz    = 1*pchar
 * segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
 *               ; non-zero-length segment without any colon ":"
 * pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
 */
static const std::string STRING_PATH_CHARACTER_REGEX(
	"(" + STRING_UNRESERVED_REGEX +
	"|" + STRING_PERCENT_ENCODED_REGEX +
	"|" + STRING_SUBSET_DELIMITERS_REGEX +
	"|:|@)" );
static const std::string STRING_SEGMENT_NON_ZERO_LENGTH_NO_COLON_REGEX(
	"(" + STRING_UNRESERVED_REGEX +
	"|" + STRING_PERCENT_ENCODED_REGEX +
	"|" + STRING_SUBSET_DELIMITERS_REGEX +
	"|@)+" );
static const std::string STRING_SEGMENT_NON_ZERO_LENGTH_REGEX( STRING_PATH_CHARACTER_REGEX + "+" );
static const std::string STRING_SEGMENT_REGEX( STRING_PATH_CHARACTER_REGEX + "*" );
static const std::string STRING_PATH_EMPTY_REGEX( STRING_PATH_CHARACTER_REGEX + "{0}" );
static const std::string STRING_PATH_ROOTLESS_REGEX(
	STRING_SEGMENT_NON_ZERO_LENGTH_REGEX + "(/" + STRING_SEGMENT_REGEX + ")*" );
static const std::string STRING_PATH_NO_SCHEME_REGEX(
	STRING_SEGMENT_NON_ZERO_LENGTH_NO_COLON_REGEX + "(/" + STRING_SEGMENT_REGEX + ")*" );
static const std::string STRING_PATH_ABSOLUTE_REGEX(
	"/(" + STRING_SEGMENT_NON_ZERO_LENGTH_REGEX + "(/" + STRING_SEGMENT_REGEX + ")*)?");
static const std::string STRING_PATH_ABSOLUTE_OR_EMPTY_REGEX(
	"(/" + STRING_SEGMENT_REGEX + ")*" );
static const std::string STRING_URI_PATH_REGEX(
	"(" + STRING_PATH_ABSOLUTE_OR_EMPTY_REGEX +
	"|" + STRING_PATH_ABSOLUTE_REGEX +
	"|" + STRING_PATH_NO_SCHEME_REGEX +
	"|" + STRING_PATH_ROOTLESS_REGEX +
	"|" + STRING_PATH_EMPTY_REGEX + ")" );

/*
 * RFC-3986: 3.4.  Query
 * query       = *( pchar / "/" / "?" )
 */
static const std::string STRING_URI_QUERY_REGEX(
	"(" + STRING_PATH_CHARACTER_REGEX + "|/|\\?)*" );

/*
 * RFC-3986: 3.5.  Fragment
 * fragment    = *( pchar / "/" / "?" )
 */
static const std::string STRING_URI_FRAGMENT_REGEX(
	"(" + STRING_PATH_CHARACTER_REGEX + "|/|\\?)*" );

static const std::regex REGEX_URI_SCHEME( "^" + STRING_URI_SCHEME_REGEX + ":?$" );
static const std::regex REGEX_URI_USER_INFORMATION( "^" + STRING_URI_USER_INFORMATION_REGEX + "@?$" );
static const std::regex REGEX_URI_HOST( "^" + STRING_URI_HOST_REGEX + "$" );
static const std::regex REGEX_URI_PORT( "^:?" + STRING_URI_PORT_REGEX + "$" );
static const std::regex REGEX_URI_PATH( "^" + STRING_URI_PATH_REGEX + "$" );
static const std::regex REGEX_URI_QUERY( "^\\??" + STRING_URI_QUERY_REGEX + "$" );
static const std::regex REGEX_URI_FRAGMENT( "^#?" + STRING_URI_FRAGMENT_REGEX + "$" );

/**
 * RFC-3986 in Appendix B. Parsing a URI Reference with a Regular Expression
 * defines a regular expression for parsing a 5 part URI, however the authority
 * is parsed as a complete unit instead of its constituents. So the expression has
 * been expanded upon to parse a 7 part URI.
 */
static const std::regex REGEX_URI_PARSE( "^(([^:/?#]+):)?(//(([^@]+)@)?([^/?#:]*)(:([^/?#]+))?)?([^?#]*)(\\?([^#]*))?(#(.*))?$" );

// Generator code:
// $ awk 'BEGIN{for(i=0;i<256;i++) printf "\"%%%2.2X\",%c",i,(0==(i+1)%16)?"\n":" "}'
static const char ARRAY_PERCENT_ENCODED_CHARACTER_TABLE[ 256 ][ 4 ] =
{
	"%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07", "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
	"%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17", "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
	"%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27", "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
	"%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37", "%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
	"%40", "%41", "%42", "%43", "%44", "%45", "%46", "%47", "%48", "%49", "%4A", "%4B", "%4C", "%4D", "%4E", "%4F",
	"%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57", "%58", "%59", "%5A", "%5B", "%5C", "%5D", "%5E", "%5F",
	"%60", "%61", "%62", "%63", "%64", "%65", "%66", "%67", "%68", "%69", "%6A", "%6B", "%6C", "%6D", "%6E", "%6F",
	"%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77", "%78", "%79", "%7A", "%7B", "%7C", "%7D", "%7E", "%7F",
	"%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87", "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
	"%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97", "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
	"%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7", "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
	"%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7", "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
	"%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7", "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
	"%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7", "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
	"%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7", "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
	"%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7", "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static std::string __uri_percent_encode_encode(
	const std::string& unencodedString )
{
	std::string encodedString;
	for ( const auto& character : unencodedString )
	{
		if ( std::string::npos == STRING_UNRESERVED_CHARACTERS.find( character ) )
		{
			encodedString.append( ARRAY_PERCENT_ENCODED_CHARACTER_TABLE[ static_cast< unsigned char >( character ) ] );
		}
		else
		{
			encodedString.append( 1, character );
		}
	}

	return encodedString;
}

// It is assumed that {@param hexadecimalCharacter} ∈ [0-9A-Fa-f]
static inline unsigned char __hexadecimal_character_to_nibble(
	unsigned char hexadecimalCharacter )
{
	return ( hexadecimalCharacter >= 'A' ) ? hexadecimalCharacter - 32 * ( hexadecimalCharacter >= 'a' ) - 'A' + 10 : hexadecimalCharacter - '0';
}

static inline unsigned char __hexadecimal_octet_to_char(
	const char* hexadecimalPair )
{
	return ( __hexadecimal_character_to_nibble( hexadecimalPair[ 0 ] ) << 4 ) | __hexadecimal_character_to_nibble( hexadecimalPair[ 1 ] );
}

static std::string __uri_percent_encode_decode(
	const std::string& encodedString )
{
	std::string unencodedString;
	for ( size_t index( -1 ); ++index < encodedString.size(); )
	{
		if ( '%' == encodedString[ index ] )
		{
			// Check that the next 2 characters are hexadecimal.
			// If the next 2 characters aren't hexadecimal, then it can be the
			// responsibility of the caller to know if that substring is valid.
			if ( ( index + 2 ) < encodedString.size() )
			{
				if ( std::isxdigit( encodedString[ index + 1 ] ) and std::isxdigit( encodedString[ index + 2 ] ) )
				{
					unencodedString.append( 1, __hexadecimal_octet_to_char( encodedString.c_str() + index + 1 ) );
					index += 2;
				}
			}
		}
		else
		{
			unencodedString.append( 1, encodedString[ index ] );
		}
	}

	return unencodedString;
}

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
	mRawScheme = other.mRawScheme;
	mRawUserInformation = other.mRawUserInformation;
	mRawUserInformationWithPassword = other.mRawUserInformationWithPassword;
	mRawHost = other.mRawHost;
	mRawPath = other.mRawPath;
	mRawQuery = other.mRawQuery;
	mRawFragment = other.mRawFragment;
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
	bool hasAuthority = false;

	mScheme.clear();
	mUserInformation.clear();
	mHost.clear();
	mPort.clear();
	mPath.clear();
	mQuery.clear();
	mFragment.clear();

	mRawScheme.clear();
	mRawUserInformation.clear();
	mRawUserInformationWithPassword.clear();
	mRawHost.clear();
	mRawPath.clear();
	mRawQuery.clear();
	mRawFragment.clear();

	// An empty URI is a valid relative-URI
	mIsAbsolute = false;
	mIsRelative = true;

	// Scheme
	if ( not schemeString.empty() )
	{
		if ( not std::regex_match( schemeString, REGEX_URI_SCHEME ) )
		{
			throw std::invalid_argument( "Invalid scheme" );
		}

		mRawScheme = schemeString;
		std::transform( mRawScheme.begin(), mRawScheme.end(), mRawScheme.begin(),
			[]( unsigned char character )
			{
				return std::tolower( character );
			} );
		mScheme = schemeString;
		mIsRelative = false;
		mIsAbsolute = true;
	}

	// User information
	if ( not userInformationString.empty() )
	{
		// TODO: Walk through the logic of when to encode/decode
		mRawUserInformation = userInformationString;
		mUserInformation = userInformationString;
		hasAuthority = true;

		if ( not std::regex_match( mRawUserInformation, REGEX_URI_USER_INFORMATION ) )
		{
			mRawUserInformation = __uri_percent_encode_encode( mRawUserInformation );
			// Try encoding the user information before throwing invalid_argument
			throw std::invalid_argument( "Invalid user information" );
		}
		else
		{
			mUserInformation = __uri_percent_encode_decode( mUserInformation );
		}
	}

	// Host
	if ( hostString.empty() )
	{
		if ( hasAuthority )
		{
			throw std::invalid_argument( "User information set without a host" );
		}
	}
	else
	{
		mRawHost = __uri_percent_encode_encode( hostString );

		if ( not std::regex_match( mRawHost, REGEX_URI_HOST ) )
		{
			throw std::invalid_argument( "Invalid host" );
		}

		mHost = hostString;
	}

	// Port
	if ( portString.empty() )
	{
		if ( hasAuthority )
		{
			// Get the default port for the scheme if set.
		}
	}
	else
	{
		if ( not std::regex_match( portString, REGEX_URI_PORT ) )
		{
			throw std::invalid_argument( "Invalid port" );
		}

		if ( not hasAuthority )
		{
			throw std::invalid_argument( "No host defined for port" );
		}

		mPort = portString;
	}

	// Path
	if ( not pathString.empty() )
	{
		if ( not std::regex_match( pathString, REGEX_URI_PATH ) )
		{
			throw std::invalid_argument( "Invalid path" );
		}

		mPath = pathString;
	}

	// Query
	if ( not queryString.empty() )
	{
		if ( not std::regex_match( queryString, REGEX_URI_QUERY ) )
		{
			throw std::invalid_argument( "Invalid query" );
		}

		mQuery = queryString;
	}

	// Fragment
	if ( not fragmentString.empty() )
	{
		if ( not std::regex_match( fragmentString, REGEX_URI_FRAGMENT ) )
		{
			throw std::invalid_argument( "Invalid fragment" );
		}

		mFragment = fragmentString;
		mIsAbsolute = false;
		mIsRelative = false;
	}
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
	mRawScheme = std::move( other.mRawScheme );
	mRawUserInformation = std::move( other.mRawUserInformation );
	mRawUserInformationWithPassword = std::move( other.mRawUserInformationWithPassword );
	mRawHost = std::move( other.mRawHost );
	mRawPath = std::move( other.mRawPath );
	mRawQuery = std::move( other.mRawQuery );
	mRawFragment = std::move( other.mRawFragment );
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

const std::string& URI::getCanonicalScheme() const
{
	return mRawScheme;
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

const std::string& URI::getRawUserInformation(
	bool includePassword ) const
{
	return ( includePassword ) ? mRawUserInformationWithPassword : mRawUserInformation;
}

const std::string& URI::getRawHost() const
{
	return mRawHost;
}

const std::string& URI::getRawPath() const
{
	return mRawPath;
}

const std::string& URI::getRawQuery() const
{
	return mRawQuery;
}

const std::string& URI::getRawFragment() const
{
	return mRawFragment;
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
