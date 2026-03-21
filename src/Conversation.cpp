#include "../inc/Conversation.hpp"
#include "../inc/parse/Parser.hpp"
#include "../inc/validate/Validator.hpp"
#include "../inc/read/Reader.hpp"

Conversation::Conversation() {
	parser = new Parser;
	reader = new Reader;
	validator = new Validator;
}
