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

	/**
	 * Get the scheme for the URI. For relative URIs, this is not defined.
	 * @return Const reference to the scheme component of the URI.
	 */
	const std::string& getScheme() const;

	/**
	 * Get the user information. This information may or may not be present.
	 * @return Const reference to the user information component of the URI.
	 */
	const std::string& getUserInformation() const;

	/**
	 * Get the host. If the URI comprises an authority, then this
	 * component is required to be present.
	 * @return Const reference to the host component of the URI.
	 */
	const std::string& getHost() const;

	/**
	 * Get the port. If the URI comprises an authority, a scheme is present, and
	 * the port is not otherwise set, then if the scheme is present in the table of
	 * known scheme default port numbers, then the default port is returned.
	 * @return Const reference to the port component of the URI.
	 */
	const std::string& getPort() const;

	/**
	 * Get the path component of the URI.
	 * @return Const reference to the path component of the URI.
	 */
	const std::string& getPath() const;

	/**
	 * Get the query component of the URI.
	 * @return Const reference to the query component of the URI.
	 */
	const std::string& getQuery() const;

	/**
	 * Get the fragment component of the URI.
	 * @return Const reference to the fragment component of the URI.
	 */
	const std::string& getFragment() const;

	/**
	 * Move assignment operator.
	 * @param other R-Value to the URI object to move to this instance.
	 * @return Reference to this URI instance is returned.
	 */
	URI& operator=(
		URI&& other );

	/**
	 * Copy assignment operator.
	 * @param other Const reference to the URI object to copy to this instance.
	 * @return Reference to this URI instance is returned.
	 */
	URI& operator=(
		const URI& other );
};
