

#include "LabText.h"
#include "ConcurrentQueue.h"
#include <map>
#include <memory>
#include <string>
#include <mutex>
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

struct CSP_Event
{
    std::string name;
    int id;
};
struct CSP
{
    std::vector<std::unique_ptr<CSP_Process>> processes;
    std::vector<int> process_active;
    std::map<std::string, std::function<void(int)>, std::less<>> lambdas;
    moodycamel::ConcurrentQueue<CSP_Event> q;
    std::mutex process_data_mutex;
};

// merge into an existing csp, or return a new one if supplied with nullptr
CSP* csp_parse(CSP* csp, char const*const src, size_t len)
{
    if (!csp)
        csp = new CSP();

    using namespace lab::Text;
    StrView curr{src, len};
    curr = SkipCommentsAndWhitespace(curr);
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
        csp->processes.emplace_back(std::unique_ptr<CSP_Process>(p));

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
        curr = parse_csp_process(curr, csp->processes, error_raised);
        curr = SkipCommentsAndWhitespace(curr);
    }

    size_t sz = csp->processes.size();
    csp->process_active.resize(sz);
    for (auto i = 0; i < sz; ++i)
    {
        if (csp->processes[i]->name[0] == '_')
            csp->process_active[i] = 0;
        else
            csp->process_active[i] = 1;
    }
    return csp;
}

void csp_bind_lambda(CSP* csp, char const*const name, std::function<void(int)> fn)
{
    if (!csp || !name || !fn)
        return;

    // guard against adding processes, or changing them
    std::unique_lock<std::mutex> lock(csp->process_data_mutex);
    csp->lambdas[name] = fn;
}

void csp_emit(CSP* csp, char const*const name, int id)
{
    if (csp && name)
        csp->q.enqueue({std::string{name}, id});
}

void csp_update(CSP* csp)
{
    if (!csp)
        return;

    // guard against adding processes, or changing them
    std::unique_lock<std::mutex> lock(csp->process_data_mutex);
    CSP_Event event;
    while (csp->q.try_dequeue(event))
    {
        size_t sz = csp->processes.size();
        for (int i = 0; i < sz; ++i)
        {
            if (csp->process_active[i] != 1)
                continue;

            CSP_Process* p = csp->processes[i].get();

            if (event.name != p->event)
                continue;

            auto fn_it = csp->lambdas.find(p->out);
            if (fn_it != csp->lambdas.end())
            {
                std::function<void(int)>& fn = fn_it->second;
                fn(event.id);
            }

            // common case: recur.
            if (p->name == p->behavior)
                continue;

            // transition to the new behavior if there is one.
            csp->process_active[i] = false;
            for (int j = 0; j < sz; ++j)
                if (csp->processes[j]->name == p->behavior)
                    csp->process_active[j] = 2; // set to pending
        }
        for (int i = 0; i < sz; ++i)
            if (csp->process_active[i] == 2)
                csp->process_active[i] = 1;     // pending becomes active, to prevent (tick -> (tick -> TOCK)) from firing immediately the second time
    }
}
