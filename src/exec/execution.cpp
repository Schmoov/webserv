#include "../../inc/execution/execution.hpp"

std::string execute(Conversation &conversation)
{
    Request request = conversation.req;
    StatusCode status = conversation.resp.status;
    bool shouldClose = conversation.resp.shouldClose;

    if(status != NOT_A_STATUS_CODE)
    {
        conversation.resp.shouldClose = true;   //<<- This should never happen if the parser fails to read headers 
                                                //Should close MUST be set to true in these case.
        return createErrorResponse(status, shouldClose);
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