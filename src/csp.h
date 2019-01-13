

#include "LabText.h"
#include <memory>
#include <string>
#include <vector>

struct CSP_Process
{
    std::string name;
    std::string event;
    std::string behavior;
    std::string out;
};

using lab::Text::StrView;

StrView parse_csp_process(StrView curr, std::vector<std::unique_ptr<CSP_Process>>& processes, bool& error_raised)
{
    // starting from just after the opening parenthesis
    curr = SkipCommentsAndWhitespace(curr);
    StrView token;
    curr = GetTokenAlphaNumeric(curr, token);
    if (IsEmpty(token))
    {
        error_raised = true;
        return curr;
    }

    CSP_Process* p = (*processes.rbegin()).get();
    p->event.assign(token.curr, token.sz);

    curr = SkipCommentsAndWhitespace(curr);
    token = Expect(curr, StrView{"->", 2});
    if (token == curr)
    {
        error_raised = true;
        return curr;
    }
    curr = SkipCommentsAndWhitespace(token);

    // check for the parenthetical recursive form: event -> (PROCESS)
    token = Expect(curr, StrView{"(", 1});
    if (token != curr)
    {
        // create an anonymous nested process
        static int unique = 0;
        CSP_Process* p2 = new CSP_Process();
        char buff[256];
        sprintf(buff, "__%s_%d", p->name.c_str(), unique++);
        p2->name.assign(buff);
        p->behavior = p2->name;
        processes.emplace_back(std::unique_ptr<CSP_Process>(p2));

        curr = parse_csp_process(token, processes, error_raised);
        if (error_raised)
            return curr;
    }
    else
    {
        curr = GetTokenAlphaNumeric(curr, token);
        if (IsEmpty(token))
        {
            error_raised = true;
            return curr;
        }
        p->behavior.assign(token.curr, token.sz);

        // given that event1 -> event2 has been parsed,
        // test for chained form: event1 -> event2 -> event3
        curr = SkipCommentsAndWhitespace(curr);
        token = Expect(curr, StrView{"->", 2});
        if (token != curr)
        {
            /// @TODO, handle this form.
            error_raised = true;
            return curr;
        }
    }
    curr = SkipCommentsAndWhitespace(curr);
    token = Expect(curr, StrView{"\"", 1}); // check for an output string
    if (token != curr)
    {
        curr = GetString(curr, false, token);
        p->out.assign(token.curr, token.sz);
        curr = SkipCommentsAndWhitespace(curr);
    }
    token = Expect(curr, StrView{")", 1});
    if (token == curr)
    {
        error_raised = true;
    }
    return token;
}

std::vector<std::unique_ptr<CSP_Process>> parse_csp(char const*const src, size_t len)
{
    using namespace lab::Text;
    StrView curr{src, len};
    curr = SkipCommentsAndWhitespace(curr);
    std::vector<std::unique_ptr<CSP_Process>> processes;
    bool error_raised = false;
    while (!IsEmpty(curr) && !error_raised)
    {
        StrView token;
        curr = GetTokenAlphaNumeric(curr, token);
        if (curr == token)
        {
            error_raised = true;
            break;
        }

        CSP_Process* p = new CSP_Process();
        p->name.assign(token.curr, token.sz);
        processes.emplace_back(std::unique_ptr<CSP_Process>(p));

        curr = SkipCommentsAndWhitespace(curr);
        token = Expect(curr, StrView{"=", 1});
        if (token == curr)
        {
            error_raised = true;
            break;
        }
        curr = SkipCommentsAndWhitespace(token);
        token = Expect(curr, StrView{"(", 1});
        if (token == curr)
        {
            error_raised = true;
            break;
        }

        curr = SkipCommentsAndWhitespace(token);
        curr = parse_csp_process(curr, processes, error_raised);
        curr = SkipCommentsAndWhitespace(curr);
    }
    return processes;
}
