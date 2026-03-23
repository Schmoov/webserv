#include "../../inc/webserv.hpp"
#include "../../inc/read/Reader.hpp"
#include "../../inc/parse/Parser.hpp"
#include "../../inc/validate/Validator.hpp" //calls needs complete type of conv subclass

#include <iostream>
//You can do *func if you prefer

std::string state_to_str(ConvState state)
{
	if(state == 0)
		return "read";
	if(state == 1)
		return "EOF";
	if(state == 2)
		return "FINISH";
	if(state == 3)
		return "PARSE";
	if(state == 4)
		return "PARSE_BODY";
	if(state == 5)
		return "VALIDATE";
	if(state == 6)
		return "VALIDATE";
	return "other code";
}

void manage(Conversation& conv) {
	std::cout << "Parse State enter: " << conv.state << std::endl;;
	//forgot about that : READ_CLIENT means both can read and need read
	//The first time manager sees IO state it comes from epoll so
	//we can interact with the IO the second time it means someone wants
	//IO so we give control back to epoll
	//We can make it 2 states if you prefer
	if (conv.state == READ_CLIENT)
		conv.reader->read(conv);

	/* DO THAT AS YOU WANT JUST PLACEHOLDER TO BE CLEAR
	else if (conv.state == WRITE_CLIENT)
		conv.writer->write(conv);
	else if (conv.state == READ_CHILD)
		conv.exec->readCGIOutput(conv);
	*/

	while (true) {
		if (conv.state == PARSE || conv.state == PARSE_BODY
				|| conv.state == EOF_CLIENT)
			conv.parser->parse(conv);
		std::cout << "Parse State enter: " << state_to_str(conv.state).c_str() << std::endl;;
		if (conv.state == VALIDATE)
			conv.validator->validate(conv);
		std::cout << "Parse State enter: " << state_to_str(conv.state).c_str() << std::endl;;
		if (conv.state == READ_CLIENT
				|| conv.state == WRITE_CLIENT
				|| conv.state == FINISH || conv.state == EXEC)
			break;
		
	}
}
