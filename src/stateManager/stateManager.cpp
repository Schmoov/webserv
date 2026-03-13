#include "../../inc/webserv.hpp"
#include "../../inc/read/Reader.hpp"
#include "../../inc/parse/Parser.hpp"
#include "../../inc/validate/Validator.hpp" //calls needs complete type of conv subclass

//You can do *func if you prefer
void stateManager(Conversation& conv) {

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
		if (conv.state == VALIDATE)
			conv.validator->validate(conv);
		if (conv.state == READ_CLIENT
				|| conv.state == WRITE_CLIENT
				|| conv.state == FINISH)
			break;
	}
}
