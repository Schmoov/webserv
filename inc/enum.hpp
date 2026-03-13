#pragma once

enum ConvState {
	//reader
	READ_CLIENT,
	EOF_CLIENT,
	FINISH, //Free ressources, set by reader when socket errors
			//set by parser when client gracefully closed connection
			//Should be set after successfully sending a response with
			//connection: closed

	//parser
	PARSE, // NEEDS TO BE SET AFTER SENDING RESPONSE !
	PARSE_BODY, // NEEDS TO BE SET AFTER ANSWERING 100 to a expect continue

	VALIDATE,

	EXEC, // set by validator any time we should respond
		  // may need actual execution may be just 400 connection close
	
	TIMEOUT_STATE, // TO DO

	//old states for inspiration no code relies on them
	RESPONSE,
	WRITE_CLIENT, //this is in stateManager.cpp
	READ_EXEC,
	TO_READ,
	TO_SEND,
	IS_SENT
};

enum StatusCode {
	NOT_A_STATUS_CODE = 0,
	CONTINUE = 100,
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	TIMEOUT = 408,
	LENGTH_REQUIRED = 411,
	ENTITY_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	EXPECTATION_FAILED = 417,
	REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	HTTP_VERSION_NOT_SUPPORTED = 505
};
