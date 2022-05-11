/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <string>

class URI
{
private:
	std::string mScheme;
	std::string mUserInformation;
	std::string mHost;
	std::string mPort;
	std::string mPath;
	std::string mQuery;
	std::string mFragment;

	void _copyAssign(
		const URI& other );

	void _initialize(
		const std::string& schemeString,
		const std::string& userInformationString,
		const std::string& hostString,
		const std::string& portString,
		const std::string& pathString,
		const std::string& queryString,
		const std::string& fragmentString );

	void _moveAssign(
		URI&& other );

	static std::tuple<
		std::string, // Scheme
		std::string, // User Information
		std::string, // Host
		std::string, // Port
		std::string, // Path
		std::string, // Query
		std::string  // Fragment
	> _parseUriString(
		const std::string& uriString );

public:
	/**
	 * Initialize an empty URI.
	 */
	URI();

	/**
	 * Move constructor.
	 * @param other R-Value to the URI object to move to this instance.
	 */
	URI(
		URI&& other );

	/**
	 * Copy constructor.
	 * @param other Const reference to the URI object to copy to this instance.
	 */
	URI(
		const URI& other );

	/**
	 * Initialize the URI object to the given URI.
	 * @param uriString The string representation of the URI.
	 * @throw std::invalid_argument is thrown if {@param uriString} is neither a valid
	 *        relative, absolute, general URI
	 */
	URI(
		const std::string& uriString );
};
