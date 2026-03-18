#include "../../inc/execution/execution.hpp"

std::string execute(Conversation &conversation)
{
    Request request = conversation.req;

    if(conversation.resp.status != NOT_A_STATUS_CODE)
    {
        //error page;
        //status code to statuic page sinon créer la page
    }


    if(request.method == "GET")
        return handleGet(conversation);
    if(request.method == "DELETE")
        return handleDelete(conversation);
    if(request.method == "POST")
        return handlePost(conversation);

    // Should we exit cause something went very wrong ? Would help catch but easier.
    return "";
}